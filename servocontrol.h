/*
 * functions to control the servo to move GoPro left/right
 * based on the steering wheel angle obtained in the ReadCan(), in the canfunctions.h
 * 
 * the code in this file is setup and run on the 2nd core
 */
#include <Servo.h>

int servoPos = 1520;
int servoCenter = servoPos;
Servo GPServo;  // create servo object to control a servo

// this func. is called from our main setup()
//   it assgnes the digital pin 5 of our Adafruit RP2040 CAN to control the servo
void SetupServo()
{
 // set init (center) servo position to 130 deg
 //  if we increase this value, we get more left to right range
 //  if we decrease this, we get nearower range
 //  value of 90 deg. gives us range of -/+ 20 deg.
 //  value of 130 deg. gives us range of -/+ 32 deg. (see below)

  GPServo.writeMicroseconds(servoCenter);  // write before attch to avoid startup twiching
  GPServo.attach(5);   // hooks the pin GIO5 to the servo control
}

// this functions moves my GoPro camera, that is 
//   on the top of the servo, left or right,
//   depending on the steering wheel angle;
// the function is called from the main loop
//   and takes the 2 Byte value from the ReadCan() that is in canfunctions.h
void MoveGopro()
{
     // GoPro records wide, so to follow the road we need
     // to move it no more than -/+ 30 deg. (nach links / nach rechts)
     // servo center is at 1520 microseconds
     // to get st. wheel to servo ratio we devide st. wheel center by servo center, i.e.:
     //    4090 / 1520 = 2.7 
     // stAngleVal lowest value of 3100 will set servo's pos. to 1148 microseconds (wheels fully to the left)
     // stAngleVal highest value of 5100 is to set servo's pos. at 1888 microseconds (wheels fully to the right)
     // to increase servo range we need to lower the ratio,
     // if we need ratio in degrees, than: stAngleVal to servo ratio will be 31.46153846
     // or we can use map func instead of calculating the ratio, like:
     //    servoPos = map(stAngleVal, 3100, 5100, 1148, 1888);  

  // because my servo is backward, and I cannot swap the motor wires, I need to reverse its movement
  //  and so we can do ratio and the reverse with the help of the map(), if we swap 2nd 3rd params as: 
  servoPos = map(stAngleVal, 5100, 3100, 1140, 1890);
  GPServo.writeMicroseconds(servoPos); // move the servo to the position dependent upon the position of the front wheels
}
