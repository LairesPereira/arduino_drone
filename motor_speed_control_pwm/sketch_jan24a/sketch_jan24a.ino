#include <Wire.h>

const int MPU = 0x68; // accelerometer address 
int motorPin1 = 10;// pin to connect to motor module
int motorPin2 = 9;// pin to connect to motor module
int mSpeed = 0;// variable to hold speed value
int mStep = 15;// increment/decrement step for PWM motor speed
int motorOneSpeed = 0;
int motorTwoSpeed = 0;
int motorThreeSpeed = 0;
int motorFourSpeed = 0;

float AccX, AccY, AccZ, Temp;
bool firstStart = true;


void setup() {
  // Robojax.com demo
  pinMode(motorPin1, OUTPUT);// set mtorPin as output
  pinMode(motorPin2, OUTPUT);// set mtorPin as output
  Serial.begin(9600);// initialize serial motor  


  // config accelerometer
  Serial.begin(9600);

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
  if(firstStart) {
    startMotors(10, 10, 10, 10);
    firstStart = false;
  } 

  float levelX = accelerometerMeassure(0);
  float levelY = accelerometerMeassure(1);
  
  if(levelY > 0.5) {
    setMotorsSpeed(255, motorTwoSpeed, motorThreeSpeed, motorFourSpeed);
  }

  Serial.print(levelX);
  Serial.print(" ");
  Serial.println(levelY);
  delay(200);
}

void setMotorsSpeed(int motorOne, int motorTwo, int motorThree, int motorFour) {
    analogWrite(motorPin1, motorOne); // send mSpeed value to motor
    analogWrite(motorPin2, motorTwo);
    
    motorOneSpeed = motorOne;
    motorTwoSpeed = motorTwo;
}

void startMotors(int motorOne, int motorTwo, int motorThree, int motorFour) {
    // Serial.println(mSpeed); // print mSpeed value on Serial monitor (click on Tools->Serial Monitor)
    // starts always at low speed
    analogWrite(motorPin1, motorOne); // send mSpeed value to motor
    analogWrite(motorPin2, motorTwo); 
}

float accelerometerMeassure(int axis) {
  // axis X = 0 
  // axis Y = 1

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  // store data
  AccX = Wire.read() << 8 | Wire.read();
  AccY = Wire.read() << 8 | Wire.read();
  AccZ = Wire.read() << 8 | Wire.read();
  Temp = Wire.read() << 8 | Wire.read();


  // visualize accerleration

  //Serial.print(AccX / 2048);
  //Serial.print(" ");
  //Serial.print(AccY / 2048);
  //Serial.print(" ");
  //Serial.print(AccZ / 2048);
  //Serial.print(" ");

  // convert data to angle
  double pitch = atan(AccX/AccZ);
  double roll = atan(AccY/AccZ);
  //Serial.print(pitch);
  //Serial.print(" ");
  //Serial.println(roll);
  if(axis == 0) {
    return pitch;
  } else if(axis == 1) {
    return roll;
  }
  
}
