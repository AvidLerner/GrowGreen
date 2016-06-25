#include <Wire.h>
#include <EEPROMVar.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#define DHT11_PIN 0      // ADC0 Analog pin A0analog input A0
// select the pins used on the LCD panel
/* completed 6/23/16 executes the menu system, display date, time, 
temperature and humidity and runs throuhg screens and loops correctly.
define some values used by the panel and buttons
 readButton for each function */
 // The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
// set up backlight
int bklDelay    = 10000;    // ms for the backlight to idle before turning off
unsigned long bklTime = 0;  // counter since backlight turned on
// create the menu counter
int menuCount   = 1;
int menuSelect = 0;

//create the plus and minus navigation delay counter with its initial maximum of 250.
byte btnMaxDelay = 200;
byte btnMinDelay = 25;
byte btnMaxIteration = 5;
byte btnCurrIteration;
// setup[ ds1307  Real Time Clock using i2c address direct
#define DS1307_I2C_ADDRESS 0x68
// backlight on/off
#define ON 0x1
#define OFF 0x0
// establish temperature humidity pin on A0
#define DHT11_PIN 0     // what pin we're connected to
//create manual override variables
boolean override = false;
byte overmenu = 0;
int overpercent = 0;
// button read variables for lcd shield
uint8_t i=0;
uint8_t buttons =0;

int minCounter = 0;         // counter that resets at midnight.
int oldMinCounter = 0;      // counter that resets at midnight.
int oneLed = 9;             // pin for channel 1
int twoLed = 10;            // pin for channel 2
int threeLed = 11;          // pin for channel 3
int fourLed = 3;            // pin for channel 4
int fiveLed = 4;            // pin for channel 5
int sixLed = 5;             // pin for channel 6
int oneVal = 0;             // current value for channel 1
int twoVal = 0;             // current value for channel 2
int threeVal = 0;           // current value for channel 3
int fourVal = 0;            // current value for channel 4
int fiveVal = 0;            // current value for channel 5
int sixVal = 0;             // current value for channel 6
// dht11 data variables for printing temp in farhenheit or celcius
int dhtFTemp=0;
int dhtCTemp=0;
int dhtHumidity=0;
double dhtDTemp=0;
double dhtDHumidity=0;

// Variables making use of EEPROM memory:

EEPROMVar<int> oneStartMins = 360;      // minute to start this channel.
EEPROMVar<int> onePhotoPeriod = 720;   // photoperiod in minutes for this channel.
EEPROMVar<int> oneMax = 100;           // max intensity for this channel, as a percentage
EEPROMVar<int> oneFadeDuration = 60;   // duration of the fade on and off for sunrise and sunset for
                                       //    this channel.
EEPROMVar<int> twoStartMins = 360;  //1330
EEPROMVar<int> twoPhotoPeriod = 720; //
EEPROMVar<int> twoMax = 100;
EEPROMVar<int> twoFadeDuration = 60;

EEPROMVar<int> threeStartMins = 360;
EEPROMVar<int> threePhotoPeriod = 720;
EEPROMVar<int> threeMax = 100;
EEPROMVar<int> threeFadeDuration = 60;
                            
EEPROMVar<int> fourStartMins = 360;
EEPROMVar<int> fourPhotoPeriod = 720;  
EEPROMVar<int> fourMax = 100;          
EEPROMVar<int> fourFadeDuration = 60; 

EEPROMVar<int> fiveStartMins = 360;
EEPROMVar<int> fivePhotoPeriod = 720;  
EEPROMVar<int> fiveMax = 100;          
EEPROMVar<int> fiveFadeDuration = 60; 

EEPROMVar<int> sixStartMins = 360;
EEPROMVar<int> sixPhotoPeriod = 720;  
EEPROMVar<int> sixMax = 100;          
EEPROMVar<int> sixFadeDuration = 60; 

