int motorPin =9;// pin to connect to motor module
int mSpeed = 0;// variable to hold speed value
int mStep = 15;// increment/decrement step for PWM motor speed
  
void setup() {
  // Robojax.com demo
  pinMode(motorPin,OUTPUT);// set mtorPin as output
  Serial.begin(9600);// initialize serial motor  
}

void loop() {
  // Robojax.com  tutorial

analogWrite(motorPin, mSpeed);// send mSpeed value to motor
  Serial.println(mSpeed); // print mSpeed value on Serial monitor (click on Tools->Serial Monitor)
  mSpeed = mSpeed + mStep;
  if (mSpeed <= 0 || mSpeed >= 255) {
    mStep = -mStep;
    if(mSpeed == 255) {
      delay(5000);
    }
  }  
  
delay(200);
}
