// @file AlphaLicense.txt
// @version 1.0
//@author Alexander Brevig
// @contact alexanderbrevig@gmail.com
// @contribution
//  Name Surname - www.domain.com - contact@domain.com
// #
// @description
// | Descripbe the rights and expectations when using my code
// #
//
// @dependencies
// | This file depends on the user having accsess to the internet, in order to read the LGPL
// #
//
// @configuration
// | No configuration needed
// #
// @license
// | Copyright (c) 2009 Alexander Brevig. All rights reserved.
// | http://www.gnu.org/licenses/lgpl.html
// #
//***************************************************************************************************
//RotaryEncoder 
//ver. 1.1
//Updated 8-5-2016
//This library is being shared under the Creative Commons Share-Alike liscence agreement.
//[url]http://creativecommons.org/licenses/by-sa/3.0/[/url]
//If you have questions, or find errors in the code, or wish to make a contribution towards future development, please contact me at matthewssrpat "at" comcast.net
//******************************************************************************************************
// six channel version of GrowGreen LED Controller       //
//                     2/10/17  adafruit version                           //
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <RotaryEncoder.h>
#include <Button.h>
#include <DS3231_Simple.h>
#include <Wire.h>
#include <EEPROM.h>
#include "Adafruit_LiquidCrystal.h"

#define PinA 2
#define PinB 3
#define PinButton 4
#define MAX_CHANNELS 6

char p_buffer[21];
#define P(str) (strcpy_P(p_buffer, PSTR(str)), p_buffer)
#define ON 0x1
#define OFF 0x0
#define menuMax 3  // max menu items

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_LiquidCrystal lcd(0);

// set up backlight
// create the menu counter
volatile int position = 0;
int minCounter = 0;         // counter that resets at midnight.
int oldMinCounter = 0;      // counter that resets at midnight.
int CurrentMinuteCtr = 0;
int oldMinute = 0;
int buttonPressTime = 0;
int debounceTime = 0;
int lastReportedPos = 0;
int oldButtonState = 0;
//create the plus and minus navigation delay counter with its initial maximum of 250.
byte btnMaxDelay = 200;
byte btnMinDelay = 25;
byte btnMaxIteration = 5;
byte btnCurrIteration;
byte buttonPressed = 0;
// menu options variables
int modeSelect = 0;
char *modeOverride = "AUTO";
//Menu variables
int mainMenuPos = 1;
int channelMenuPos = 0;
int channelMenuChoice = 0;

boolean channelEdit = false;
boolean clockEdit = false;

boolean clockSelect = false;
// setup[ ds3231  Real Time Clock
DS3231_Simple Clock;
// backlight on/off

// lcd screen variable
int LCD_R=20;  // lcd rows
int LCD_C=4;  // lcd columns
int hr, mn, sc, mnth, Date, yr, temperature;
boolean override = false;
byte overmenu = 0;
int overpercent = 0;
boolean Show = true;
boolean ShowClean = false;
// Led Array
typedef struct {
int versionCheck; // Just a number to check to ensure we've read in valid settings!  
int StartMins; // minute to start this channel.
int PhotoPeriod; // photoperiod in minutes for this channel.
int ledVal; // current intensity for this channel, as a percentage
int ledMin; // min intensity for this channel, as a percentage
int ledMax; // max intensity for this channel, as a percentage
int FadeDuration; // duration of the fade on and off for sunrise and sunset for this channel.
} channelVals_t;
// time  array
channelVals_t channel[MAX_CHANNELS];
typedef struct {
int Hours;    // current hour
int Minutes;  // current minute
int Seconds;  // current second
int Date;     // current Day
int Month;    // current Month
int Year;     // Current Year
} timeVals_t;

timeVals_t DateAndTime[1];

int LedPin[MAX_CHANNELS] = {6,7,8,9,10,11};
boolean Inverted[MAX_CHANNELS] = {false,false,false,false,false,false};
char *Desc[MAX_CHANNELS]={"Channel 1", "Channel 2","Channel 3","Channel 4","Channel 5","Channel 6"};
int EEPROM_ADD[MAX_CHANNELS]={0,0,0,0,0,0};
  
// setup Pins for output for PWM signal to drive relay on/off

