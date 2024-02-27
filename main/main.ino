#include <Servo.h>//Using servo library to control esc1
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Kalman.h>

Servo right_prop;
Servo left_prop;

/*MPU-6050 gives you 16 bits data so you have to create some 16int constants
 * to store the data for accelerations and gyro*/

int16_t Acc_rawX, Acc_rawY, Acc_rawZ,Gyr_rawX, Gyr_rawY, Gyr_rawZ;
 
float Acceleration_angle[2];
float Gyro_angle[2];
float Total_angle[2];

float elapsedTime, time, timePrev;
int i;
float rad_to_deg = 180/3.141592654;

float PID, pwmLeft, pwmRight, error, previous_error;
float pid_p=0;
float pid_i=0;
float pid_d=0;

/////////////////PID CONSTANTS/////////////////
double kp=0.11;//3.55
double ki=0.0011;//0.003
double kd=0.3;//2.05

// BOM RESULTADO
//double kp=8.9;//3.55
//double ki=0.00095;//0.003
//double kd=1.2;//2.05
///////////////////////////////////////////////

double throttle=1500; //initial value of throttle to the motors
float desired_angle = 0; //This is the angle in which we whant the
                         //balance to stay steady

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
bool firstStart = true;

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
  Serial.begin(250000);

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  right_prop.attach(6); //attatch the right motor to pin 3
  left_prop.attach(5);  //attatch the left motor to pin 5

  time = millis(); //Start counting time in milliseconds
  /*In order to start up the ESCs we have to send a min value
   * of PWM to them before connecting the battery. Otherwise
   * the ESCs won't start up or enter in the configure mode.
   * The min value is 1000us and max is 2000us, REMEMBER!*/
  
  left_prop.writeMicroseconds(2000);
  right_prop.writeMicroseconds(2000);
  Serial.println("\n");
  Serial.println("CONNECT BATTERY");
  delay(200); /*Give some delay, 7s, to have time to connect
                *the propellers and let everything start up*/ 
  left_prop.writeMicroseconds(1000);
  right_prop.writeMicroseconds(1000);
 
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
  readAngle();
  pidCalc();
  correctionSpeed();
  
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
        //previousMillis = millis();
        throttle += 10;
        
      } else if(strcmp(directionInstruction, "DOWN") == 0) {
        left_prop.writeMicroseconds(1000);
        right_prop.writeMicroseconds(1000);
        delay(3000);
      }

    } else if(strcmp(controllerInstruction, "DIRECTION") == 0) {
      if(strcmp(directionInstruction, "UP") == 0){
        motorsRunning = true;
        setMotorsSpeed(1,1,1,1); // increses motors speed
      }
    }
  }
}


