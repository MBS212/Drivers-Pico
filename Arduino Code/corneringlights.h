/*****************************************
this file contains fuction that control
which cornering light is on or off

both ON if going backward
left or right ON dep. on st. wheel angle and trun indicators

29.04.24 - moved the nightTime condition from the loop1() to here

02.12.24 - added headlights == 'L' cond. that will allow me to switch off corn. lights
           together with the headlights if I don't want corn. lights
*/
#define OFFSET_DEGREES 120     // how many degrees from center, at which the corn. lights are to be sw. ON/OFF
#define MAX_SPEED 15          // maximum speed in kmh the cornering lights to be enabled

#include <Adafruit_NeoPixel.h>
#include "hardware/pio.h"          // to control PIO
#include "corneringlights.pio.h"   // RP2040 PIO assembled program to control the relays

// use of pico's onboard neopixel for debugging & fault finding
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

   // putting our on-board neopixel to diff. colours for easier debugging
int yellow = 0xbca600;  // cornering lights OFF
int  green = 0x00aa00;  // left cornering light ON
int    red = 0xaa0000;  // right cornering light ON
int   blue = 0x0000aa;  // headlights ON, speed > 18 kmh
int   grey = 0x333333;  // speed < 18, st. wheel within (-/+45 deg.)
int purple = 0xaa00aa;  // corn. lights ON (both - vehicle going backward)
int currentPixColor = 0x000000;

//#define  leftRelay 1  // pin for the left corn. light relay
//#define rightRelay 25  // pin for the right corn. light relay

PIO pio = pio0;   // pio to use, we use only one and two of its state machines
uint offset1;     // our pio program location in the memory
uint smLeft;      // the state machine to use for the left relay
uint smRight;     // the state machine to use for the right relay

unsigned long corneringTime = 0;    // used with millis() to sw. corn. lights
unsigned long switchingDelay = 100; // default delay to switch ON/OFF the corn. lights

void SetupCorneringLights()
{
  offset1 = pio_add_program(pio, &corneringlights_program);
  smLeft = pio_claim_unused_sm(pio, true);
  corneringlights_program_init(pio, smLeft, offset1, 1);  // init our asm program to run on pin 1 (left)
  smRight = pio_claim_unused_sm(pio, true);
  corneringlights_program_init(pio, smRight, offset1, 25);  // init our asm program to run on pin 3 (right)

//  pinMode(leftRelay, OUTPUT);     // pin 1
//  pinMode(rightRelay, OUTPUT);    // pin 25

// set initial values at pico start (ignition ON)
      pio_sm_put_blocking(pio, smLeft, 0);  // switch on the left corn. light relay
//      digitalWrite(leftRelay, LOW);       // switch on the left corn. light relay
      pio_sm_put_blocking(pio, smRight, 0);  // switch on the right corn. light relay
//      digitalWrite(rightRelay, LOW);      // switch on the right corn. light relay
      currentPixColor = yellow;             // headlights ON, speed low & positive, but st. wheel within middle range
}

//////////////////////////////////
// vars and function to check if the st. wheel is beyond the offset value
uint16_t stCenter = 4096;              // the steering wheel center position
uint16_t stOffset = OFFSET_DEGREES*2;  // the angle value from center, at which the corn. lights are to be sw. ON/OFF
char     corneringLight = 'N';         // which corn. light to switch on, N - none, L - left, R - right

void CheckStAngle()
{
  if( stAngleVal <= (stCenter - stOffset) )
  {
    corneringLight = 'L';      // for the left cornering light
  }
  else if( stAngleVal >= (stCenter + stOffset) )
  {
    corneringLight = 'R';      // for the right corn. light
  }
  else
  {
    corneringLight = 'N';      // none of the corn. light should be on
  }
}

