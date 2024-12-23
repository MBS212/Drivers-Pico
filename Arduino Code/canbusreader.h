/* contains functions:
SetupCan()
ReadCan() to read the can bus
          and to process the data 
*/
#include <Adafruit_MCP2515.h>

#define CS_PIN    PIN_CAN_CS
Adafruit_MCP2515 mcp(CS_PIN);  // the CAN bus processing
  
// Set CAN bus baud rate depending on which CAN bus we are connected to
#define CAN_B   125000  // bus B speed: 125 kbps
#define CAN_E   500000  // bus E speed: 500 kbps

// variables related to reading the vehicle's light sensor
uint8_t lightSensorId = 37;       // light sensor messages id (00025 in hex)
uint8_t lightSensorMaxVal = 199;  // the darker outside the higher the value - needs fine tuning
bool nightTime = false;           // remember if it's light or dark

// variable to process lights messages
int16_t headlightsId = 105;  // vehicle lights msg. id (00069 in hex),
uint8_t headlightsVal = 12;  // 0x0C in hex, the value that we need has to be above 4
                             //   only first Byte changing with the switch, as follows:
                             //   no lights - 0, side lights - 4 (0x04), auto - 76(0x4C), low beam - 12 (0x0C)
char headlights = 'N';       // remember the state of the headlights [N-none, S-side, L-low beam]

// variables to process speed messages
uint16_t speedId = 515;     // vehicle speed msg id (00203 in hex)
uint16_t currentSpeed = 0;  // remember the last CAN speed value
uint8_t speedDirection = 4; // remember the direction of travel, 4 - forward, 8 - backward

// variables to process the turn signal messages
int16_t turnSignalId = 41;    // turn indicator L & R msg id (00029 in hex big-endian),
                                 //   only first 2 B are changing, as follows:
                                 //   no indicators: 00 00, left on: 50 E1, right on: 90 E1,
                                 //   so we have only 1st Byte changing: 0x50 for left, and 0x90 for right
uint8_t turnSignalLeft = 80;     // 50 in hex
uint8_t turnSignalRight = 144;   // 90 in hex
char turnSignal = 'N';           // none of the turn signals is blinking

// variables to process the steering angle messages
int16_t  stAngleId = 3;    // steering angle msg id (00003 in hex)
uint16_t stAngleVal = 4096;  // this will hold the first 2 Bytes of data from the steering angle sensor's msg
                                 //   and it will also be used to control the GoPro servo movement in (see servocontrol.h)



////////////////////////////
// declaration of the function in this file
void ReadLightSensor(uint8_t byte1);            // find out if it's day or night time
void ReadHeadlights(uint8_t byte1);             // read & save current state of the headlights
void ReadSpeed(uint8_t byte1, uint8_t byte2);   // read & save current speed
void ReadTurnSignal(uint8_t byte1);             // find out if the turn indic. is on and which one
//////////////////////////////////////////////////////////////////////////////
                                
// this func. is called from the main setup() function in the 001-steering (our main) file
void SetupCan() 
{
//Serial.begin(115200);
  if( ! mcp.begin(CAN_B) )  { while(1); } 
}


// the main and only function in this file - to read and process CAN msgs
//   and in the second part to switch our cornering lights on/off
void ReadCAN()
{
  uint16_t canMsgId = 0; // current message ID
  uint8_t a = 0;  // 1st byte from CAN msg data
  uint8_t aa = 0; // 2nd byte from CAN msg data

  int packetSize = mcp.parsePacket();

  if( packetSize )  // if CAN frame is detected, otherwise skip to the end of the func and repeat again
  {
    canMsgId = (uint16_t) mcp.packetId();  // read the CAN msg ID

    if( mcp.available() )      // if the msg data Bytes are avail., or skip to the end and cycle again
    {                          //     in any of the 5 frames that we need, only the 1st 2B of the msg data is usefull to us, so:
      a = mcp.read();          // read 1st byte from the msg data field
      aa = mcp.read();         // read 2nd byte from the msg data field
    }
    else
    {
      return; 
    }
       // check for the steering angle messages 
    if( canMsgId == stAngleId )   // 3 in decimal and in hex
    {
      stAngleVal = (a<<8) + (aa<<0);   // move a into the 1st Byte, aa into the 2nd, of the stAngleVal
        //Serial.print("st angle:"); Serial.print(stAngleVal);  // for debuging
    }
       //  look for light sensor msg. and receord its value
    else if( canMsgId == lightSensorId )  // 37 decimal
    {
      ReadLightSensor(a);
    }
//  check for the headlights messages and call the function to get the values
//  01.12.24 - enabled reading of the headlights to be able to switch off
//             cornering lights together with headlights if needed
//             also added the condition in the RunCorneringLights() in corneringlights.h
    else if( canMsgId == headlightsId )  // 105 decimal
    {
      ReadHeadlights(a);
    }
    else
    {
      // just ignore
    }
    // read & process CAN bus msgs further only if it's night time,
    if( nightTime )  // headlightsOn && nightTime )
    { 
      if( canMsgId == speedId )    // check if new speed msg (with id 515) was sent
      {
        ReadSpeed(a, aa);
      }
      if( canMsgId == turnSignalId )     // check if turn indicator was triggered
      {
        ReadTurnSignal(a);
      }
    }  // end of if headlights ON condition 
  }  // end of frame detection
}
/////////////////////////////////////
// functions defs. used above in this file

