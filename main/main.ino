#include <Servo.h>//Using servo library to control esc1
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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

float AccX, AccY, AccZ, Temp;

void setup() {
  Serial.begin(9600);// initialize serial motor  

  pinMode(motorPin1, OUTPUT);// set mtorPin as output
  pinMode(motorPin2, OUTPUT);// set mtorPin as output
  pinMode(motorPin3, OUTPUT);// set mtorPin as output
  pinMode(motorPin4, OUTPUT);// set mtorPin as output

  esc1.attach(motorPin1); //Specify the esc1 signal pin
  esc2.attach(motorPin2); //Specify the esc1 signal pin
  esc3.attach(motorPin3); //Specify the esc1 signal pin
  esc4.attach(motorPin4); //Specify the esc1 signal pin

  esc1.write(0); //using val as the signal to esc1
  delay(1000);

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
  Serial.println(lastMotorOneSpeed);
  esc1.write(lastMotorOneSpeed); //using val as the signal to esc1
  

  if(radio.available()) {
    char controllerInstruction[32] = "";
    char directionInstruction[32] = "";
    radio.read(&controllerInstruction, sizeof(controllerInstruction));
    radio.read(&directionInstruction, sizeof(directionInstruction));
    Serial.println(controllerInstruction);
    Serial.println(directionInstruction);    
    
    if(strcmp(controllerInstruction, "switch_motors_power") == 0) {
      //startStop();
    }
    
    if(strcmp(controllerInstruction, "GENERAL_SPEED") == 0) {
      if(strcmp(directionInstruction, "UP") == 0) {
        setGeneralSpeed(1,1,1,1);
        delay(200);
      }

    } else if(strcmp(controllerInstruction, "DIRECTION") == 0) {
      if(strcmp(directionInstruction, "UP") == 0){
        motorsRunning = true;
        setMotorsSpeedUp(1,1,1,1); // increses motors speed
      }
    }
  }

  //float levelX = accelerometerMeassure(0);
  //float levelY = accelerometerMeassure(1);
  
  // setMotorsSpeed(levelY, motorTwoSpeed, motorThreeSpeed, motorFourSpeed);
}

//void startStop(){
  //if(!motorsRunning) {
    //setMotorsSpeed(10, 10, 10, 10);
    //motorsRunning = true;
    //delay(1000);
  //} else {
    //setMotorsSpeed(0, 0, 0, 0);
    //motorsRunning = false;
    //delay(1000);
 // }
//}

void setGeneralSpeed(int motorOne, int motorTwo, int motorThree, int motorFour) {
    Serial.println("aqui");
    //esc1.write(lastMotorOneSpeed + motorOne); //using val as the signal to esc1
    esc2.write(lastMotorTwoSpeed + motorTwo);
    esc3.write(lastMotorThreeSpeed + motorThree);
    esc4.write(lastMotorFourSpeed + motorFour);

    lastMotorOneSpeed = lastMotorOneSpeed + motorOne;
    lastMotorTwoSpeed = lastMotorTwoSpeed + motorTwo;
    lastMotorThreeSpeed = lastMotorThreeSpeed + motorThree;
    lastMotorFourSpeed = lastMotorFourSpeed + motorFour;
}

void setMotorsSpeedUp(float motorOne, int motorTwo, int motorThree, int motorFour) {
    // CREATE METHOD FOR SLOWLY INCREASES SPEED FOR AVOID BUMPS
    esc1.write(motorOne); //using val as the signal to esc1
    esc2.write(motorOne);
    esc3.write(motorOne);
    esc4.write(motorOne);

}

void startMotors(int motorOne, int motorTwo, int motorThree, int motorFour) {
    // Serial.println(mSpeed); // print mSpeed value on Serial monitor (click on Tools->Serial Monitor)
    // starts always at low speed
    esc1.write(motorOne); 
     
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

  // Serial.print(AccX / 2048);
  // Serial.print(" ");
  // Serial.print(AccY / 2048);
  // Serial.print(" ");
  // Serial.print(AccZ / 2048);
  // Serial.print(" ");

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
