/* Revised the variableas to include updating all EEPROM variables, resetting variables to their original defaults, 
 * established an object called channel[] to ionclude the data for each channel using max_channels as the max variable
 * Developed a print buffer to reduce problems with corrupted lcd screen.  Now uses a buffer to update time, and channel values
 */
#include <Wire.h>
#include <EEPROMVar.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
//#include <dht11.h>  
// setup DHT for use
//dht11 DHT;
// DHT data  pin
#define DHT11_PIN 0 // ADC0 for Uno
int DHT11Temp = 0;
int DHT11Hum = 0;
#define MAX_CHANNELS 5
//#define RESET_EEPROM  remove note to reset EEPROM's during setup
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
int bklDelay    = 5000;    // ms for the backlight to idle before turning off
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
// lcd screen variable
int LCD_R=16;  // lcd rows
int LCD_C=2;  // lcd columns
// establish temperature humidity pin on A0
//create manual override variables
boolean override = false;
byte overmenu = 0;
int overpercent = 0;
// button read variables for lcd shield
uint8_t i=0;
uint8_t buttons =0;
//------------------- system variables-----------------------//
int minCounter = 0;         // counter that resets at midnight.
int oldMinCounter = 0;      // counter that resets at midnight.
int oneLed = 9;             // pin for channel 1
int twoLed = 10;            // pin for channel 2
int threeLed = 11;          // pin for channel 3
int fourLed = 5;            // pin for channel 4
int fiveLed = 6;            // pin for channel 5
int sixLed = 7;             // pin for channel 6
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

         // current led value
EEPROMVar<int> oneStartMins = 360;      // minute to start this channel.
EEPROMVar<int> onePhotoPeriod = 720;    // photoperiod in minutes for this channel.
EEPROMVar<int> oneMax = 99;             // max intensity for this channel, as a percentage
EEPROMVar<int> oneMin = 0;
EEPROMVar<int> oneFadeDuration = 15;    // duration of the fade on and off for sunrise and sunset for
EEPROMVar<boolean> oneInverted = false; // inverted PWM signal 
                                        // this channel.

EEPROMVar<int> twoStartMins = 360;  //  6:am 60 x 6
EEPROMVar<int> twoPhotoPeriod = 720; //
EEPROMVar<int> twoMax = 99;
EEPROMVar<int> twoMin = 0;
EEPROMVar<int> twoFadeDuration = 15;
EEPROMVar<boolean> twoInverted = false; 


EEPROMVar<int> threeStartMins = 360;
EEPROMVar<int> threePhotoPeriod = 720;
EEPROMVar<int> threeMax = 99;
EEPROMVar<int> threeMin = 0;
EEPROMVar<int> threeFadeDuration = 15;
EEPROMVar<boolean> threeInverted = false;
 
                     
EEPROMVar<int> fourStartMins = 1080;  // 6:pm 60 x 18
EEPROMVar<int> fourPhotoPeriod = 720;  
EEPROMVar<int> fourMax = 99;
EEPROMVar<int> fourMin = 0;          
EEPROMVar<int> fourFadeDuration = 15;
EEPROMVar<boolean> fourInverted = false;  


EEPROMVar<int> fiveStartMins = 1080;
EEPROMVar<int> fivePhotoPeriod = 720;  
EEPROMVar<int> fiveMax = 99; 
EEPROMVar<int> fiveMin = 0;         
EEPROMVar<int> fiveFadeDuration = 15; 
EEPROMVar<boolean> fiveInverted = false; 


EEPROMVar<int> sixStartMins = 1080;
EEPROMVar<int> sixPhotoPeriod = 720;  
EEPROMVar<int> sixMax = 99; 
EEPROMVar<int> sixMin = 0;         
EEPROMVar<int> sixFadeDuration = 15;
EEPROMVar<boolean> sixInverted = false; 
 
// variables to invert the output PWM signal,
// for use with drivers that consider 0 to be "on"
// i.e. buckpucks. If you need to provide an inverted 
// signal on any channel, set the appropriate variable to true.
// structure for channel data
typedef struct {
char* Desc; // Channel Description
int Led; // channel pin
int StartMins; // minute to start this channel.
int PhotoPeriod; // photoperiod in minutes for this channel.
int ledMin; // min intensity for this channel, as a percentage
int ledMax; // max intensity for this channel, as a percentage
int ledVal; // current intensity for this channel, as a percentage
int FadeDuration; // duration of the fade on and off for sunrise and sunset for this channel.
boolean Inverted;
} channelVals_t;