/*
int oneStartMins = 1380;      // minute to start this channel.
int onePhotoPeriod = 120;   // photoperiod in minutes for this channel.
int oneMax = 100;           // max intensity for this channel, as a percentage
int oneFadeDuration = 60;   // duration of the fade on and off for sunrise and sunset for
                                       //    this channel.                                    
int twoStartMins = 800;
int twoPhotoPeriod = 60;
int twoMax = 100;
int twoFadeDuration = 15;

int threeStartMins = 800;
int threePhotoPeriod = 60;
int threeMax = 100;
int threeFadeDuration = 30;
                            
int fourStartMins = 800;
int fourPhotoPeriod = 120;  
int fourMax = 100;          
int fourFadeDuration = 60;  

int fiveStartMins = 800;
int fivePhotoPeriod = 60;
int fiveMax = 100;
int fiveFadeDuration = 30;
                            
int sixStartMins = 800;
int sixPhotoPeriod = 120;  
int sixMax = 100;          
int sixFadeDuration = 60;  
*/
// variables to invert the output PWM signal,
// for use with drivers that consider 0 to be "on"
// i.e. buckpucks. If you need to provide an inverted 
// signal on any channel, set the appropriate variable to true.
boolean oneInverted = false;
boolean twoInverted = false;
boolean threeInverted = false;
boolean fourInverted = false; 
boolean fiveInverted = false;
boolean sixInverted = false; 

int state = 0;       // state of keypress master state = 1 means read keypres
int h = 0;  // hours
int m = 0;  // minutes
int s = 0;  // seconds

// lcd screen variable
int LCD_R=16;  // lcd rows
int LCD_C=2;  // lcd columns

/****** RTC Functions ******/
/***************************/

// Convert decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}
// Sets date and time, starts the clock
void setDate(byte second,        // 0-59
             byte minute,        // 0-59
             byte hour,          // 1-23
             byte dayOfWeek,     // 1-7
             byte dayOfMonth,    // 1-31
             byte month,         // 1-12
             byte year)          // 0-99
{
   Wire.beginTransmission(DS1307_I2C_ADDRESS);
   Wire.write(0);
   Wire.write(decToBcd(second));
   Wire.write(decToBcd(minute));
   Wire.write(decToBcd(hour));
   Wire.write(decToBcd(dayOfWeek));
   Wire.write(decToBcd(dayOfMonth));
   Wire.write(decToBcd(month));
   Wire.write(decToBcd(year));
   Wire.endTransmission();
}

// Gets the date and time
void getDate(byte *second,
             byte *minute,
             byte *hour,
             byte *dayOfWeek,
             byte *dayOfMonth,
             byte *month,
             byte *year)
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}
/****** LED Functions ******/
/***************************/
//function to set LED brightness according to time of day
//function has three equal phases - ramp up, hold, and ramp down

int   setLed(int mins,         // current time in minutes
            int ledPin,        // pin for this channel of LEDs
            int start,         // start time for this channel of LEDs
            int period,        // photoperiod for this channel of LEDs
            int fade,          // fade duration for this channel of LEDs
            int ledMax,        // max value for this channel
            boolean inverted   // true if the channel is inverted
            )  {
  int val = 0;
      
      //fade up
      if (mins > start || mins <= start + fade)  {
        val = map(mins - start, 0, fade, 0, ledMax);
      }
      //fade down
      if (mins > start + period - fade && mins <= start + period)  {
        val = map(mins - (start + period - fade), 0, fade, ledMax, 0);
      }
      //off or post-midnight run.
      if (mins <= start || mins > start + period)  {
        if((start+period)%1440 < start && (start + period)%1440 > mins )
          {
            val=map((start+period-mins)%1440,0,fade,0,ledMax);
          }
        else  
        val = 0; 
      }
    
    
    if (val > ledMax)  {val = ledMax;} 
    if (val < 0) {val = 0; } 
    
  if (inverted) {analogWrite(ledPin, map(val, 0, 100, 255, 0));}
  else {analogWrite(ledPin, map(val, 0, 100, 0, 255));}
  if(override){val=overpercent;}
  return val;
}
/**** Display Functions ****/
/***************************/

//button hold function
int btnCurrDelay(byte curr)
{
  if(curr==btnMaxIteration)
  {
    btnCurrIteration = btnMaxIteration;
    return btnMaxDelay;
  }
  else if(btnCurrIteration ==0)
  {
    return btnMinDelay;
  }
  else
  {
    btnCurrIteration--;
    return btnMaxDelay;
  }
}

