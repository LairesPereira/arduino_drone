int startButton = 2;

void setup() {
  pinMode(startButton, INPUT);
  Serial.begin(9600);
  
}

void loop() {
  if(digitalRead(startButton) == HIGH) {
    Serial.println("HIGH");
  }
  delay(200);
}
