#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define POT A1

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long start_time;
int valorPot = 0;
int count = 0; 
int fan_control = 3;
int rpm;

void setup() {
  lcd.init();
  Serial.begin(9600);
  pinMode(POT, INPUT);
  pinMode(fan_control, OUTPUT);
}

void loop() {
  valorPot = analogRead(POT);
  Serial.println(valorPot);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("bY Laires!! RPM:");
  lcd.setCursor(0, 1);
  delay(1000);
  if(valorPot >= 0 && valorPot <= 250) {
    analogWrite(fan_control, 0);
    lcd.print((valorPot * 10) / 2);
  }
  
  if(valorPot >= 251 && valorPot <= 500) {
    analogWrite(fan_control, 50);
    lcd.print((valorPot * 10) / 2);
  }

  if(valorPot >= 500 && valorPot <= 750) {
    analogWrite(fan_control, 100);
    lcd.print((valorPot * 10) / 2);
  }

  if(valorPot >= 751 && valorPot <= 1000) {
    analogWrite(fan_control, 180);
    lcd.print((valorPot * 10) / 2);
  }
  if(valorPot >= 1000) {
    analogWrite(fan_control, 255);
    lcd.print((valorPot * 10) / 2);
  }
}

void counter() {
  count++;
}