// format a number of minutes into a readable time (24 hr format)
void printMins(int mins,       //time in minutes to print
               boolean ampm    //print am/pm?
              )  {
  int hr = (mins%1440)/60;
  int mn = mins%60;
    if(hr<10){
      lcd.print(" ");
    }
    lcd.print(hr);
    lcd.print(":");
    if(mn<10){
      lcd.print("0");
    }
    lcd.print(mn); 
}

// format hours, mins, secs into a readable time (24 hr format)
void printHMS (byte hr,
               byte mn,
               byte sec      //time to print
              )  
{
  
    if(hr<10){
      lcd.print(" ");
    }
    lcd.print(hr, DEC);
    lcd.print(":");
    if(mn<10){
      lcd.print("0");
    }
    lcd.print(mn, DEC);
    lcd.print(":");
    if(sec<10){
      lcd.print("0");
    }
    lcd.print(sec, DEC);
}
void ovrSetAll(int pct){
    analogWrite(oneLed,map(pct,0,100,0,255));
    analogWrite(twoLed,map(pct,0,100,0,255));
    analogWrite(threeLed,map(pct,0,100,0,255));
    analogWrite(fourLed,map(pct,0,100,0,255));
    analogWrite(fiveLed,map(pct,0,100,0,255));
    analogWrite(sixLed,map(pct,0,100,0,255));
}
// setup ddrc and portc for temp/humidity sensor
void DHTSetup(){
  DDRC |= _BV(DHT11_PIN);
  PORTC |= _BV(DHT11_PIN);
  }
// read dht11 data line for input get current temperature and humidity
byte read_dht11_dat()
{
  
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while(!(PINC & _BV(DHT11_PIN)));  // wait for 50us
    delayMicroseconds(30);
    if(PINC & _BV(DHT11_PIN)) 
      result |=(1<<(7-i));
    while((PINC & _BV(DHT11_PIN)));  // wait '1' finish
    }
    return result;
}
// read data for input from dht11 temp/humidity sensor 
void GetDHTData()
  {
  byte dht11_dat[5];
  byte dht11_in;
  byte i;// start condition
   // 1. pull-down i/o pin from 18ms
  PORTC &= ~_BV(DHT11_PIN);
  delay(18);
  PORTC |= _BV(DHT11_PIN);
  delayMicroseconds(40);
  DDRC &= ~_BV(DHT11_PIN);
  delayMicroseconds(40);
  
  dht11_in = PINC & _BV(DHT11_PIN);
  if(dht11_in)
  {
    Serial.println("dht11 start condition 1 not met");
    lcd.print("DHT11 checksum error");
    return;
  }
  delayMicroseconds(80);
  dht11_in = PINC & _BV(DHT11_PIN);
  if(!dht11_in)
  {
    Serial.println("dht11 start condition 2 not met");
    return;
  }
  
  delayMicroseconds(80);// now ready for data reception
  for (i=0; i<5; i++)
    dht11_dat[i] = read_dht11_dat();
  DDRC |= _BV(DHT11_PIN);
  PORTC |= _BV(DHT11_PIN);
  byte dht11_check_sum = dht11_dat[0]+dht11_dat[1]+dht11_dat[2]+dht11_dat[3];// check check_sum
  if(dht11_dat[4]!= dht11_check_sum)
  
  {
    Serial.println("DHT11 checksum error");
  }
  dhtCTemp=(int) round(dht11_dat[2]);
  //double dhtDTemp= double dht11_dat[2];
  dhtFTemp=(int)round(1.8*dht11_dat[2]+32);
  dhtHumidity=(int) dht11_dat[0];
  //double dhtDHumidity= double dht11_dat[0];
  } // end read data
void printTempMenu()
  {
  lcd.setCursor(9,0);
  lcd.print(dhtFTemp);
  lcd.setCursor(11,0);
  lcd.print((char)223);
  lcd.setCursor(13,0);
  lcd.print(dhtHumidity);
  lcd.setCursor(15,0);
  lcd.print("%");
  }
// void setup
void setup()
{
DHTSetup();
Wire.begin();
// setup lcd screen object
PrintBegin();
// set serial printer
SPrintBegin();
//splash screen
lcd.print("GrowGreen CoB");
lcd.setCursor(0,1);
lcd.print("LED Controller");
delay(5000);
lcd.clear();
//analogWrite(bkl,bklIdle);
btnCurrIteration = btnMaxIteration;


} // end setup