channelVals_t channel[MAX_CHANNELS];

int state = 0;       // state of keypress master state = 1 means read keypres
int h = 0;  // hours
int m = 0;  // minutes
int s = 0;  // seconds
//-----------print functions ------------------/
void PrintBegin()
{
  lcd.begin(LCD_R, LCD_C);   // start the library  
  lcd.setBacklight(ON);
   
}
void printTempMenu()
  {
  lcd.setCursor(12,0);
  lcd.print(dhtFTemp);
  lcd.setCursor(15,0);
  lcd.print((char)223);
  }
  void printTempHumidityMenu()
  {
  lcd.setCursor(9,0);
  lcd.print(dhtFTemp);
  lcd.setCursor(12,0);
  lcd.print((char)223);
  lcd.setCursor(13,0);
  lcd.print(dhtHumidity);
  lcd.setCursor(15,0);
  lcd.print("%");
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
// print setup splash screen
void splash_screen() {
lcd.print("GrowGreen CoB");
lcd.setCursor(0,1);
lcd.print("LED Controller");
delay(5000);
lcd.clear();  
}
// readButtons from lcd shield
void ReadButtons()
   {
    uint8_t buttons = lcd.readButtons();

  if (buttons) {
    //lcd.clear();
    //lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
     
    }
    if (buttons & BUTTON_DOWN) {
    
    }
    if (buttons & BUTTON_LEFT) {
     
    }
    if (buttons & BUTTON_RIGHT) {
     
    }
    if (buttons & BUTTON_SELECT) {
      
    }
  }
}
// end ReadButtons
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
    //lcd.print(":");
    //if(sec<10){
      //lcd.print("0");
    //}
    //lcd.print(sec, DEC);
}
//------------- EEPROM functions----------------//
void updateEEPromVars() {
// Update EEProms with any values that may have changed
// Only updates values that have changed
if (channel[0].StartMins != oneStartMins) {
oneStartMins = channel[0].StartMins;
}
if (channel[0].PhotoPeriod != onePhotoPeriod) {
onePhotoPeriod = channel[0].PhotoPeriod;
}
if (channel[0].ledMin != oneMin) {
oneMin = channel[0].ledMin;
}
if (channel[0].ledMax != oneMax) {
oneMax = channel[0].ledMax;
}
if (channel[0].FadeDuration != oneFadeDuration) {
oneFadeDuration = channel[0].FadeDuration;
}

if (channel[1].StartMins != twoStartMins) {
twoStartMins = channel[1].StartMins;
}
if (channel[1].PhotoPeriod != twoPhotoPeriod) {
twoPhotoPeriod = channel[1].PhotoPeriod;
}
if (channel[1].ledMin != twoMin) {
twoMax = channel[1].ledMax;
}
if (channel[1].ledMax != twoMax) {
twoMax = channel[1].ledMax;
}
if (channel[1].FadeDuration != twoFadeDuration) {
twoFadeDuration = channel[1].FadeDuration;
}

if (channel[2].StartMins != threeStartMins) {
threeStartMins = channel[2].StartMins;
}
if (channel[2].PhotoPeriod != threePhotoPeriod) {
threePhotoPeriod = channel[2].PhotoPeriod;
}
if (channel[2].ledMin != threeMin) {
threeMin = channel[2].ledMin;
}
if (channel[2].ledMax != threeMax) {
threeMax = channel[2].ledMax;
}
if (channel[2].FadeDuration != threeFadeDuration) {
threeFadeDuration = channel[2].FadeDuration;
}

if (channel[3].StartMins != fourStartMins) {
fourStartMins = channel[3].StartMins;
}
if (channel[3].PhotoPeriod != fourPhotoPeriod) {
fourPhotoPeriod = channel[3].PhotoPeriod;
}
if (channel[3].ledMin != fourMin) {
fourMin = channel[3].ledMin;
}
if (channel[3].ledMax != fourMax) {
fourMax = channel[3].ledMax;
}
if (channel[4].FadeDuration != fiveFadeDuration) {
fiveFadeDuration = channel[4].FadeDuration;
}
if (channel[4].StartMins != fiveStartMins) {
fiveStartMins = channel[4].StartMins;
}
if (channel[4].PhotoPeriod != fivePhotoPeriod) {
fivePhotoPeriod = channel[4].PhotoPeriod;
}
if (channel[4].ledMin != fiveMin) {
fiveMin = channel[4].ledMin;
}
if (channel[4].ledMax != fiveMax) {
fiveMax = channel[4].ledMax;
}
if (channel[5].FadeDuration != sixFadeDuration) {
sixFadeDuration = channel[5].FadeDuration;
}
if (channel[5].StartMins != sixStartMins) {
sixStartMins = channel[5].StartMins;
}
if (channel[5].PhotoPeriod != sixPhotoPeriod) {
sixPhotoPeriod = channel[5].PhotoPeriod;
}
if (channel[5].ledMin != sixMin) {
sixMin = channel[5].ledMin;
}
if (channel[5].ledMax != sixMax) {
sixMax = channel[5].ledMax;
}
}
// end EEPROM functions
void ovrSetAll(int pct){
     for (int i = 0; i <= MAX_CHANNELS +1; i++) {
    analogWrite(channel[i].Led,map(pct,0,100,0,255));
  }
}
// setup ddrc and portc for temp/humidity sensor
// DHT11_GetData Use with Mega 2560 simple and direct
/*
void GetDHT11Data()
{
int chk = DHT.read(DHT11_PIN);    // READ DATA
  DHT11Temp = (int) round(DHT.temperature);
  dhtHumidity = DHT.humidity;
  dhtFTemp =  (int)round((1.8*DHT11Temp)+32);
}
// use for Uno compile does not work with Mega 2560
*/
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
// use for Uno compile does not work with mega 2560

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
   // Serial.println("dht11 start condition 1 not met");
    
    return;
  }
  delayMicroseconds(80);
  dht11_in = PINC & _BV(DHT11_PIN);
  if(!dht11_in)
  {
    //Serial.println("dht11 start condition 2 not met");
   
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
    //Serial.println("DHT11 checksum error");
    
  }
  dhtCTemp=(int) round(dht11_dat[2]);
  //double dhtDTemp= double dht11_dat[2];
  dhtFTemp=(int)round(1.8*dht11_dat[2]+32);
  dhtHumidity=(int) dht11_dat[0];
  //double dhtDHumidity= double dht11_dat[0];
  } // end read data DHT11
  //temperture menu