// initiate encoder and push button
RotaryEncoder knob( PinA, PinB); // pinA, pinB, initiate encoder
Button button = Button( PinButton, INPUT_PULLUP); // initiate button

//Bounce bouncer = Bounce( PinButton,5 ); // Instantiate a Bounce object with a 5 millisecond debounce time
//
//------------------------------ RTC Functions ----------------------//
// read ds3231 rtc and temperature
void ReadDS3231()
{
  DateTime MyDateTime;
  MyDateTime = Clock.read();
  MyDateAndTime[0].Hours = MyDateTime.Hour;
  MyDateAndTime[0].Minutes = MyDateTime.Minute;
  MyDateAndTime[0].Seconds = MyDateTime.Second;
  MyDateAndTime[0].Month = MyDateTime.Month;
  MyDateAndTime[0].Date = MyDateTime.Day;
  MyDateAndTime[0].Year = MyDateTime.Year;

}
//-----------print lcd functions ------------------/
void PrintBegin()
{
  lcd.begin(LCD_R, LCD_C);   // start the library  
  lcd.setBacklight(ON);
   
}
//  setup serial printer for use
void SPrintBegin()
  {
  Serial.begin(9600);
  Serial.print(P("Ready")); 
  }

// clean lcd screen
void cleanScreen()
{
  lcd.setCursor(0,0);
  lcd.print(P("                    ")); 
  lcd.setCursor(0,1);
  lcd.print(P("                    "));
  lcd.setCursor(0,2);
  lcd.print(P("                    "));
  lcd.setCursor(0,3);
  lcd.print(P("                    "));
}
void cleanMainScreen()
{
  lcd.setCursor(0,0);
  lcd.print(P("    Main Menu       "));
  lcd.setCursor(0,1); 
  lcd.print(P("                    "));
  lcd.setCursor(0,2);
  lcd.print(P("                    "));
  lcd.setCursor(0,3);
  lcd.print(P("                    "));
}
// print setup splash screen
void splash_screen() {
lcd.print(P("GrowGreen CoB 6-ch  "));
lcd.setCursor(0,1);
lcd.print(P("LED Controller V.3  "));
delay(5000);
lcd.clear();  
}
void displayChannel() {
  lcd.setCursor(0,1);
  lcd.print(P("Start:  :    Max:"));
  lcd.setCursor(0,2);
  lcd.print(P("Stop :  :    Min:"));
  lcd.setCursor(0,3);
  lcd.print(P("Fade :  "));
}

//---------- LED Functions --------------------------------------/
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
//-------------------------Display Functions ------------------------------//
// format a number of minutes into a readable time (24 hr format)
void printMins(int mins,       //time in minutes to print
               boolean ampm    //print am/pm?
              )  {
 
  int hr = (mins%1440)/60;
  int mn = mins%60;
  if(hr<10){
      lcd.print(P(" "));
    }
    lcd.print(hr);
    lcd.print(P(":"));
    if(mn<10){
      lcd.print(P("0"));
    }
   
    lcd.print(mn);
   
}
// format a number of minutes into a readable time (24 hr format)
void printMin(int mins       //time in minutes to print
               )  
  {
 
   
    if(mins<10){
      lcd.print(P("0"));
    }
   
    lcd.print(mins);
   
}

// format hours, mins, secs into a readable time (24 hr format)
void printHMS (byte hr,
               byte mn,
               byte sec      //time to print
              )
  
{
    
    if(hr<10){
      lcd.print(P(" "));
    }
   
    lcd.print(hr);
    lcd.print(P(":"));
    if(mn<10){
      lcd.print(P(" "));
    }
    
    lcd.print(mn);
    lcd.print(P(":"));
    if(sec<10){
      lcd.print(P("0"));
    }
     
     lcd.print(sec);
   }
// format hours, mins, secs into a readable time (24 hr format)
void printHrs (byte hr
                    //time to print
              )  
          
{
    
    if(hr<10){
      lcd.print(P(" "));
    }
    lcd.print(hr);
    //lcd.print(P(":"));
    
}
// format hours, mins, secs into a readable time (24 hr format)
void printDate (byte dt
                    //time to print
              )  
          
