This is the repo for fitting Adafurit RP2040 CAN bus Feather to Mercedes-Benz vehicles as additional controller and plugging it into the vehicle's CAN bus to interract with other control units.

Currently implemented the following functions:
  1. Fitting custom cornering lights to Mercedes-Benz vehicle, model S212 by aquiring CAN bus messges. The code was created in Arduino IDE. The CAN bus IDs used in this project so far:
     1. Steering Angle Sensor;
     2. Light Sensor;
     3. Turn Indicator switch msgs;
     4. Vehicle Speed Sensors;
     5. Vehicle's Headlight Sensor.

