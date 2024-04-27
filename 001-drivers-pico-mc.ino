// main file of the project to:
// - to switch on/off the custom cornering lights;
// - to move servo with my GoPro camera attached to it;
// based on the Mercedes S212 E300 Hybrid, 
//   the sensor data aquired through the CAN bus frames

#include "canbusreader.h"  // file to process CAN bus messages and get the values
#include "servocontrol.h"  // file to control GoPro servo motor dep. on st. wheel angle
#include "corneringlights.h"  // to control the cornering lights

bool Debug = false;  // onboard neopix is OFF, set to true for debugging

// trying to smooth servo movements with millis()
unsigned long canTime = 0;
unsigned long servoTime = 0;
unsigned long pixelsTime = 0;

void setup()  // rp2040 1st core setup
{
  SetupCan();     // defined in canfunctions.h
  delay(1000);
}

void loop()  // rp2040 1st core loop
{
  // process CAN bus msgs and return the steering wheel angle (value betw. 3100 & 5100)
  // this function is in canbusreader.h
  //
  //   27-04-2024:
  //   COMMENTED millis() to run ReadCAN() without any delays
  //    because otherwise it does not always pick up CAN frames
  //    so now ReadCAN runs without any delays
//  if( millis() - canTime >= 10 )
//  {
    ReadCAN();  
//    canTime = millis();
//  }
}

///////////////////////////////////////
void setup1()  // rp2040 2nd core setup
{
  SetupServo();   // defined in servocontrol.h

  SetupCorneringLights();

  // Set the on-board Neopixel pin to output and high
  //   in order to enable the NeoPixels. 
  if( Debug )
  {
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);
    pixels.begin();            // init. NeoPixel object
    pixels.setBrightness(5);  // NeoPixel's brightness
  }
  delay(1000);
}

void loop1()  // rp2040 2nd core loop
{
  // if it's daylight,
  // than move our GoPro's servo depending on
  // the st. wheel angle, 
  // else we don't need to control servo at night
  if( !nightTime )
  {
    if( millis() - servoTime >= 10 )
    {
      MoveGopro();  // this function is in servocontrol.h
      servoTime = millis();
    }
  }

  if( nightTime ) RunCorneringLights();

  if( Debug )
  {
    if( millis() - pixelsTime >= 100 )
    {
      pixels.fill(currentPixColor); // set the pix color
      pixels.show();                // light up the pix
      pixelsTime = millis();
    }
  }
}
