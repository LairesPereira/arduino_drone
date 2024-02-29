#include <Servo.h>//Using servo library to control esc1
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Kalman.h>

Servo front_right_prop;
Servo front_left_prop;

/*MPU-6050 gives you 16 bits data so you have to create some 16int constants
 * to store the data for accelerations and gyro*/

int16_t Acc_rawX, Acc_rawY, Acc_rawZ,Gyr_rawX, Gyr_rawY, Gyr_rawZ;
 
float Acceleration_angle[2];
float Gyro_angle[2];
float Total_angle[2];

float elapsedTime, time, timePrev;
float rad_to_deg = 180/3.141592654;

float PID, pwmLeft, pwmRight, error, previous_error;

float pid_p=0;
float pid_i=0;
float pid_d=0;

/////////////////PID CONSTANTS/////////////////
double kp=0.2;//3.55
double ki=0.002;//0.003
double kd=0.4;//2.05

// BOM RESULTADO
//double kp=8.9;//3.55
//double ki=0.00095;//0.003
//double kd=1.2;//2.05
///////////////////////////////////////////////

double throttle=1300; //initial value of throttle to the motors
float desired_angle = 0; //This is the angle in which we whant the
                         //balance to stay steady

RF24 radio(7, 8); // create a radio class 

Servo esc1; // Creating a servo class with name as esc1
Servo esc2;
Servo esc3;
Servo esc4;

const byte address[6] = "00001";

const int MPU = 0x68; // accelerometer address 

bool motorsRunning = false;
bool motorsAtMinimalSpeed = false;
bool firstStart = true;

double AccX, AccY, AccZ, Temp, GyX, GyY, GyZ;
uint32_t timer;

Kalman kalmanX;
Kalman kalmanY;
Kalman kalmanZ;

double gyroXangle;
double gyroYangle;
double kalAngleX;
double kalAngleY;
double kalAngleZ;

void setup() {
  Serial.begin(250000);

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  front_left_prop.attach(5);  //attatch the left motor to pin 5
  front_right_prop.attach(6); //attatch the right motor to pin 6

  time = millis(); //Start counting time in milliseconds
  /*In order to start up the ESCs we have to send a min value
   * of PWM to them before connecting the battery. Otherwise
   * the ESCs won't start up or enter in the configure mode.
   * The min value is 1000us and max is 2000us, REMEMBER!*/
  
  front_left_prop.writeMicroseconds(2000);
  front_right_prop.writeMicroseconds(2000);
  Serial.println("\n");
  Serial.println("CONNECT BATTERY");
  delay(200); /*Give some delay, 7s, to have time to connect
                *the propellers and let everything start up*/ 
  front_left_prop.writeMicroseconds(1000);
  front_right_prop.writeMicroseconds(1000);
 
  // radio initializer
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  // initialize sensor
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU);
  Wire.write(0x1B);
  Wire.write(0x00011000);
  Wire.endTransmission();
}

void loop() {
  radioInstructions();
  pid();
  printValues();
}


