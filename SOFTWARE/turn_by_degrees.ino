#include <math.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_MotorShield.h>

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.

   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3-5V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
   2015/AUG/27  - Added calibration and system status helpers
*/

//IMU
/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)
Adafruit_BNO055 bno = Adafruit_BNO055(55);

//Motors
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *leftMotor = AFMS.getMotor(1);
Adafruit_DCMotor *rightMotor = AFMS.getMotor(3);

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
//void displaySensorDetails(void)
//{
//  sensor_t sensor;
//  bno.getSensor(&sensor);
//  Serial.println("------------------------------------");
//  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
//  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
//  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
//  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
//  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
//  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
//  Serial.println("------------------------------------");
//  Serial.println("");
//  delay(500);
//}

/**************************************************************************/
/*
    Display some basic info about the sensor status
*/
/**************************************************************************/
//void displaySensorStatus(void)
//{
//  /* Get the system status values (mostly for debugging purposes) */
//  uint8_t system_status, self_test_results, system_error;
//  system_status = self_test_results = system_error = 0;
//  bno.getSystemStatus(&system_status, &self_test_results, &system_error);
//
//  /* Display the results in the Serial Monitor */
//  Serial.println("");
//  Serial.print("System Status: 0x");
//  Serial.println(system_status, HEX);
//  Serial.print("Self Test:     0x");
//  Serial.println(self_test_results, HEX);
//  Serial.print("System Error:  0x");
//  Serial.println(system_error, HEX);
//  Serial.println("");
//  delay(500);
//}

/**************************************************************************/
/*
    Display sensor calibration status
*/
/**************************************************************************/
//void displayCalStatus(void)
//{
//  /* Get the four calibration values (0..3) */
//  /* Any sensor data reporting 0 should be ignored, */
//  /* 3 means 'fully calibrated" */
//  uint8_t system, gyro, accel, mag;
//  system = gyro = accel = mag = 0;
//  bno.getCalibration(&system, &gyro, &accel, &mag);
//
//  /* The data should be ignored until the system calibration is > 0 */
//  Serial.print("\t");
//  if (!system)
//  {
//    Serial.print("! ");
//  }
//
//  /* Display the individual values */
//  Serial.print("Sys:");
//  Serial.print(system, DEC);
//  Serial.print(" G:");
//  Serial.print(gyro, DEC);
//  Serial.print(" A:");
//  Serial.print(accel, DEC);
//  Serial.print(" M:");
//  Serial.print(mag, DEC);
//}

double targetXOrientation;
void setRelativeTargetRotation(double degreeDelta, double current)
{
  targetXOrientation = fmod(current + degreeDelta, 360.0);
}

double getTargetRotationDelta(double currentRotation)
{
  double inDelta = targetXOrientation - currentRotation;
  double outDelta = inDelta;
  double absDelta = abs(inDelta);

  if (absDelta > 180)
  {
    outDelta = 360 - absDelta;
    if (inDelta > 0)
    {
      outDelta = -outDelta;
    }
  }

  return outDelta;
}

void updateTargetRotationLoop(double currentRotation, double &leftPowerOut, double &rightPowerOut)
{
  const double minSpeed = 100;
  const double maxSpeed = 255;
  const double minSpeedDegrees = 1; //delta required to warrant moving at all
  const double maxSpeedDegrees = 30; //delta required to warrant going full speed
  
  double delta = getTargetRotationDelta(currentRotation);
  double deltaIntensity = constrain(abs(delta)/maxSpeedDegrees, 0.0, 1.0);

  double power = 0;
  if (abs(delta) >= minSpeedDegrees)
  {
    power = lerp(minSpeed, maxSpeed, deltaIntensity) * sign(delta);
  }
  else
  {
    power = 0;
  }

  //Assumes positive means forward
  leftPowerOut = power;
  rightPowerOut = -power;
}

//linearly interpolates from a to b based on percent (0-1)
double lerp(double a, double b, double percent)
{
  return a + (b-a)*percent;
}

int sign(double input)
{
  if (input < 0)
  {
    return -1;
  }
  else if (input > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


//void turnInX(double deltaX) {
//  Serial.print(" current X: ");
//  Serial.println(deltaX, 4);
//}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(9600);
  Serial.println("Orientation Sensor Test"); Serial.println("");

  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

//  delay(1000);
//
//  /* Display some basic information on this sensor */
//  displaySensorDetails();
//
//  /* Optional: Display current status */
//  displaySensorStatus();
//
//  bno.setExtCrystalUse(true);

  //Motors
  AFMS.begin();
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/

unsigned long beginStraightMillis = 0;

void loop(void)
{
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);
  

  /* Optional: Display calibration status */
  //displayCalStatus();

  /* Optional: Display sensor status (debug only) */
  //displaySensorStatus();
  const unsigned long straightDuration = 3000;
  if (millis() - beginStraightMillis < straightDuration) //go straight
  {
    leftMotor->setSpeed(255);
    rightMotor->setSpeed(255);
  }
  else //turny stuff
  {
    double currentRotation = event.orientation.x;
    double deltaFromTarget = getTargetRotationDelta(currentRotation);
    
    if (abs(deltaFromTarget) < 1)
    {
      setRelativeTargetRotation(90, currentRotation);
      beginStraightMillis = millis();
      Serial.println("Set new target rotation");
    }
    else
    {
      double leftPower;
      double rightPower;
      updateTargetRotationLoop(currentRotation, leftPower, rightPower);
    
      leftMotor->setSpeed(leftPower);
      leftMotor->run(FORWARD);
      rightMotor->setSpeed(rightPower);
      rightMotor->run(FORWARD);

      Serial.print("currentRotation="); Serial.print(currentRotation);
      Serial.print(" | deltaFromTarget="); Serial.print(deltaFromTarget);
      Serial.print(" | leftPower="); Serial.print(leftPower);
      Serial.print(" | rightPower="); Serial.print(rightPower);
      Serial.println("");
    }
  }

  
  
  /* Wait the specified delay before requesting nex data */
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
