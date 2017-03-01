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
//  MEMBER FUNCTIONS

#include "RotaryEncoder.h"

int* RotaryEncoder::attached_val = NULL;

boolean RotaryEncoder::A_set = false;
boolean RotaryEncoder::B_set = false;
boolean RotaryEncoder::rotating = false;
byte RotaryEncoder::bit_maskA=0;
byte RotaryEncoder::bit_maskB=0;
volatile byte* RotaryEncoder::registerA=0;
volatile byte* RotaryEncoder::registerB=0;
byte RotaryEncoder::PinA = 0;
byte RotaryEncoder::PinB = 0;

RotaryEncoder::RotaryEncoder()
{
  // do nothing
}
 
RotaryEncoder::RotaryEncoder(RotaryEncoder& _object)
{
  // do nothing
  
}

RotaryEncoder::RotaryEncoder(byte _pin1, byte _pin2)
{
 
 setupPins(_pin1, _pin2);
 
 
 

}

void RotaryEncoder::doEncoderA(){
   if ( RotaryEncoder::rotating ) delay (2); 
    if(digitalRead(RotaryEncoder::PinA) != RotaryEncoder::A_set) {
      RotaryEncoder::A_set = !RotaryEncoder::A_set;
       if ( RotaryEncoder::A_set && !RotaryEncoder::B_set)
          *RotaryEncoder::attached_val+=1;
           RotaryEncoder::rotating = false;
       delay(2);    
    }
}
void RotaryEncoder::doEncoderB(){
  if ( RotaryEncoder::rotating ) delay (2);  
    if(digitalRead(RotaryEncoder::PinB) != RotaryEncoder::B_set) {
      RotaryEncoder::B_set = !RotaryEncoder::B_set;
      if (RotaryEncoder::B_set && !RotaryEncoder::A_set)
        *RotaryEncoder::attached_val-=1;
        RotaryEncoder::rotating = false;
        if(*RotaryEncoder::attached_val < 0)
           *RotaryEncoder::attached_val = 0;
      delay(2);   
    }
} 

void RotaryEncoder::attachFunction()
{
attachInterrupt(0, RotaryEncoder::doEncoderA, CHANGE);
attachInterrupt(1, RotaryEncoder::doEncoderB, CHANGE);

}

void RotaryEncoder::detachFunction()
{
detachInterrupt(0);
detachInterrupt(1);

}
void RotaryEncoder::detachVariable()
{
  detachFunction();
  attached_val = NULL;
  
}
void RotaryEncoder::attachVariable(int& _var)
{
  attached_val = &(_var);
  attachFunction();
}
void RotaryEncoder::setupPins(byte _pin1, byte _pin2)
{
  byte portA;
  byte portB;

  pinMode(_pin1, INPUT_PULLUP);
  pinMode(_pin2, INPUT_PULLUP);
  digitalWrite(_pin1, HIGH);
  digitalWrite(_pin2, HIGH);
  

  portA=digitalPinToPort(_pin1);
  portB=digitalPinToPort(_pin2);
  
  RotaryEncoder::PinA = _pin1;
  RotaryEncoder::PinB = _pin2;
  RotaryEncoder::bit_maskA = digitalPinToBitMask(_pin1);
  RotaryEncoder::bit_maskB = digitalPinToBitMask(_pin2);
  RotaryEncoder::registerA = portInputRegister(portA);
  RotaryEncoder::registerB = portInputRegister(portB);
  
  
}
