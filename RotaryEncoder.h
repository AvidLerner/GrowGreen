/**************************************************************************************************
***************************************************************************************************
***************************************************************************************************

RotaryEncoder 

ver. 1.1

Updated 8-5-2016


This library is being shared under the Creative Commons Share-Alike liscence agreement.

[url]http://creativecommons.org/licenses/by-sa/3.0/[/url]

If you have questions, or find errors in the code, or wish to make a contribution towards future development, please contact me at matthewssrpat "at" comcast.net


******************************************************************************************************
******************************************************************************************************
******************************************************************************************************/
#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "arduino.h"



#include "pins_arduino.h"
/* ======================================================================================
********* Class Definition *************
======================================================================================*/
class RotaryEncoder {

private:
static boolean A_set;              
static boolean B_set;
static boolean rotating;
static byte bit_maskA;
static byte bit_maskB;
static volatile byte* registerA;
static volatile byte* registerB;
static byte PinA;
static byte PinB;
byte bit_mask;
byte port;
volatile byte* output_register;
volatile byte* input_register;
//unsigned char mode_register;
uint8_t pin;
uint8_t mode;
uint8_t state;
boolean timedOut;

public:
static int* attached_val;
int buttonPress;
unsigned int lastReportedPos;
static unsigned int encoderPos;

int debounce; 
int DCgap; 
int holdTime; 
int longHoldTime; 
static int retVal;
boolean buttonVal; 
boolean buttonLast; 
boolean DCwaiting; 
boolean DConUp; 
boolean singleOK; 
long downTime; 
long upTime; 
boolean ignoreUp; 
boolean waitForUp; 
boolean holdEventPast; 
boolean longHoldEventPast;
long lastDebounceTime;
long debounceDelay;
int button_State;
   
public:
RotaryEncoder();

RotaryEncoder(RotaryEncoder&);
RotaryEncoder(byte, byte);
void attachVariable(int&);
void detachVariable();

private:
static void setupPins(byte, byte);
void attachFunction();
void detachFunction();
static void doEncoderA();
static void doEncoderB();


};

#endif