{
    
    if(dt<10){
      lcd.print(P(" "));
    }
    lcd.print(dt);
    lcd.print(P("/"));
    
}
//------------- EEPROM functions----------------//

// ---------------------- channel functions ----------------------//
//----------------------------- start time -----------------------//
void doStartTime(int val) {
//set start time
  lcd.cursor();
  lcd.blink();
knob.attachVariable(channel[val].StartMins);
displayChannel();
while (!channelEdit){
     lcd.setCursor(0,0);
     lcd.print(Desc[val]);
     
     lcd.setCursor(12,0);
     lcd.print(P("   START"));
     lcd.setCursor(6,1);
     printMins(channel[val].StartMins, false);  
     if(channel[val].StartMins < 1440){
        channel[val].StartMins++;
    }
    if(channel[val].StartMins > 0){
        channel[val].StartMins--;
   }
    if (button.uniquePress()){
     knob.detachVariable();
     cleanScreen();
    
     channelEdit = true;
   }
   if (button.isPressed()){
   
     knob.detachVariable(); 
     channelEdit = true;
   }
  
} // end while
  lcd.noCursor();
  lcd.noBlink();
  channelEdit = true;
      int EEPROM_address = (val * sizeof(channelVals_t));
      EEPROM.put(EEPROM_address, channel[val]);
}
//------------------- end tieme -------------------------------//
void doEndTime(int val) {
lcd.cursor();
lcd.blink();
knob.attachVariable(channel[val].PhotoPeriod);
displayChannel();
while (!channelEdit){
  lcd.setCursor(0,0);
  lcd.print(Desc[val]);
  
  lcd.setCursor(12,0);
  lcd.print(P("     END"));
  lcd.setCursor(6,2);
  printMins(channel[val].StartMins+channel[val].PhotoPeriod, false);
   if(channel[val].PhotoPeriod < 1440){
      channel[val].PhotoPeriod++;
 
   }
   else {
  
   }
    
   if(channel[val].PhotoPeriod > 0){
     channel[val].PhotoPeriod--;

   }
   else {
     //channel[val].PhotoPeriod = 1439;
   }
 if (button.uniquePress()){
     knob.detachVariable();
     cleanScreen();
   
     channelEdit = true;
   }    
 if (button.isPressed()){

 
  knob.detachVariable();
  channelEdit = true;
  }

} // end while
lcd.noCursor();
lcd.noBlink();
  int EEPROM_address = (val * sizeof(channelVals_t));
  EEPROM.put(EEPROM_address, channel[val]);

}

