#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ResponsiveAnalogRead.h>

const byte address[6] = "00001";
int switchMotorsPower = 0;

// declare push buttons pins
static const int startButton = 3;
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

// potentiometer variables
int responsivePotReading = 0;
int potentiometerValueRead = 0;
int lastPotentiometerValueRead = 0;
int potentiometerThreshold = 1;
float snapMultiplier = 0.01;

RF24 radio(7, 8);
ResponsiveAnalogRead responsivePot(potentiometer, true, snapMultiplier);

void setup() {
  Serial.begin(9600);

  responsivePot.setAnalogResolution(180);

  pinMode(potentiometer, INPUT); //Sets the pinmode to input
  pinMode(startButton, INPUT);
  pinMode(speedUpBtn, INPUT);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {

    responsivePotReading = analogRead(potentiometer);
    responsivePot.update(responsivePotReading);
    int responsivePotValue = responsivePot.getValue();

    currentMillis = millis();    // store the current time
    startBtnState = digitalRead(startButton);
    speedUpBtnState = digitalRead(speedUpBtn);

    if(responsivePotValue >= lastPotentiometerValueRead + potentiometerThreshold ||
       responsivePotValue <= lastPotentiometerValueRead - potentiometerThreshold) {
      Serial.print("Potentiometer value: ");
      if(potentiometerValueRead < 5) {
        Serial.println( responsivePotValue ); //Print the value in the serial monitor
        speedUp(responsivePotValue);
        lastPotentiometerValueRead = responsivePotValue;
        responsivePotValue = 0;
      }
      
    }
      
}

void start() {
    const char text[] = "switch_motors_power";
    radio.write(&text, sizeof(text));
    startBtnState = startBtnLastState;
}

void speedUp(int potentiometerValue) {
    const char text[] = "speed_up";
    radio.write(&text, sizeof(text));
    radio.write(&potentiometerValue, sizeof(potentiometerValue));
    speedUpBtnState = speedUpBtnLastState;
   
}
