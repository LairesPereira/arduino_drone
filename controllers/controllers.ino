#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ResponsiveAnalogRead.h>

const byte address[6] = "00001";
int switchMotorsPower = 0;

// declare push buttons pins
static const int startButton = 2;
static const int speedUpBtn = 4;                    // switch pin
int buttonStatePrevious = LOW;                      // previousstate of the switch
int potentiometer = A0; //Assign to pin A0

unsigned long minButtonLongPressDuration = 500;    // Time we wait before we see the press as a long press
unsigned long buttonLongPressMillis;                // Time in ms when we the button was pressed
bool buttonStateLongPress = false;                  // True if it is a long press

const int intervalButton = 50;                      // Time between two readings of the button state
unsigned long previousButtonMillis;                 // Timestamp of the latest reading

unsigned long buttonPressDuration;                  // Time the button is pressed in ms

//// GENERAL ////

unsigned long currentMillis;          // Variabele to store the number of milleseconds since the Arduino has started

// declare btn states for avoiding double click on push buttons
int startBtnState = 0;
int startBtnLastState = 0;
int speedUpBtnState = 0;
int speedUpBtnLastState = 0;

// joystick config
int eixo_X_speed = A3; //PINO REFERENTE A LIGAÇÃO DO EIXO X DO PINO DE VELOCIDADE GERAL DOS MOTORES
int eixo_X= A1; //PINO REFERENTE A LIGAÇÃO DO EIXO X
int eixo_Y = A2; //PINO REFERENTE A LIGAÇÃO DO EIXO Y
int switchJoystick = 2;

RF24 radio(7, 8);

void setup() {
  Serial.begin(9600);
  pinMode(potentiometer, INPUT); 
  pinMode(startButton, INPUT);
  pinMode(speedUpBtn, INPUT);
  pinMode(switchJoystick, INPUT_PULLUP);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
    readJoystick();
    currentMillis = millis();    // store the current time
    startBtnState = digitalRead(startButton);
    speedUpBtnState = digitalRead(speedUpBtn);      
}

void readJoystick() {
    if((digitalRead(switchJoystick)) == 0) {
      sendEmergencyBreak();
    }
    if(analogRead(eixo_X_speed) == 0) {
      while(analogRead(eixo_X_speed) == 0) {
        sendSpeed("UP");
        delay(200);
      } 
    } else if(analogRead(eixo_X_speed) == 1023) {
      while(analogRead(eixo_X_speed) == 1023) {
        sendSpeed("DOWN");
        delay(200);
      }
    }

    if((analogRead(eixo_X)) == 0){ //SE LEITURA FOR IGUAL A 0, FAZ
        while(analogRead(eixo_X) == 0) {
        sendDirection("UP");
        delay(500);
        }
    }else{
          if((analogRead(eixo_X)) == 1023){ //SE LEITURA FOR IGUAL A 1023, FAZ
              sendDirection("DOWN");
          }else{
                if((analogRead(eixo_Y)) == 0){ //SE LEITURA FOR IGUAL A 0, FAZ
                  sendDirection("RIGHT");
                }else{
                      if((analogRead(eixo_Y)) == 1023){ //SE LEITURA FOR IGUAL A 1023, FAZ
                          sendDirection("LEFT");
                      }
                }
          }
    }
    delay(100); //INTERVALO DE 500 MILISSEGUNDOS
}

void start() {
    const char text[] = "switch_motors_power";
    radio.write(&text, sizeof(text));
    startBtnState = startBtnLastState;
}

void sendSpeed(String speedDirection) {
  const char instruction[] = "GENERAL_SPEED";
  if(speedDirection == "UP") {
    const char direction[] = "UP";
    radio.write(&instruction, sizeof(instruction));
    radio.write(&direction, sizeof(direction));
  } else if(speedDirection == "DOWN") {
    const char direction[] = "DOWN";
    Serial.println("aqui");
    radio.write(&instruction, sizeof(instruction));
    radio.write(&direction, sizeof(direction));
  } 
}

void sendEmergencyBreak() {
    const char instruction[] = "EMERGENCY BREAK";
    radio.write(&instruction, sizeof(instruction));
    Serial.println("EMERGENCY BREAK");
}

void sendDirection(String joystickInstruction) {
    const char instruction[] = "DIRETCTION";

    if(joystickInstruction == "UP"){
      const char direction[] = "UP";
      radio.write(&instruction, sizeof(instruction));
      radio.write(&direction, sizeof(direction));
    } else if (joystickInstruction == "DOWN") {
      const char direction[] = "DOWN";
      radio.write(&instruction, sizeof(instruction));
      radio.write(&direction, sizeof(direction));
    } else if (joystickInstruction == "LEFT") {
      const char direction[] = "LEFT";
      radio.write(&instruction, sizeof(instruction));
      radio.write(&direction, sizeof(direction));
    } else if (joystickInstruction == "RIGHT") {
      const char direction[] = "RIGHT";
      radio.write(&instruction, sizeof(instruction));
      radio.write(&direction, sizeof(direction));
    }

    String direction = joystickInstruction;
    Serial.println(joystickInstruction);
    Serial.println(instruction);
}