//-----------------FadeDuration -------------------------------//
void doFadeDuration(int val) {
lcd.cursor();
lcd.blink();

knob.attachVariable(channel[val].FadeDuration);  
displayChannel();
//set fade duration
while (!channelEdit){
  lcd.setCursor(0,0);
  lcd.print(Desc[val]);
  
  lcd.setCursor(12,0);
  lcd.print(P("    FADE"));
  lcd.setCursor(6,3);
  printMins(channel[val].FadeDuration, false);
  if (channel[val].FadeDuration < channel[val].PhotoPeriod/2 || channel[val].FadeDuration > 1) {
    channel[val].FadeDuration++;
   
    
  }
  if (channel[val].FadeDuration > 1) {
    channel[val].FadeDuration--;

    
  }
  if (button.uniquePress()){
      knob.detachVariable();
      cleanScreen();
      channelEdit = true;
  }
 if (button.isPressed()){

 knob.detachVariable();
 channelEdit = true;
 }

 
} // end while
lcd.noCursor();
lcd.noBlink();
  int EEPROM_address = (val * sizeof(channelVals_t));
  EEPROM.put(EEPROM_address, channel[val]);
}
// ------------------doMaxIntensity ---------------------------//
void doMaxIntensity(int val) {
lcd.cursor();
lcd.blink();
displayChannel();
knob.attachVariable(channel[val].ledMax);
//set intensity
while (!channelEdit){
  lcd.setCursor(0,0);
  lcd.print(Desc[val]);
  displayChannel();
  lcd.setCursor(12,0);
  lcd.print(P("     MAX"));
  lcd.setCursor(17,1);
  lcd.print(channel[val].ledMax);
  if( channel[val].ledMax < 101){
      channel[val].ledMax++;

     
  }
  if(channel[val].ledMax > 0){
     channel[val].ledMax--;

   
  }
  if (button.uniquePress())
   {
    knob.detachVariable();
    cleanScreen();

   channelEdit = true;
 }
  if (button.isPressed()){

 knob.detachVariable();
 channelEdit = true;
 }   
 
} // end while
 
lcd.noCursor();
lcd.noBlink();
  int EEPROM_address = (val * sizeof(channelVals_t));
  EEPROM.put(EEPROM_address, channel[val]);
}
// -----------DoMinIntensity --------------------------------//
void doMinIntensity(int val) {
lcd.cursor();
lcd.blink();
displayChannel();
//set intensity
//byte buttonPress = button.ReadButton();
knob.attachVariable(channel[val].ledMin);
while (!channelEdit){
  lcd.setCursor(0,0);
  lcd.print(Desc[val]);
 
  lcd.setCursor(12,0);
  lcd.print(P("     MIN"));
  lcd.setCursor(17,2);
  lcd.print(channel[val].ledMin);
  if( channel[val].ledMin < 100){
        channel[val].ledMin++;
        
      }
      if(channel[val].ledMin > 0){
        channel[val].ledMin--;
       
      }
    if (button.uniquePress()){
    knob.detachVariable();
    cleanScreen();

    channelEdit = true;
   }   
    if (button.isPressed()){

       knob.detachVariable();
       channelEdit = true;
   } 
  
} // end while
lcd.noCursor();
lcd.noBlink();
  int EEPROM_address = (val * sizeof(channelVals_t));
  EEPROM.put(EEPROM_address, channel[val]);
} 
//----------------------- clock functions()-----------------------//
// set Hour
void SetHour() {
//set hours
knob.attachVariable(MyDateAndTime[0].Hours);
while (!clockEdit) {
  lcd.setCursor(0,0);
  lcd.print (P("Set Hour:          "));
  lcd.setCursor(0,1);
  lcd.print(P("Time:"));
  lcd.setCursor(0,3); 
  lcd.print(P("Push to select      "));  
  lcd.setCursor(6,1); 
  printHrs(MyDateAndTime[0].Hours);
  if (MyDateAndTime[0].Hours < 25) {
     MyDateAndTime[0].Hours++;
   }
   else {
   MyDateAndTime[0].Hours = 0;
   }
   
  if (MyDateAndTime[0].Hours > 0) {
    MyDateAndTime[0].Hours--;
  }
  else {
    MyDateAndTime[0].Hours = 24;
  }
 
  if(button.isPressed()){
    knob.detachVariable();
    clockEdit = true;
    clockSelect = true;
}
if (button.uniquePress()){
  knob.detachVariable();
  cleanScreen();
  knob.attachVariable(mainMenuPos);
  clockEdit = true;
  clockSelect = true;
}

  }// end while
  
lcd.noCursor();
lcd.noBlink();
}
// set minutes
void SetMinute() {
//set minutes
lcd.cursor();
lcd.blink();

knob.attachVariable(MyDateAndTime[0].Minutes);
while (!clockEdit) {
  lcd.setCursor(0,0);
  lcd.print (P("Set Minute:        "));
  lcd.setCursor(0,1);
  lcd.print(P("Time:"));
  lcd.setCursor(0,3); 
  lcd.print(P("Push to select      "));  
  lcd.setCursor(9,1);
  printMin(MyDateAndTime[0].Minutes);
  if (MyDateAndTime[0].Minutes < 60) {
    MyDateAndTime[0].Minutes++;
  }
  else {
    MyDateAndTime[0].Minutes = 0;
  }

  if (MyDateAndTime[0].Minutes > 0) {
    MyDateAndTime[0].Minutes--;
  }
  else {
    MyDateAndTime[0].Minutes = 59;
  }
 
 if(button.isPressed()){
  knob.detachVariable();
  clockEdit = true;
  clockSelect = true;
}
if (button.uniquePress()){
   knob.detachVariable();
   cleanScreen();
   knob.attachVariable(mainMenuPos);
   clockEdit = true;
   clockSelect = true;
 }

}// end while

lcd.noCursor();
lcd.noBlink();
}

