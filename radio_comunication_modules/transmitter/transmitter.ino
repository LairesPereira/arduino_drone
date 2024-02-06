#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const byte address[6] = "00001";
int startButton = 2;
int speedUpButton = 4;

RF24 radio(7, 8);

void setup() {
  Serial.begin(9600);
  pinMode(startButton, INPUT);
  pinMode(speedUpButton, INPUT);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  if(digitalRead(startButton) == HIGH) {
    const char text[] = "switch_motors_power";
    radio.write(&text, sizeof(text));
    delay(1000);
  } else if(digitalRead(speedUpButton) == HIGH) {
    const char text[] = "speed_up";
    radio.write(&text, sizeof(text));
    Serial.println("aqui");
    delay(1000);
  }

}
