#include <Servo.h>//Using servo library to control esc1
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Kalman.h>


float accPitch = 0;
float accRoll = 0;

float kalPitch = 0;
float kalRoll = 0;


RF24 radio(7, 8); // create a radio class 

Servo esc1; // Creating a servo class with name as esc1
Servo esc2;
Servo esc3;
Servo esc4;

const byte address[6] = "00001";

const int MPU = 0x68; // accelerometer address 

// motors config
// motors power pins
int motorPin1 = 5;  // pin to connect to motor module
int motorPin2 = 6;  // pin to connect to motor module
int motorPin3 = 9;  // pin to connect to motor module
int motorPin4 = 10;  // pin to connect to motor module

// save last speed value
int lastMotorOneSpeed = 0;
int lastMotorTwoSpeed = 0;
int lastMotorThreeSpeed = 0;
int lastMotorFourSpeed = 0;

bool motorsRunning = false;
bool motorsAtMinimalSpeed = false;

double AccX, AccY, AccZ, Temp, GyX, GyY, GyZ;
uint32_t timer;

// here we keep tracking the last time we recived instructions
// from our controllers. 
unsigned long previousMillis = 0; // last time update
long interval = 300; // interval at which we call stabilization if not reciving any instructions

Kalman kalmanX;
Kalman kalmanY;
Kalman kalmanZ;

double gyroXangle;
double gyroYangle;

double kalAngleX;
double kalAngleY;
double kalAngleZ;

void setup() {
  Serial.begin(9600);// initialize serial motor 

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  // store data
  AccX = Wire.read() << 8 | Wire.read();
  AccY = Wire.read() << 8 | Wire.read();
  AccZ = Wire.read() << 8 | Wire.read();
  Temp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  double pitch = atan(AccX/sqrt(AccY*AccY + AccZ*AccZ)) * RAD_TO_DEG;
  double roll = atan(AccY/sqrt(AccX*AccX + AccZ*AccZ)) * RAD_TO_DEG; 

  kalmanX.setAngle(roll);
  kalmanY.setAngle(pitch);

  gyroXangle = roll;
  gyroYangle = pitch;

  timer = micros();

  pinMode(motorPin1, OUTPUT);// set mtorPin as output
  pinMode(motorPin2, OUTPUT);// set mtorPin as output
  pinMode(motorPin3, OUTPUT);// set mtorPin as output
  pinMode(motorPin4, OUTPUT);// set mtorPin as output

  esc1.attach(motorPin1); //Specify the esc1 signal pin
  esc2.attach(motorPin2); //Specify the esc1 signal pin
  esc3.attach(motorPin3); //Specify the esc1 signal pin
  esc4.attach(motorPin4); //Specify the esc1 signal pin

  // set all motors speed to zero
  esc1.write(0);
  esc2.write(0);
  esc3.write(0);
  esc4.write(0);

  delay(1000);

  setMinimalStartSpeed(); // set all speeds to minimal value 40

  // radio initializer
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  // config accelerometer

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
  //Serial.println(lastMotorOneSpeed);
  //Serial.println();

  
 
  accelerometerMeassure(1);
  if(radio.available()) {
    char controllerInstruction[32] = "";
    char directionInstruction[32] = "";
    radio.read(&controllerInstruction, sizeof(controllerInstruction));
    radio.read(&directionInstruction, sizeof(directionInstruction));
    Serial.println(controllerInstruction);
    Serial.println(directionInstruction);    
    
    if(strcmp(controllerInstruction, "EMERGENCY BREAK") == 0) {
        exit(1);
    }
    
    if(strcmp(controllerInstruction, "GENERAL_SPEED") == 0) {
      if(strcmp(directionInstruction, "UP") == 0) {
        previousMillis = millis();
        setGeneralSpeed(1,1,1,1, "UP");
        delay(200);
      } else if(strcmp(directionInstruction, "DOWN") == 0) {
        previousMillis = millis();
        setGeneralSpeed(1,1,1,1, "DOWN");
        delay(200);
      }

    } else if(strcmp(controllerInstruction, "DIRECTION") == 0) {
      if(strcmp(directionInstruction, "UP") == 0){
        motorsRunning = true;
        setMotorsSpeed(1,1,1,1); // increses motors speed
      }
    }
  }
}