// set Month
void SetMonth() {
knob.attachVariable(MyDateAndTime[0].Month);
// print date format
while (!clockEdit) {
  lcd.setCursor(0,0);
  lcd.print (P("Set Month:         "));
  lcd.setCursor(0,2);
  lcd.print(P("Date:"));
  lcd.setCursor(0,3); 
  lcd.print(P("Push to select      "));  
  lcd.setCursor(6,2);
  printDate(MyDateAndTime[0].Month);
  if (MyDateAndTime[0].Month < 13) {
    MyDateAndTime[0].Month++;
  }
  else {
   MyDateAndTime[0].Month = 0;
  }

  if (MyDateAndTime[0].Month > 0) {
    MyDateAndTime[0].Month--;
  }
  else {
    MyDateAndTime[0].Month = 12;
  }


if(button.isPressed()){
  knob.detachVariable();
  clockEdit = true;
  clockSelect = true;
}
if (button.uniquePress()){
 knob.detachVariable();
 cleanScreen();
 knob.attachVariable(mainMenuPos);
 clockEdit = true;
 clockSelect = true;
}

}
lcd.noCursor();
lcd.noBlink();
}
// set Date or DayOfMonth
void SetDate() {
knob.attachVariable(MyDateAndTime[0].Date);
// print date format
//
while (!clockEdit) {
  lcd.setCursor(0,0);
  lcd.print (P("Set Day:           "));
  lcd.setCursor(0,2);
  lcd.print(P("Date:"));
  lcd.setCursor(0,3); 
  lcd.print(P("Push to select      "));  
  lcd.setCursor(9,2);
  printDate(MyDateAndTime[0].Date);
  if (MyDateAndTime[0].Date < 32) {
    MyDateAndTime[0].Date++;
  }
  else {
    MyDateAndTime[0].Date = 0;
  }
  
  if (MyDateAndTime[0].Date > 0) {
    MyDateAndTime[0].Date--;
  }
  else {
    MyDateAndTime[0].Date = 31;
  }
 
  if(button.isPressed()){
  knob.detachVariable();
  clockEdit = true;
  clockSelect = true;
}
if (button.uniquePress()){
  knob.detachVariable();
  cleanScreen();
  knob.attachVariable(mainMenuPos);
  clockEdit = true;
  clockSelect = true;
 } 

}// end while

lcd.noCursor();
lcd.noBlink();
}
// Set year
void SetYear() {
knob.attachVariable(MyDateAndTime[0].Year);
// print date format
while (!clockEdit) {
  lcd.setCursor(0,0);
  lcd.print (P("Set Year:             "));
  lcd.setCursor(0,2);
  lcd.print(P("Date:"));
  lcd.setCursor(0,3); 
  lcd.print(P("Push to select      "));   
  lcd.setCursor(12,2);
  lcd.print(MyDateAndTime[0].Year);
  if (MyDateAndTime[0].Year < 99) {
    MyDateAndTime[0].Year++;
  }
  else {
    MyDateAndTime[0].Year = 0;
  }
 


  if (MyDateAndTime[0].Year > 0) {
    MyDateAndTime[0].Year--;
  }
  else {
    MyDateAndTime[0].Year = 99;
  }
 
  if(button.isPressed()){
    knob.detachVariable();
    clockEdit = true;
    clockSelect = true;
}
if (button.uniquePress()){ 
  knob.detachVariable();
  cleanScreen();
  knob.attachVariable(mainMenuPos);
  clockEdit = true;
  clockSelect = true;
 }
}

lcd.noCursor();
lcd.noBlink();
}