// ---User Fucntions ------------------//
// Set_fadae
 //check & set fade durations
void set_fade(){ 
   for (int i = 0; i <= MAX_CHANNELS; i++) { //check & adjust fade durations
   if(channel[i].FadeDuration > channel[i].PhotoPeriod/2 && channel[i].PhotoPeriod >0) {channel[i].FadeDuration = channel[i].PhotoPeriod/2;}
   if(channel[i].FadeDuration < 1) {channel[i].FadeDuration = 1;}
}
}
// end saet_fade
// set outputs
void set_output() {
 if(!override){
  for (int i = 0; i <= MAX_CHANNELS; i++){
      channel[i].ledVal = setLed(minCounter, channel[i].Led, channel[i].StartMins, channel[i].PhotoPeriod, channel[i].FadeDuration, channel[i].ledMax, channel[i].Inverted);}
  }
  else{
    ovrSetAll(overpercent);
  }
}
// end set outputs
// void setup
void setup()
{
#ifdef RESET_EEPROM
oneStartMins = 360; // minute to start this channel.
onePhotoPeriod = 1050; // photoperiod in minutes for this channel.
oneMin = 0; // min intensity for this channel, as a percentage
oneMax = 99; // max intensity for this channel, as a percentage
oneFadeDuration = 15; // duration of the fade on and off for sunrise and sunset for  this channel.
oneInverted = false;
twoStartMins = 360;
twoPhotoPeriod = 1050;
twoMin = 0; // min intensity for this channel, as a percentage
twoMax = 99;
twoFadeDuration = 15;
twoInverted = false;
threeStartMins = 360;
threePhotoPeriod = 1050;
threeMin = 0; // min intensity for this channel, as a percentage
threeMax = 99;
threeFadeDuration = 15;
threeInverted = false;
fourStartMins = 1080;
fourPhotoPeriod = 1050;
fourMin = 0; // min intensity for this channel, as a percentage
fourMax = 99;
fourFadeDuration = 15;
fourInverted = false;
fiveStartMins = 1080;
fivePhotoPeriod = 1050;
fiveMin = 0; // min intensity for this channel, as a percentage
fiveMax = 99;
fiveFadeDuration = 15;
fiveInverted = false;
sixStartMins = 1080;
sixPhotoPeriod = 1050;
sixMin = 0; // min intensity for this channel, as a percentage
sixMax = 99;
sixFadeDuration = 15;
sixInverted = false;
#endif
// Initialize channel variables. Set LED channel pin and retrieve values from EEPROM
channel[0].Desc = "Channel 1:";
channel[0].Led = oneLed;
channel[0].StartMins = oneStartMins;
channel[0].PhotoPeriod = onePhotoPeriod;
channel[0].ledMin = oneMin;
channel[0].ledMax = oneMax;
channel[0].ledVal = oneVal;
channel[0].FadeDuration = oneFadeDuration;
channel[0].Inverted = oneInverted;

channel[1].Desc = "Channel 2:";
channel[1].Led = twoLed;
channel[1].StartMins = twoStartMins;
channel[1].PhotoPeriod = twoPhotoPeriod;
channel[1].ledMin = twoMin;
channel[1].ledMax = twoMax;
channel[1].ledVal = twoVal;
channel[1].FadeDuration = twoFadeDuration;
channel[1].Inverted = twoInverted;

channel[2].Desc = "Channel 3:";
channel[2].Led = threeLed;
channel[2].StartMins = threeStartMins;
channel[2].PhotoPeriod = threePhotoPeriod;
channel[2].ledMin = threeMin;
channel[2].ledMax = threeMax;
channel[2].ledVal = threeVal;
channel[2].FadeDuration = threeFadeDuration;
channel[2].Inverted = threeInverted;

channel[3].Desc = "Channel 4:";
channel[3].Led = fourLed;
channel[3].StartMins = fourStartMins;
channel[3].PhotoPeriod = fourPhotoPeriod;
channel[3].ledMin = fourMin;
channel[3].ledMax = fourMax;
channel[3].ledVal = fourVal;
channel[3].FadeDuration = fourFadeDuration;
channel[4].Inverted = fourInverted;

channel[4].Desc = "Channel 5:";
channel[4].Led = fiveLed;
channel[4].StartMins = fiveStartMins;
channel[4].PhotoPeriod = fivePhotoPeriod;
channel[4].ledMin = fiveMin;
channel[4].ledMax = fiveMax;
channel[4].ledVal = fiveVal;
channel[4].FadeDuration = fiveFadeDuration;
channel[4].Inverted = fiveInverted;

channel[5].Desc = "Channel 6:";
channel[5].Led = sixLed;
channel[5].StartMins = sixStartMins;
channel[5].PhotoPeriod = sixPhotoPeriod;
channel[5].ledMin = sixMin;
channel[5].ledMax = sixMax;
channel[5].ledVal = sixVal;
channel[5].FadeDuration = sixFadeDuration;
channel[5].Inverted = sixInverted;
Wire.begin();
//DHTSetup();

// setup lcd screen object
PrintBegin();
// set serial printer
SPrintBegin();
//splash screen
splash_screen();
//lcd.setBacklight(OFF);
//analogWrite(bkl,bklIdle);
btnCurrIteration = btnMaxIteration;
updateEEPromVars();

} // end setup

