// I2C comunication library
#include <Wire.h>

// Sensor address
const int MPU = 0x68; 

// variables for sensors values
float AccX, AccY, AccZ, Temp, GyrX, GyrY, GyrZ;


void setup() {
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
  Serial.write(" teste");
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  // store data
  AccX = Wire.read() << 8 | Wire.read();
  AccY = Wire.read() << 8 | Wire.read();
  AccZ = Wire.read() << 8 | Wire.read();
  Temp = Wire.read() << 8 | Wire.read();
  GyrX = Wire.read() << 8 | Wire.read();
  GyrY = Wire.read() << 8 | Wire.read();
  GyrZ = Wire.read() << 8 | Wire.read();

  Serial.print(AccX / 2048);
  Serial.print(" ");
  Serial.print(AccY / 2048);
  Serial.print(" ");
  Serial.print(AccZ / 2048);
  Serial.print(" ");

  // convert data to angle
  double pitch = atan(AccX/AccZ);
  double roll = atan(AccY/AccZ);
  Serial.print(pitch);
  Serial.print(" ");
  Serial.println(roll);

  delay(10);
}