// oversetAll not used for basic LED drivers
void ovrSetAll(int pct){
     for (int i = 0; i < MAX_CHANNELS; i++) {
    analogWrite(channel[i].ledVal,map(pct,0,100,0,255));
  }
}
// MainmenuLoop 
void MainMenuLoop (){
  
   if (!ShowClean){
      cleanScreen();
      ShowClean = true;
   }
   lcd.setCursor(0,0);
   lcd.print(P(" Main   Menu        ")); 
   lcd.setCursor(0,3); 
   lcd.print(P("Turn to select      "));    
   lcd.noCursor();
   lcd.noBlink();
   Show = false;
   knob.attachVariable(mainMenuPos);
   if (lastReportedPos != mainMenuPos) {
     if(mainMenuPos > menuMax){ mainMenuPos = 1;}
       if(mainMenuPos < 1){mainMenuPos = menuMax;}
      
     lcd.setCursor(0,1);
     {displayMenu(mainMenuPos);}
    lastReportedPos = mainMenuPos;
    } 
    if (millis () - buttonPressTime >= debounceTime){ // debounce
      buttonPressTime = millis ();  // when we closed the switch 
      if (button.uniquePress()) {
      // exit and return to main loop
      knob.detachVariable();
      Show = true;
      lcd.noCursor();
      lcd.noBlink();
      }
      if (button.isPressed() ){
          {selectMenu(mainMenuPos);}
          ShowClean = false;
        
      }
      
    }  // end if debounce time up
    //Main menu section
   knob.detachVariable();
    // end if mainMenuPos
  
}
// end MainMenuLoop
// -------------------edit functions  -----------------------//