// Main loop
void loop(){ 
  char buffer[17];
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  getDate(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  oldMinCounter = minCounter;
  minCounter = hour * 60 + minute;
  // set fade
  set_fade();
  //check & set any time functions
    
  //set outputs
 set_output();
   
  //turn the backlight off and reset the menu if the idle time has elapsed
  if(bklTime + bklDelay < millis() && bklTime > 0 ){
    //lcd.setBacklight(OFF);
    menuCount = 1;
    lcd.clear();
    bklTime = 0;
  }

  //iterate through the menus
  uint8_t buttons = lcd.readButtons();
  if(buttons & BUTTON_SELECT){
    lcd.setBacklight(ON);
    bklTime = millis();
    if(menuCount < 28){
      menuCount++;
    }else {
      menuCount = 1;
    }
  lcd.clear();
  } // end if
//main screen turn on!!! 
  
  if(menuCount == 1){
   
    //if (minCounter > oldMinCounter){
    //  lcd.clear();
    //}
    
    lcd.setCursor(0,0);
    sprintf(buffer, "%3d %3d %3d", channel[0].ledVal, channel[1].ledVal, channel[2].ledVal);
    lcd.print(buffer);
    lcd.setCursor(0,1);
    sprintf(buffer, "%3d %3d %3d%2d:%02d", channel[3].ledVal, channel[4].ledVal, channel[5].ledVal, hour, minute);
    lcd.print(buffer);
    
    //debugging function to use the select button to advance the timer by 1 minute
    //if(select.uniquePress()){setDate(second, minute+1, hour, dayOfWeek, dayOfMonth, month, year);}
  }
  
  //Manual Override Menu
  if(menuCount == 2){
    lcd.setCursor(0,0);
    lcd.print("Manual Overrides");
    lcd.setCursor(0,1);
    lcd.print("All: ");
   
    if(buttons & BUTTON_RIGHT){
     
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
        else{onePhotoPeriod=1439;} // 24 hours
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && oneStartMins > 0){
        oneStartMins--;
        if(onePhotoPeriod<1439){onePhotoPeriod++;} // LT 24 hours
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
    printMins(oneStartMins+onePhotoPeriod, true); // add start and duration to calcuate end time
    if(buttons & BUTTON_RIGHT){
      if(onePhotoPeriod < 1439){ // 24 hours max
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
        onePhotoPeriod=1439; // 24 hours
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
    printMins(fiveStartMins, true);
    if((buttons & BUTTON_RIGHT) && fiveStartMins < 1440){
        fiveStartMins++;
        if(fivePhotoPeriod >0){fivePhotoPeriod--;}
        else{fivePhotoPeriod=1439;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fiveStartMins > 0){
        fiveStartMins--;
        if(fivePhotoPeriod<1439){fivePhotoPeriod++;}
        else{fivePhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
 //set end time for channel five
  if(menuCount == 20){
    lcd.setCursor(0,0);
    lcd.print("Channel 5 End");
    lcd.setCursor(0,1);
    printMins(fiveStartMins+fivePhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(fivePhotoPeriod < 1439){
      fivePhotoPeriod++;}
      else{
        fivePhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(fivePhotoPeriod >0){
        fivePhotoPeriod--;}
      else{
        fivePhotoPeriod=1439;
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
    printMins(fiveFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (fiveFadeDuration < fivePhotoPeriod/2 || fiveFadeDuration == 0)){
      fiveFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fiveFadeDuration > 1){
      fiveFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel five
  if(menuCount == 22){
    lcd.setCursor(0,0);
    lcd.print("Channel 5 Max");
    lcd.setCursor(1,1);
    lcd.print(fiveMax);
    lcd.print("   ");
    if((buttons & BUTTON_RIGHT) && fiveMax < 100){
      fiveMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && fiveMax > 0){
      fiveMax--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set start time for channel six
  if(menuCount == 23){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 Start");
    lcd.setCursor(0,1);
    printMins(sixStartMins, true);
    if((buttons & BUTTON_RIGHT) && sixStartMins < 1440){
        sixStartMins++;
        if(sixPhotoPeriod >0){sixPhotoPeriod--;}
        else{sixPhotoPeriod=1439;}
        delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && sixStartMins > 0){
        sixStartMins--;
        if(sixPhotoPeriod<1439){sixPhotoPeriod++;}
        else{sixPhotoPeriod=0;}
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
 //set end time for channel six
  if(menuCount == 24){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 End");
    lcd.setCursor(0,1);
    printMins(sixStartMins+sixPhotoPeriod, true);
    if(buttons & BUTTON_RIGHT){
      if(sixPhotoPeriod < 1439){
      sixPhotoPeriod++;}
      else{
        sixPhotoPeriod=0;
      }
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if(buttons & BUTTON_LEFT){
      if(sixPhotoPeriod >0){
        sixPhotoPeriod--;}
      else{
        sixPhotoPeriod=1439;
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
    printMins(sixFadeDuration, false);
    if((buttons & BUTTON_RIGHT) && (sixFadeDuration < sixPhotoPeriod/2 || sixFadeDuration == 0)){
      sixFadeDuration++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && sixFadeDuration > 1){
      sixFadeDuration--;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
  }
//set intensity for channel six
  if(menuCount == 26){
    lcd.setCursor(0,0);
    lcd.print("Channel 6 Max");
    lcd.setCursor(1,1);
    lcd.print(sixMax);
    lcd.print("   ");
    if((buttons & BUTTON_RIGHT) && sixMax < 100){
      sixMax++;
      delay(btnCurrDelay(btnCurrIteration-1));
      bklTime = millis();
    }
    if((buttons & BUTTON_LEFT) && sixMax > 0){
      sixMax--;
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
// end of sketch