///////////////////////////////////////////////////
// the function to switch on/off the corn. lights
// based on the CAN bus data
void SwitchCorneringLights()
{
  if( gearLeverPos == 193 || speedDirection > 4 )  // if we are going backward - keep both ON ?
  {
    pio_sm_put_blocking(pio, smLeft, 1);  // switch on the left corn. light relay
//    digitalWrite(leftRelay, HIGH);     // switch on the left corn. light relay
    pio_sm_put_blocking(pio, smRight, 1);  // switch on the right corn. light relay
//    digitalWrite(rightRelay, HIGH);    // switch on the right corn. light relay
    currentPixColor = purple;          // for debugging - purple means both should be ON
    switchingDelay = 4000;    // set switch off delay after going back
  }
  else if( corneringLight == 'L' )
  {
    if( turnSignal == 'R' )
    {
      pio_sm_put_blocking(pio, smRight, 1);  // switch on the right corn. light relay
      pio_sm_put_blocking(pio, smLeft, 0);  // switch on the left corn. light relay
//      digitalWrite(rightRelay, HIGH);   // switch on the right corn. light relay
//      digitalWrite(leftRelay, LOW);   // switch on the left corn. light relay
      currentPixColor = red;              // right corn. light must now be on
//      switchingDelay = 3000;    // set switch off delay after turn signal
    }
    else
    {
      pio_sm_put_blocking(pio, smLeft, 1);  // switch on the left corn. light relay
      pio_sm_put_blocking(pio, smRight, 0);  // switch on the right corn. light relay
//      digitalWrite(leftRelay, HIGH);   // switch on the left corn. light relay
//      digitalWrite(rightRelay, LOW);   // switch on the left corn. light relay
      currentPixColor = green; // right corn. light must now be on
      switchingDelay = 4000;    // set switch off delay after going forward
    }
  }
  else if( corneringLight == 'R' )
  {
    if( turnSignal == 'L' )   
    {
      pio_sm_put_blocking(pio, smLeft, 1);  // switch on the left corn. light relay
      pio_sm_put_blocking(pio, smRight, 0);  // switch on the left corn. light relay
//      digitalWrite(leftRelay, HIGH);   // switch on the left corn. light relay
//      digitalWrite(rightRelay, LOW);   // switch on the left corn. light relay
      currentPixColor = green;                  // right corn. light must now be on
//      switchingDelay = 3000;    // set switch off delay after turn indicator
    }
    else
    {
      pio_sm_put_blocking(pio, smRight, 1);  // switch on the right corn. light relay
      pio_sm_put_blocking(pio, smLeft, 0);  // switch on the left corn. light relay
//      digitalWrite(rightRelay, HIGH);   // switch on the right corn. light relay
//      digitalWrite(leftRelay, LOW);   // switch on the left corn. light relay
      currentPixColor = red;                  // right corn. light must now be on
      switchingDelay = 4000;    // set switch off delay after going forward
    }
  }
  else
  {
    pio_sm_put_blocking(pio, smLeft, 0);  // switch on the left corn. light relay
    pio_sm_put_blocking(pio, smRight, 0);  // switch on the right corn. light relay
//    digitalWrite(leftRelay, LOW);   // switch on the left corn. light relay
//    digitalWrite(rightRelay, LOW);  // switch on the right corn. light relay
    currentPixColor = yellow;          // headlights ON, speed low & positive, but st. wheel within middle range
    switchingDelay = 200;            // set switch off delay after going back
  }
}
////////////////////////////////////
// the function to schedule switching on/off the cornering lights
void RunCorneringLights()
{
  // 29.04.24 - moved the nightTime condition from the loop1() to here
  // 02.12.24 - added headlights == 'L' cond. that will allow me to switch off corn. lights
  //           together with the headlights if I don't want corn. lights
  if( nightTime && currentSpeed < MAX_SPEED && headlights == 'L' ) 
  {
    CheckStAngle();

    if( turnSignal != 'N' )
    {
      switchingDelay = 100;        // quick switch over to opposite side if any indicator is on
    }
    if( millis() - corneringTime >= switchingDelay ) 
    {
      SwitchCorneringLights(); 
      corneringTime = millis();
    }
  }
  else
  {
    pio_sm_put_blocking(pio, smLeft, 0);  // switch on the left corn. light relay
    pio_sm_put_blocking(pio, smRight, 0);  // switch on the right corn. light relay
//    digitalWrite(leftRelay, LOW);   // switch on the left corn. light relay
//    digitalWrite(rightRelay, LOW);  // switch on the right corn. light relay
    currentPixColor = yellow;          // headlights/nightTime ON, speed low & positive, but st. wheel within middle range
    switchingDelay = 200;              // set switch off delay to default
  }
}