// -------------------displayMenu ---------------------------//
void displayMenu(int x) {
  
    switch(x){
      case 1:
        lcd.setCursor(0,0);
        lcd.print(P(" Main   Menu        ")); 
        lcd.setCursor(0,1);      
        lcd.print(P(" Modify  Channels   ")); 
        lcd.setCursor(0,3); 
        lcd.print(P("Push to select      "));    
      break; 
      case 2:
        lcd.setCursor(0,0);
        lcd.print(P(" Main   Menu        ")); 
        lcd.setCursor(0,1);
        lcd.print(P(" Modify Clock       ")); 
        lcd.setCursor(0,3); 
        lcd.print(P("Push to select     "));   
        break;
      case 3:
         lcd.setCursor(0,0);
         lcd.print(P(" Main   Menu       ")); 
         lcd.setCursor(0,1);        
         lcd.print(P("     Exit          "));
         lcd.setCursor(0,3); 
         lcd.print(P("Push to select      "));   
         break;
      } // end switch
} // end displayMenu     
//----------------Selection menu ---------------------------------//
void selectMenu(int x) {
DateTime MyDateTime;
MyDateTime = Clock.read();
     switch (x) {
      case 1:
        knob.detachVariable();
        lcd.noBlink();
        lcd.noCursor();
        cleanScreen();
        displayChannel();
        knob.attachVariable(channelMenuPos);
      while (!button.isPressed()){
        if (lastReportedPos != channelMenuPos) {
          if(channelMenuPos > (MAX_CHANNELS-1)){ channelMenuPos = 0;}
            lcd.setCursor(0,0);
            lcd.print(Desc[channelMenuPos]);
            lcd.setCursor(6,1);
            printMins(channel[channelMenuPos].StartMins, false);
            lcd.setCursor(6,2);
            printMins(channel[channelMenuPos].StartMins+channel[channelMenuPos].PhotoPeriod, false);
            lcd.setCursor(6,3);
            printMins(channel[channelMenuPos].FadeDuration, false);
            lcd.setCursor(17,1);
            lcd.print(channel[channelMenuPos].ledMax);
            lcd.setCursor(17,2);
            lcd.print(channel[channelMenuPos].ledMin);
            lastReportedPos = channelMenuPos;
          } // end if lastreportedpos
      }
          if(button.uniquePress()){
              knob.detachVariable();
              lcd.noCursor();
              lcd.noBlink();
              cleanScreen();
              knob.attachVariable(mainMenuPos);
            }
          if (button.isPressed()){
              channelEdit = false;
              channelMenuChoice = channelMenuPos;
              knob.detachVariable();
              displayChannel();
              //lcd.setCursor(6,1);
              //printMins(channel[channelMenuChoice].StartMins, false);
              doStartTime(channelMenuChoice);
              channelEdit = false;
              doEndTime(channelMenuChoice);
              channelEdit = false;
              doFadeDuration(channelMenuChoice);
              channelEdit = false;
              doMaxIntensity(channelMenuChoice);
              channelEdit = false;
              doMinIntensity(channelMenuChoice); 
              channelEdit = false;

              cleanScreen();
              knob.detachVariable();
              lcd.noCursor();
              lcd.noBlink();
              knob.attachVariable(mainMenuPos);
            }
            
      break;
      case 2:
        knob.detachVariable();
        lcd.noBlink();
        lcd.cursor();
        cleanScreen();
        clockEdit = false;
        lcd.setCursor(0,0);
        lcd.print(P("Set Time/Date:       "));
        lcd.setCursor(0,1);
        lcd.print(P("Time:"));
        lcd.setCursor(0,2);
        lcd.print(P("Date:"));
        // show time
        llcd.setCursor(6,1);
        printHrs(MyDateAndTime[0].Hours);
        lcd.setCursor(8,1);
        lcd.print(P(":"));
        lcd.setCursor(9,1);
        printMin(MyDateAndTime[0].Minutes);
        lcd.setCursor(6,2);
        printDate(MyDateAndTime[0].Month);
        lcd.setCursor(9,2);
        printDate(MyDateAndTime[0].Date);
        lcd.setCursor(12,2);
        lcd.print(MyDateAndTime[0].Year);
        
        if (button.uniquePress()){
           knob.detachVariable();
           clockEdit = false;
           cleanScreen();
           lcd.noCursor();
           lcd.noBlink();
           knob.attachVariable(mainMenuPos);
        }
        //  set start time 
          SetHour(MyDateAndTime[0].Hours);
          clockEdit = false;
          SetMinute(MyDateAndTime[0].Minutes);
          clockEdit = false;
          SetDate(MyDateAndTime[0].Date);
          clockEdit = false;
          clockSelect = true;
          SetMonth(MyDateAndTime[0].Month);
          clockEdit = false;
          SetYear(MyDateAndTime[0].Year);
          clockEdit = false;
          cleanScreen();
          knob.detachVariable();
          // update clock rtc from array after update
          MyDateTime.Hour = MyDateAndTime[0].Hours;
          MyDateTime.Minute = MyDateAndTime[0].Minutes;
          MyDateTime.Second = MyDateAndTime[0].Seconds;
          MyDateTime.Month = MyDateAndTime[0].Month;
          MyDateTime.Day = MyDateAndTime[0].Date;
          MyDateTime.Year = MyDateAndTime[0].Year;
          MyDatetime = Clock.write();
          lcd.noCursor();
          lcd.noBlink();
          knob.attachVariable(mainMenuPos);
        
         break;
       case 3:
         
        lcd.setCursor(0,1);
        lcd.print(P("Exit                "));
        knob.detachVariable();
        Show = true;
        lcd.noCursor();
        lcd.noBlink();
        cleanScreen();
        break;
    } // end switch
  
}  // end SelectMenu 
// -------------------------- LED Functions ------------------------------------//
// --------------------------- check & set fade durations ----------------------//
void set_fade(){ 
   char updateFlag = 'N';
   for (int i = 0; i < MAX_CHANNELS; i++) { //check & adjust fade durations
   if(channel[i].FadeDuration > channel[i].PhotoPeriod/2 && channel[i].PhotoPeriod >0) {channel[i].FadeDuration = channel[i].PhotoPeriod/2;updateFlag = 'Y';}
   if(channel[i].FadeDuration < 1) {channel[i].FadeDuration = 1;updateFlag = 'Y';}
   if (updateFlag == 'Y') {
      updateFlag = 'N';
      int EEPROM_address = (i * sizeof(channelVals_t));
      EEPROM.put(EEPROM_ADD[EEPROM_address], channel[i]);
   }
}
}
// end set LED fade
// ---------------------------- set LED outputs -------------------------------//
void set_output() {
 if(!override){
  for (int i = 0; i < MAX_CHANNELS; i++){
      channel[i].ledVal = setLed(minCounter, LedPin[i], channel[i].StartMins, channel[i].PhotoPeriod, channel[i].FadeDuration, channel[i].ledMax, Inverted[i]);}
  }
  else{
    ovrSetAll(overpercent);
  }
}
// end set outputs
// -----------------showChannelValues  print channel values to lcd-------------------//
void showChannelValues() { // display the current intensity for each led channel
float MyFloatTemperature;
MyFloatTemperature = Clock.getTemperatureFloat();
float c = MyFloatTemperature; 
float f = 1.8*c+32;
ReadDS3231();
int lastReportedPos = 0;
int oldButtonState = 0;
char buffer[21];
int DSFTemp=(int)round(f);
lcd.noCursor();
lcd.noBlink();
lcd.setCursor(0,0);
sprintf(buffer, "%2d:%02d:%02d        %3d",MyDateAndTime[0].Hours,MyDateAndTime[0].Minutes, MyDateAndTime[0].Seconds, DSFTemp);
lcd.print(buffer);
lcd.setCursor(19,0);
lcd.print((char)223);
lcd.setCursor(0,1);
sprintf(buffer, "%3d   %3d   %3d", channel[0].ledVal, channel[1].ledVal, channel[2].ledVal);
lcd.print(buffer);
lcd.setCursor(0,2);
sprintf(buffer, "%3d   %3d   %3d", channel[3].ledVal, channel[4].ledVal, channel[5].ledVal);
lcd.print(buffer);
lcd.setCursor(0,3);
sprintf(buffer, "%2d/%02d/%02d ", MyDateAndTime[0].Month, MyDateAndTime[0].Date, MyDateAndTime[0].Year);
lcd.print(buffer);
}  
// end show channel values
// read settingsfrom eeprom
void readSettings() {
int EEPROM_address = 0;
for (byte i = 0; i < MAX_CHANNELS; i++) {
EEPROM.get(EEPROM_address, channel);
EEPROM_ADD[i] = EEPROM_address;
if (channel[i].versionCheck != 9898) {
// Set default values for the settings and save them to EEPROM
channel[i].versionCheck = 9898;
switch (i) {
case 0:
channel[0].StartMins = 360;
channel[0].PhotoPeriod = 720;
channel[0].ledVal = 100;
channel[0].ledMin = 0;
channel[0].ledMax = 100;
channel[0].FadeDuration = 1;
break;
case 1:
channel[1].StartMins = 360;
channel[1].PhotoPeriod = 720;
channel[1].ledVal = 100;
channel[1].ledMin = 0;
channel[1].ledMax = 100;
channel[1].FadeDuration = 1;
break;
case 2:
channel[2].StartMins = 360;
channel[2].PhotoPeriod = 720;
channel[2].ledVal = 100;
channel[2].ledMin = 0;
channel[2].ledMax = 100;
channel[2].FadeDuration = 1;
break;
case 3:
channel[3].StartMins = 360;
channel[3].PhotoPeriod = 720;
channel[3].ledVal = 100;
channel[3].ledMin = 0;
channel[3].ledMax = 100;
channel[3].FadeDuration = 1;
break;
case 4:
channel[4].StartMins = 360;
channel[4].PhotoPeriod = 720;
channel[4].ledVal = 100;
channel[4].ledMin = 0;
channel[4].ledMax = 100;
channel[4].FadeDuration = 1;

break;
case 5:
channel[5].StartMins = 1065;
channel[5].PhotoPeriod = 30;
channel[5].ledVal = 100;
channel[5].ledMin = 0;
channel[5].ledMax = 100;
channel[5].FadeDuration = 1;

break;

break;
} // end switch
EEPROM.put(EEPROM_ADD[i], channel[i]);
} // end if
} // end for
}
// end read settings

// end
// main setup
void setup()
{
// Initialize channel variables. Set LED channel pin and retrieve values from EEPROM
readSettings();
Wire.begin();
ReadDS3231();
// setup lcd screen object
PrintBegin();
// set serial printer
SPrintBegin();
//splash screen
splash_screen();
} // end setup
// Main loop
void loop(){ 
//check & set any time functions
  DateTime MyDateTime;
  ReadDS3231();
  oldMinCounter = minCounter;
  minCounter = MyDateAndTime[0].Hours * 60 + MyDateAndTime[0].Minutes;

  if(button.uniquePress()){
     //lcd.setBacklight(ON);
     Show = false;
  }
  if (Show == true)
  {
    set_fade();
    set_output();
    showChannelValues();
 }
 if (Show == false){
    MainMenuLoop();   
 }

 
 
}  // end loop