// Main loop
void loop(){ 
  
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  getDate(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  oldMinCounter = minCounter;
  minCounter = hour * 60 + minute;
  
  //check & set fade durations
  if(oneFadeDuration > onePhotoPeriod/2 && onePhotoPeriod >0){oneFadeDuration = onePhotoPeriod/2;}
  if(oneFadeDuration<1){oneFadeDuration=1;}
  
  if(twoFadeDuration > twoPhotoPeriod/2 && twoPhotoPeriod >0){twoFadeDuration = twoPhotoPeriod/2;} 
  if(twoFadeDuration<1){twoFadeDuration=1;}
  
  if(threeFadeDuration > threePhotoPeriod/2 && threePhotoPeriod >0){threeFadeDuration = threePhotoPeriod/2;}
  if(threeFadeDuration<1){threeFadeDuration=1;}
  
  if(fourFadeDuration > fourPhotoPeriod/2 && fourPhotoPeriod > 0){fourFadeDuration = fourPhotoPeriod/2;}
  if(fourFadeDuration<1){fourFadeDuration=1;}
 
 if(fiveFadeDuration > fivePhotoPeriod/2 && fivePhotoPeriod >0){fiveFadeDuration = fivePhotoPeriod/2;}
  if(fiveFadeDuration<1){fiveFadeDuration=1;}
  
  if(sixFadeDuration > sixPhotoPeriod/2 && sixPhotoPeriod > 0){sixFadeDuration = sixPhotoPeriod/2;}
  if(sixFadeDuration<1){sixFadeDuration=1;} 
  
  //check & set any time functions
  
  
  //set outputs
  if(!override){
  oneVal = setLed(minCounter, oneLed, oneStartMins, onePhotoPeriod, oneFadeDuration, oneMax, oneInverted);
  twoVal = setLed(minCounter, twoLed, twoStartMins, twoPhotoPeriod, twoFadeDuration, twoMax, twoInverted);
  threeVal = setLed(minCounter, threeLed, threeStartMins, threePhotoPeriod, threeFadeDuration, threeMax, threeInverted);
  fourVal = setLed(minCounter, fourLed, fourStartMins, fourPhotoPeriod, fourFadeDuration, fourMax, fourInverted);
  fiveVal = setLed(minCounter, threeLed, threeStartMins, threePhotoPeriod, threeFadeDuration, threeMax, threeInverted);
  sixVal = setLed(minCounter, fourLed, fourStartMins, fourPhotoPeriod, fourFadeDuration, fourMax, fourInverted);
  
  }
  else{
    ovrSetAll(overpercent);
  }
  
  
  //turn the backlight off and reset the menu if the idle time has elapsed
  if(bklTime + bklDelay < millis() && bklTime > 0 ){
    lcd.setBacklight(OFF);
    menuCount = 1;
    lcd.clear();
    bklTime = 0;
  }

  //iterate through the menus
  buttons = lcd.readButtons();
  if(buttons & BUTTON_SELECT){
    lcd.setBacklight(ON);
    bklTime = millis();
    if(menuCount < 28){
      menuCount++;
    }else {
      menuCount = 1;
    }
  lcd.clear();
  }
//main screen turn on!!! 
  
  if(menuCount == 1){
   
    if (minCounter > oldMinCounter){
      lcd.clear();
    }
    lcd.setCursor(0,0);
    printHMS(hour, minute, second);
    lcd.setCursor(0,1);
    lcd.print(oneVal);
    lcd.setCursor(4,1);
    lcd.print(twoVal);
    lcd.setCursor(8,1);
    lcd.print(threeVal);
    lcd.setCursor(12,1);
    lcd.print(fourVal);
    //lcd.setCursor(12,1);
    //lcd.print(fiveVal);
    //lcd.setCursor(15,1);
    //lcd.print(sixVal);
    // print temperature/humidity
    GetDHTData();
    printTempMenu();
    //debugging function to use the select button to advance the timer by 1 minute
    //if(select.uniquePress()){setDate(second, minute+1, hour, dayOfWeek, dayOfMonth, month, year);}
  }
  
  //Manual Override Menu
  if(menuCount == 2){
    buttons = lcd.readButtons();
    lcd.setCursor(0,0);
    lcd.print("Manual Overrides");
    lcd.setCursor(0,1);
    lcd.print("All: ");
    //buttons = lcd.readButtons();
    if(buttons & BUTTON_RIGHT){
      //buttons = lcd.readButtons();
      if(menuSelect < 3 ){
        menuSelect++;
        
      }
      else{menuSelect = 0;}
      bklTime = millis();
    }
    
    if(menuSelect == 0){
      lcd.print("Timer");
      override = false;}
    if(menuSelect == 1  ){
      lcd.print("ON   ");
      overpercent = 100;
      override = true;}
    if(menuSelect == 2){
      lcd.print("OFF  ");
      overpercent = 0;
      override = true;}    
    if(menuSelect == 3  ){
      override = true;
      lcd.print(overpercent,DEC);
      lcd.print("%  ");
      if((buttons & BUTTON_RIGHT) && overpercent <100)
        {
          overpercent++;
          delay(btnCurrDelay(btnCurrIteration-1));
          bklTime = millis();
        }
        
        if((buttons & BUTTON_LEFT) && overpercent > 0)
        {
          overpercent--;
          delay(btnCurrDelay(btnCurrIteration-1));
          bklTime = millis();
        }
      }
  } // menucount = 2
  

//set start time for channel one
  if(menuCount == 3){
    lcd.setCursor(0,0);
    lcd.print("Channel 1 Start");
    lcd.setCursor(0,1);
    printMins(oneStartMins, true);
    
    if((buttons & BUTTON_RIGHT) && oneStartMins < 1440){
        oneStartMins++;
        if(onePhotoPeriod >0){onePhotoPeriod--;}
        else{onePhotoPeriod=1439;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && oneStartMins > 0){
        oneStartMins--;
        if(onePhotoPeriod<1439){onePhotoPeriod++;}
        else{onePhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set end time for channel one
  if(menuCount == 4){
    lcd.setCursor(0,0);
    lcd.print("Channel 1 End");
    lcd.setCursor(0,1);
    printMins(oneStartMins+onePhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(onePhotoPeriod < 1439){
      onePhotoPeriod++;}
      else{
        onePhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(onePhotoPeriod >0){
        onePhotoPeriod--;}
      else{
        onePhotoPeriod=1439;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set fade duration for channel one
  if(menuCount == 5){
    lcd.setCursor(0,0);
    lcd.print("Channel 1 Fade");
    lcd.setCursor(0,1);
    printMins(oneFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (oneFadeDuration < onePhotoPeriod/2 || oneFadeDuration == 0)){
      oneFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && oneFadeDuration > 1){
      oneFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel one
  if(menuCount == 6){
    lcd.setCursor(0,0);
    lcd.print("Channel 1 Max");
    lcd.setCursor(1,1);
    lcd.print(oneMax);
    lcd.print("  ");
    if((buttons & BUTTON_RIGHT) && oneMax < 100){
      oneMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && oneMax > 0){
      oneMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set start time for channel two
  if(menuCount == 7){
    lcd.setCursor(0,0);
    lcd.print("Channel 2 Start");
    lcd.setCursor(0,1);
    printMins(twoStartMins, true);
    if((buttons & BUTTON_RIGHT) && twoStartMins < 1440){
        twoStartMins++;
        if(twoPhotoPeriod >0){twoPhotoPeriod--;}
        else{twoPhotoPeriod=1439;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && twoStartMins > 0){
        twoStartMins--;
        if(twoPhotoPeriod<1439){twoPhotoPeriod++;}
        else{twoPhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
 //set end time for channel two
  if(menuCount == 8){
   lcd.setCursor(0,0);
    lcd.print("Channel 2 End");
    lcd.setCursor(0,1);
    printMins(twoStartMins+twoPhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(twoPhotoPeriod < 1439){
      twoPhotoPeriod++;}
      else{
        twoPhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(twoPhotoPeriod >0){
        twoPhotoPeriod--;}
      else{
        twoPhotoPeriod=1439;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set fade duration for channel two
  if(menuCount == 9){
    lcd.setCursor(0,0);
    lcd.print("Channel 2 Fade");
    lcd.setCursor(0,1);
    printMins(twoFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (twoFadeDuration < twoPhotoPeriod/2 || twoFadeDuration == 0)){
      twoFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && twoFadeDuration > 1){
      twoFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel two
  if(menuCount == 10){
    lcd.setCursor(0,0);
    lcd.print("Channel 2 Max");
    lcd.setCursor(1,1);
    lcd.print(twoMax);
    lcd.print("  ");
    if((buttons & BUTTON_RIGHT) && twoMax < 100){
      twoMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && twoMax > 0){
      twoMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set start time for channel three
  if(menuCount == 11){
    lcd.setCursor(0,0);
    lcd.print("Channel 3 Start");
    lcd.setCursor(0,1);
    printMins(threeStartMins, true);
    if((buttons & BUTTON_RIGHT) && threeStartMins < 1440){
        threeStartMins++;
        if(threePhotoPeriod >0){threePhotoPeriod--;}
        else{threePhotoPeriod=1439;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && threeStartMins > 0){
        threeStartMins--;
        if(threePhotoPeriod<1439){threePhotoPeriod++;}
        else{threePhotoPeriod=0;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set end time for channel three
  if(menuCount == 12){
    lcd.setCursor(0,0);
    lcd.print("Channel 3 End");
    lcd.setCursor(0,1);
    printMins(threeStartMins+threePhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(threePhotoPeriod < 1439){
      threePhotoPeriod++;}
      else{
        threePhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(threePhotoPeriod >0){
        threePhotoPeriod--;}
      else{
        threePhotoPeriod=1439;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set fade duration for channel three
  if(menuCount == 13){
    lcd.setCursor(0,0);
    lcd.print("Channel 3 Fade");
    lcd.setCursor(0,1);
    printMins(threeFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (threeFadeDuration < threePhotoPeriod/2 || threeFadeDuration == 0)){
      threeFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && threeFadeDuration > 1){
      threeFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel three
  if(menuCount == 14){
    lcd.setCursor(0,0);
    lcd.print("Channel 3 Max");
    lcd.setCursor(1,1);
    lcd.print(threeMax);
    lcd.print("  ");
    if((buttons & BUTTON_RIGHT) && threeMax < 100){
      threeMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && threeMax > 0){
      threeMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set start time for channel four
  if(menuCount == 15){
    lcd.setCursor(0,0);
    lcd.print("Channel 4 Start");
    lcd.setCursor(0,1);
    printMins(fourStartMins, true);
    if((buttons & BUTTON_RIGHT) && fourStartMins < 1440){
        fourStartMins++;
        if(fourPhotoPeriod >0){fourPhotoPeriod--;}
        else{fourPhotoPeriod=1439;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourStartMins > 0){
        fourStartMins--;
        if(fourPhotoPeriod<1439){fourPhotoPeriod++;}
        else{fourPhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
 //set end time for channel four
  if(menuCount == 16){
    lcd.setCursor(0,0);
    lcd.print("Channel 4 End");
    lcd.setCursor(0,1);
    printMins(fourStartMins+fourPhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(fourPhotoPeriod < 1439){
      fourPhotoPeriod++;}
      else{
        fourPhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(fourPhotoPeriod >0){
        fourPhotoPeriod--;}
      else{
        fourPhotoPeriod=1439;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set fade duration for channel four
  if(menuCount == 17){
    lcd.setCursor(0,0);
    lcd.print("Channel 4 Fade");
    lcd.setCursor(0,1);
    printMins(fourFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (fourFadeDuration < fourPhotoPeriod/2 || fourFadeDuration == 0)){
      fourFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourFadeDuration > 1){
      fourFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel four
  if(menuCount == 18){
    lcd.setCursor(0,0);
    lcd.print("Channel 4 Max");
    lcd.setCursor(1,1);
    lcd.print(fourMax);
    lcd.print("   ");
    if((buttons & BUTTON_RIGHT) && fourMax < 100){
      fourMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourMax > 0){
      fourMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set start time for channel five
  if(menuCount == 19){
    lcd.setCursor(0,0);
    lcd.print("Channel 5 Start");
    lcd.setCursor(0,1);
    printMins(fourStartMins, true);
    if((buttons & BUTTON_RIGHT) && fourStartMins < 1440){
        fourStartMins++;
        if(fourPhotoPeriod >0){fourPhotoPeriod--;}
        else{fourPhotoPeriod=1439;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourStartMins > 0){
        fourStartMins--;
        if(fourPhotoPeriod<1439){fourPhotoPeriod++;}
        else{fourPhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
 //set end time for channel five
  if(menuCount == 20){
    lcd.setCursor(0,0);
    lcd.print("Channel 5 End");
    lcd.setCursor(0,1);
    printMins(fourStartMins+fourPhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(fourPhotoPeriod < 1439){
      fourPhotoPeriod++;}
      else{
        fourPhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(fourPhotoPeriod >0){
        fourPhotoPeriod--;}
      else{
        fourPhotoPeriod=1439;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set fade duration for channel five
  if(menuCount == 21){
    lcd.setCursor(0,0);
    lcd.print("Channel 5 Fade");
    lcd.setCursor(0,1);
    printMins(fourFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (fourFadeDuration < fourPhotoPeriod/2 || fourFadeDuration == 0)){
      fourFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourFadeDuration > 1){
      fourFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel five
  if(menuCount == 22){
    lcd.setCursor(0,0);
    lcd.print("Channel 5 Max");
    lcd.setCursor(1,1);
    lcd.print(fourMax);
    lcd.print("   ");
    if((buttons & BUTTON_RIGHT) && fourMax < 100){
      fourMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourMax > 0){
      fourMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set start time for channel six
  if(menuCount == 23){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 Start");
    lcd.setCursor(0,1);
    printMins(fourStartMins, true);
    if((buttons & BUTTON_RIGHT) && fourStartMins < 1440){
        fourStartMins++;
        if(fourPhotoPeriod >0){fourPhotoPeriod--;}
        else{fourPhotoPeriod=1439;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourStartMins > 0){
        fourStartMins--;
        if(fourPhotoPeriod<1439){fourPhotoPeriod++;}
        else{fourPhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
 //set end time for channel six
  if(menuCount == 24){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 End");
    lcd.setCursor(0,1);
    printMins(fourStartMins+fourPhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(fourPhotoPeriod < 1439){
      fourPhotoPeriod++;}
      else{
        fourPhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(fourPhotoPeriod >0){
        fourPhotoPeriod--;}
      else{
        fourPhotoPeriod=1439;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set fade duration for channel six
  if(menuCount == 25){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 Fade");
    lcd.setCursor(0,1);
    printMins(fourFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (fourFadeDuration < fourPhotoPeriod/2 || fourFadeDuration == 0)){
      fourFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourFadeDuration > 1){
      fourFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel six
  if(menuCount == 26){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 Max");
    lcd.setCursor(1,1);
    lcd.print(fourMax);
    lcd.print("   ");
    if((buttons & BUTTON_RIGHT) && fourMax < 100){
      fourMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fourMax > 0){
      fourMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }    
//set hours
  if(menuCount == 27){
    lcd.setCursor(0,0);
    lcd.print("Set Time: Hrs");
    lcd.setCursor(0,1);
    printHMS(hour, minute, second);
    if((buttons & BUTTON_RIGHT) && hour < 23){
      hour++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && hour > 0){
      hour--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  setDate(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
  }
 //set minutes 
  if(menuCount == 28){
    lcd.setCursor(0,0);
    lcd.print("Set Time: Mins");
    lcd.setCursor(0,1);
    printHMS(hour, minute, second);
    if((buttons & BUTTON_RIGHT) && minute < 59){
      minute++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && minute > 0){
      minute--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  setDate(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
  } // end menu count 28
}  // end loop
// setup lcd for use
void PrintBegin()
{
  lcd.begin(LCD_R, LCD_C);   // start the library  
  lcd.setBacklight(ON);
   
}
//  setup serial printer for use
void SPrintBegin()
  {
  Serial.begin(9600);
  Serial.println("Ready"); 
  }

// clean lcd screen
void cleanScreen()
{
  lcd.setCursor(0,0);
  lcd.print("                "); 
  lcd.setCursor(0,1);
  lcd.print("                ");
}
// readButtons from lcd shield
void ReadButtons()
   {
    uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
      lcd.setBacklight(ON);
    }
    if (buttons & BUTTON_DOWN) {
     lcd.setBacklight(ON);
    }
    if (buttons & BUTTON_LEFT) {
      lcd.setBacklight(ON);
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.setBacklight(ON);
    }
    if (buttons & BUTTON_SELECT) {
      lcd.setBacklight(ON);
    }
  }
}
// end ReadButtons
// end of sketch
