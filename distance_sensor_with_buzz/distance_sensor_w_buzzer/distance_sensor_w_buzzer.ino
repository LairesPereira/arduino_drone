#include <Ultrasonic.h>

#define pino_trigger 4
#define pino_echo 5

Ultrasonic ultrasonic(pino_trigger, pino_echo);

void setup() {
  pinMode(8, OUTPUT);
  Serial.begin(9600);
  Serial.println("Sensor de distancia: ");

}

void loop() {
  float distancia = ultrasonic.read();
  if(distancia < 10) {
    tone(8, 2000);
  } else {
    noTone(8);
  }

  //distancia = ultrasonic.convert(tempo, Ultrasonic::CM)

  Serial.print("Distancia em cm: ");
  Serial.println(distancia);
  delay(500);
}