void readAngle() {
  /////////////////////////////I M U/////////////////////////////////////
    timePrev = time;  // the previous time is stored before the actual time read
    time = millis();  // actual time read
    elapsedTime = (time - timePrev) / 1000; 
  
  /*The tiemStep is the time that elapsed since the previous loop. 
   * This is the value that we will use in the formulas as "elapsedTime" 
   * in seconds. We work in ms so we haveto divide the value by 1000 
   to obtain seconds*/

  /*Reed the values that the accelerometre gives.
   * We know that the slave adress for this IMU is 0x68 in
   * hexadecimal. For that in the RequestFrom and the 
   * begin functions we have to put this value.*/
   
     Wire.beginTransmission(0x68);
     Wire.write(0x3B); //Ask for the 0x3B register- correspond to AcX
     Wire.endTransmission(false);
     Wire.requestFrom(0x68,6,true); 
   
   /*We have asked for the 0x3B register. The IMU will send a brust of register.
    * The amount of register to read is specify in the requestFrom function.
    * In this case we request 6 registers. Each value of acceleration is made out of
    * two 8bits registers, low values and high values. For that we request the 6 of them  
    * and just make then sum of each pair. For that we shift to the left the high values 
    * register (<<) and make an or (|) operation to add the low values.*/
    
     Acc_rawX=Wire.read()<<8|Wire.read(); //each value needs two registres
     Acc_rawY=Wire.read()<<8|Wire.read();
     Acc_rawZ=Wire.read()<<8|Wire.read();
     //Serial.print(" TESTE ");
     //Serial.print(Acc_rawX);
     //Serial.print(" ");
     //Serial.print(Acc_rawY);
     //Serial.print(" ");
     //Serial.print(Acc_rawZ);

 
    /*///This is the part where you need to calculate the angles using Euler equations///*/
    
    /* - Now, to obtain the values of acceleration in "g" units we first have to divide the raw   
     * values that we have just read by 16384.0 because that is the value that the MPU6050 
     * datasheet gives us.*/
    /* - Next we have to calculate the radian to degree value by dividing 180º by the PI number
    * which is 3.141592654 and store this value in the rad_to_deg variable. In order to not have
    * to calculate this value in each loop we have done that just once before the setup void.
    */

    /* Now we can apply the Euler formula. The atan will calculate the arctangent. The
     *  pow(a,b) will elevate the a value to the b power. And finnaly sqrt function
     *  will calculate the rooth square.*/
     /*---X---*/
     Acceleration_angle[0] = atan((Acc_rawY/16384.0)/sqrt(pow((Acc_rawX/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;
     /*---Y---*/
     Acceleration_angle[1] = atan(-1*(Acc_rawX/16384.0)/sqrt(pow((Acc_rawY/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;

     //Serial.print(" ");
     //Serial.print(Acceleration_angle[0]);
     //Serial.print(" ");
     //Serial.println(Acceleration_angle[1]);

 
   /*Now we read the Gyro data in the same way as the Acc data. The adress for the
    * gyro data starts at 0x43. We can see this adresses if we look at the register map
    * of the MPU6050. In this case we request just 4 values. W don¡t want the gyro for 
    * the Z axis (YAW).*/
    
   Wire.beginTransmission(0x68);
   Wire.write(0x43); //Gyro data first adress
   Wire.endTransmission(false);
   Wire.requestFrom(0x68,4,true); //Just 4 registers
   
   Gyr_rawX=Wire.read()<<8|Wire.read(); //Once again we shif and sum
   Gyr_rawY=Wire.read()<<8|Wire.read();
 
   /*Now in order to obtain the gyro data in degrees/seconda we have to divide first
   the raw value by 131 because that's the value that the datasheet gives us*/

   /*---X---*/
   Gyro_angle[0] = Gyr_rawX/131.0; 
   /*---Y---*/
   Gyro_angle[1] = Gyr_rawY/131.0;

   /*Now in order to obtain degrees we have to multiply the degree/seconds
   *value by the elapsedTime.*/
   /*Finnaly we can apply the final filter where we add the acceleration
   *part that afects the angles and ofcourse multiply by 0.98 */

   /*---X axis angle---*/
   Total_angle[0] = 0.98 *(Total_angle[0] + Gyro_angle[0]*elapsedTime) + 0.02*Acceleration_angle[0];
   /*---Y axis angle---*/
   //Total_angle[1] = 0.98 *(Total_angle[1] + Gyro_angle[1]*elapsedTime) + 0.02*Acceleration_angle[1];
   //int teste = accelerometerMeassure(1);  
   Total_angle[1] = (float)accelerometerMeassure(1); 
   /*Now we have our angles in degree and values from -10º0 to 100º aprox*/
   Serial.print(Total_angle[1]);
   //Serial.print(" ");
   //Serial.print(teste);
   Serial.print(" ");

}

void pidCalc() {
  /*///////////////////////////P I D///////////////////////////////////*/
/*Remember that for the balance we will use just one axis. I've choose the x angle
to implement the PID with. That means that the x axis of the IMU has to be paralel to
the balance*/

/*First calculate the error between the desired angle and 
*the real measured angle*/
error = Total_angle[1] - desired_angle;
    
/*Next the proportional value of the PID is just a proportional constant
*multiplied by the error*/

pid_p = kp*error;

/*The integral part should only act if we are close to the
desired position but we want to fine tune the error. That's
why I've made a if operation for an error between -2 and 2 degree.
To integrate we just sum the previous integral value with the
error multiplied by  the integral constant. This will integrate (increase)
the value each loop till we reach the 0 point*/
if(-3 <error <3)
{
  pid_i = pid_i+(ki*error);  
}

/*The last part is the derivate. The derivate acts upon the speed of the error.
As we know the speed is the amount of error that produced in a certain amount of
time divided by that time. For taht we will use a variable called previous_error.
We substract that value from the actual error and divide all by the elapsed time. 
Finnaly we multiply the result by the derivate constant*/

pid_d = kd*((error - previous_error)/elapsedTime);

/*The final PID values is the sum of each of this 3 parts*/
PID = pid_p + pid_i + pid_d;

}

void correctionSpeed() {
  
/*We know taht the min value of PWM signal is 1000us and the max is 2000. So that
tells us that the PID value can/s oscilate more than -1000 and 1000 because when we
have a value of 2000us the maximum value taht we could sybstract is 1000 and when
we have a value of 1000us for the PWM sihnal, the maximum value that we could add is 1000
to reach the maximum 2000us*/
if(PID < -1000)
{
  PID=-1000;
}
if(PID > 1000)
{
  PID=1000;
}

/*Finnaly we calculate the PWM width. We sum the desired throttle and the PID value*/
pwmLeft = throttle + PID;
pwmRight = throttle - PID;


/*Once again we map the PWM values to be sure that we won't pass the min
and max values. Yes, we've already maped the PID values. But for example, for 
throttle value of 1300, if we sum the max PID value we would have 2300us and
that will mess up the ESC.*/
//Right
if(pwmRight < 1000)
{
  pwmRight= 1000;
}
if(pwmRight > 2000)
{
  pwmRight=2000;
}
//Left
if(pwmLeft < 1000)
{
  pwmLeft= 1000;
}
if(pwmLeft > 2000)
{
  pwmLeft=2000;
}

/*Finnaly using the servo function we create the PWM pulses with the calculated
width for each pulse*/

//Serial.print(PID);
Serial.print(" ");
Serial.print(throttle);
Serial.print(" ");
Serial.print(pwmLeft);
Serial.print(" ");
Serial.println(pwmRight);

left_prop.writeMicroseconds(pwmLeft);
right_prop.writeMicroseconds(pwmRight);
previous_error = error; //Remember to store the previous error.

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
    esc1.writeMicroseconds(motorOne);
    esc2.writeMicroseconds(motorTwo);
    esc3.writeMicroseconds(motorThree);
    esc4.writeMicroseconds(motorFour);

    //esc1.write(motorOne); //using val as the signal to esc1
    //esc2.write(motorOne);
    //esc3.write(motorOne);
    //esc4.write(motorOne);
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

  //Serial.print(" ");
  //Serial.println(kalAngleY);
  if(kalAngleY - 1.1 > 0.05 || kalAngleY - 1.1 < -0.05){
    return kalAngleY - 1.1; // subtracao para correcao de erro observado com sensor em repouso
  } else {
    return 0;
  }
  
}

void setAngleCorrection(float kalAngleX) {
  // stabilize motors speed only when reach correct level and 
  // not recived any controller instruction for the last 300 milliseconds
  if(kalAngleX >= -2 && kalAngleX < 0) {
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;  
    }
  }
  if (
      kalAngleX < -2.5 && 
      lastMotorOneSpeed <= 169 &&
      lastMotorTwoSpeed <= 169 &&
      lastMotorThreeSpeed <= 169 &&
      lastMotorFourSpeed <= 169
     ) {
      esc3.write(lastMotorThreeSpeed + 1);
      esc4.write(lastMotorFourSpeed + 1);
      lastMotorThreeSpeed++;
      lastMotorFourSpeed++;
      
      delay(30);
  }
}