void setGeneralSpeed(int motorOne, int motorTwo, int motorThree, int motorFour, String direction) {
    if(direction == "UP") {
      esc1.write(lastMotorOneSpeed + motorOne);
      esc2.write(lastMotorTwoSpeed + motorTwo);
      esc3.write(lastMotorThreeSpeed + motorThree);
      esc4.write(lastMotorFourSpeed + motorFour);

      lastMotorOneSpeed = lastMotorOneSpeed + motorOne;
      lastMotorTwoSpeed = lastMotorTwoSpeed + motorTwo;
      lastMotorThreeSpeed = lastMotorThreeSpeed + motorThree;
      lastMotorFourSpeed = lastMotorFourSpeed + motorFour;

    } else if(direction == "DOWN" &&
              lastMotorOneSpeed > 0 &&
              lastMotorTwoSpeed > 0 &&
              lastMotorThreeSpeed > 0 &&
              lastMotorFourSpeed > 0
    ) {
      esc1.write(lastMotorOneSpeed - motorOne);
      esc2.write(lastMotorTwoSpeed - motorTwo);
      esc3.write(lastMotorThreeSpeed - motorThree);
      esc4.write(lastMotorFourSpeed - motorFour);

      lastMotorOneSpeed = lastMotorOneSpeed - motorOne;
      lastMotorTwoSpeed = lastMotorTwoSpeed - motorTwo;
      lastMotorThreeSpeed = lastMotorThreeSpeed - motorThree;
      lastMotorFourSpeed = lastMotorFourSpeed - motorFour;
    }
}

void setMotorsSpeed(float motorOne, int motorTwo, int motorThree, int motorFour) {
    // CREATE METHOD FOR SLOWLY INCREASES SPEED FOR AVOID BUMPS
    esc1.write(motorOne); //using val as the signal to esc1
    esc2.write(motorOne);
    esc3.write(motorOne);
    esc4.write(motorOne);
}

void setMinimalStartSpeed() {
  esc1.write(40);
  esc2.write(40);
  esc3.write(40);
  esc4.write(40);

  lastMotorOneSpeed = 40;
  lastMotorTwoSpeed = 40;
  lastMotorThreeSpeed = 40;
  lastMotorFourSpeed = 40;
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
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  double dt = (double)(micros() - timer) / 1000000;
  timer = micros();

  
  double pitch = atan(AccX/sqrt(AccY*AccY + AccZ*AccZ)) * RAD_TO_DEG;
  double roll = atan(AccY/sqrt(AccX*AccX + AccZ*AccZ)) * RAD_TO_DEG; 

  gyroXangle = GyX / 131.0;
  gyroYangle = GyY / 131.0;

  kalAngleX = kalmanX.getAngle(roll, gyroXangle, dt);
  kalAngleY = kalmanY.getAngle(pitch, gyroYangle, dt);

  //Serial.print(pitch);
  //Serial.print(" ");
  //Serial.print(roll);
  Serial.print(" ");
  Serial.println(kalAngleX);
  //Serial.print(" ");
  //Serial.println(kalAngleY);


  // reduce 9 and 10 motors speed
  setAngleCorrection(kalAngleX);
  
  
  if(axis == 0) {
    return pitch;
  } else if(axis == 1) {
    return roll;
  }
  
}

void setAngleCorrection(float kalAngleX) {
  if(kalAngleX >= -2 && kalAngleX < 0) {
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;  
        Serial.println("foiiiii");
        stabilize();

      // do something
    }
  }
  if (
      kalAngleX > 5 &&
      lastMotorOneSpeed <= 169 &&
      lastMotorTwoSpeed <= 169 &&
      lastMotorThreeSpeed <= 169 &&
      lastMotorFourSpeed <= 169
     ) {
      esc1.write(lastMotorOneSpeed + 1);
      esc2.write(lastMotorTwoSpeed + 1);
      esc3.write(lastMotorThreeSpeed + 1);
      esc4.write(lastMotorFourSpeed + 1);

      lastMotorOneSpeed++;
      lastMotorTwoSpeed++;
      lastMotorThreeSpeed++;
      lastMotorFourSpeed++;
      
      motorsAtMinimalSpeed = false;
      
      delay(50);
  }
}

void stabilize() {
      // set all motors to minimal flight speed
      //while(
        //lastMotorOneSpeed > 40 ||
        //lastMotorTwoSpeed > 40 ||
        //lastMotorThreeSpeed > 40 ||
        //lastMotorFourSpeed > 40 
      //) {
        
     //   if(lastMotorOneSpeed > 40) {
         // lastMotorOneSpeed--;
       // } 
       // if(lastMotorTwoSpeed > 40) {
       //   lastMotorOneSpeed--;
       // }
       // if(lastMotorThreeSpeed > 40) {
       //   lastMotorThreeSpeed--;
       // }
       // if(lastMotorFourSpeed > 40) {
        //  lastMotorFourSpeed--;
       // }
        
        esc1.write(40);
        esc2.write(40);
        esc3.write(40);
        esc4.write(40);

        lastMotorOneSpeed = 40;
        lastMotorTwoSpeed = 40;
        lastMotorThreeSpeed = 40;
        lastMotorFourSpeed = 40;     
}

class Motor : public Servo {

  public:
    void attachMotorPins(int pin) {
      
    }

  private: 
    int motorPinAtArduino;
    int motorSpeed;

};