///////////////////
//  read the state of our light sensor (NOT the sun sernsor)
void ReadLightSensor(uint8_t byte1)
{
  if( byte1 < lightSensorMaxVal )   // if it's dark enough
  {
    nightTime = false;
  } 
  else 
  {
    nightTime = true;
  }
//Serial.print("light sensor:"); Serial.print(byte1); Serial.print(":"); Serial.println(nightTime);
}
// to read and remember the curr. state of the headlight unit
void ReadHeadlights(uint8_t byte1)
{
  if( byte1 < 4 )
  {
    headlights = 'N';    // all the lights are OFF (N - none)
  } 
  else if( byte1 < 12 )
  {
    headlights = 'S';    // the lights are set to SIDE LIGHTS
  } 
  else if( byte1 > 11 && byte1 <= 76 ) 
  {
    headlights = 'L';    // the lights are set to LOW BEAM
  } 
  else 
  {
    headlights = 'N';
  }
//Serial.print("headlights:"); Serial.print(byte1); Serial.println("-"); Serial.println(headlights);
}
// now we get the vehicle speed so that we can switch the corn. lights on at less than set speed
//  the speed data in the msg is in mph, taken for each wheel, and, as my tests have shown, 
//    it starts with bit 5 (FL wheel), and is 12 bit long,
//    and for all wheels it starts with: FL-bit5,FR-bit21,RL-bit37,RR-bit53, 
//  I think data on the dashboard is the average for all wheels???
//  Also, we don't need high pressision in this project, so we take only speed from a single wheel (FL).
//  The conversion ratio is to mutliply the value by 0.0375.
//  So we are reading the first two Bytes and putting their bits into our currentSpeed variable
//  It looks like the 1st 4 bits of the 1st Byte indicate direction: 0x4 forward, and 0x8 backward. 
void ReadSpeed(uint8_t byte1, uint8_t byte2)
{
//  speedDirection = byte1;               // direction is on leftmost 4 bits, so assign 1st Byte only to speedDirection
//  speedDirection &= 0b11110000;         // mask 4 rightmost bits with zeros, i.e.: 0b11110000
  speedDirection = byte1 >> 4;  // shift 4 leftmost bits to the rightmost position, and we get like: 0b00001111
                                        // so if first 4 bits of the 1st Byte are 0x8, speedDirection will have value of 0x08

  // for current speed we need 12 bits, so we set 1st 4 bits as zeros, 
  //   then 4 bits from byte1 into the 1st Byte, then first 4 bits from byte2 into the 2nd Byte
  uint8_t tmp = byte1 << 4;              // shift last 4 bits from byte1 into temp. byte
  byte1 = tmp >> 4;                      // shift first 4 bits from temp. byte into byte1 so that first 4 bits of byte1 will be zeros
  currentSpeed = (byte1<<8) | (byte2<<0);  // move both Bytes into currentSpeed
  currentSpeed = (currentSpeed * 0.0375) * 1.609;  // calculate current speed in kmh, dashboard always shows 3 kmh more than CAN bus
//Serial.print("speed:"); Serial.print(speedDirection); Serial.println("-"); Serial.println(currentSpeed);
}
///////////////////////////////////////////////////////
// the function to read the state of the turn signals
void ReadTurnSignal(uint8_t byte1)
{
  if( byte1 == turnSignalLeft )   turnSignal = 'L';       // update turn signal value
  else if( byte1 == turnSignalRight )  turnSignal = 'R';  // update turn signal value
  else turnSignal = 'N';                                  // turn singnal is set to None
//Serial.print("turn signal:"); Serial.print(byte1); Serial.println("-"); Serial.println(turnSignal);
}
// end of function definitions
/////////////////////////////////////
/*
notes on the steering angle

msg 0x00003 (hex) & 3 (dec)
the first two bytes:
0C 26 (hex) = 3110 (dec) = -491 deg. (full left)
10 00 (hex) = 4090 (dec) =    0 deg.  (center)
13 DE (hex) = 5086 (dec) = +496 deg  (full right)

full range = 1976 or 987 degrees
so 1976 / 988 = 2 - ratio: value to degrees,
so 988 / 2 = 494 - st wheel's halft value, so:
0 - st wheel in the center;
0 - 494 = -494 - st wheel fully to the left;
0 + 494 = +494 - st wheel fully to the right;

the car's front wheels angle range goes from -29 to 29 degs
or 58 deg. full range from left to right,
so 1976 / 58 = 34 - ratio: st angle sensor to wheel degrees,
like 4090 / 34 = 120.3 - the vale to subtract to get degrees from the center, i.e.:
3110 / 34 - 120 = -28.5 degrees fully to the left;
5086 / 34 - 120 = +29.5 degrees fully to the right;

*/
