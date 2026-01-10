#ifndef ESP_IDF_CONFIG
#define ESP_IDF_CONFIG 0
#endif

#if (ESP_IDF_CONFIG == 0)
#include <Arduino.h>
#elif (ESP_IDF_CONFIG == 1)
#endif

#include "pinsSetup.h"
#include "serialDebug.h"
#include "states.h"

uint8_t state = STATE_START;
uint32_t now = 0;
 
// // Variables will change:
uint8_t ledState = HIGH;        // the current state of the output pin
uint8_t buttonState;            // the current reading from the input pin
uint8_t lastButtonState = LOW;  // the previous reading from the input pin

// struct flagsBtnLed
// {
//   uint8_t buttonState : 1;
//   uint8_t ledState : 1;
//   uint8_t lastButtonState : 1;
// };


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
uint32_t lastDebounceTime = 0;  // the last time the output pin was toggled
uint32_t debounceDelay = 50;    // the debounce time; increase if the output flickers


#if (ESP_IDF_CONFIG == 0)

void initPins()
{
  pinMode(PIN_BUTTON, INPUT);

  pinMode(PIN_LED_RGB, OUTPUT);
  digitalWrite(PIN_LED_RGB, LOW);

  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);
}

void initSerial()
{
#ifdef DEBUG
  Serial.begin(UART_SPD);
  // delay(10);
  Serial.flush();
  Serial.println();
#endif
}


void changeRedLedState(bool state)
{
  if (state)
  {
    digitalWrite(PIN_LED_RED, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED_RED, LOW);
  }
}

void buttonHandler()
{
  // read the state of the switch into a local variable:
  bool reading = digitalRead(PIN_BUTTON);
  uint32_t now = millis();
 
  
  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > debounceDelay) {
   
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }


  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}


#elif (ESP_IDF_CONFIG == 1)

// if esp-idf selected
// voi9d moid
// {
//   setup();
//   while (1) {
//     loop();
//   }
// }

void initPins()
{
  // Pin initialization for ESP-IDF can be added here
}

void initSerial()
{
}

#endif


void setup()
{
  DEBUG_PRINT("INIT START");

 
  bool initialized = false;
  while (!initialized)
  {
    switch (state)
    {

    case STATE_START:
      state = STATE_INIT_PINS;
      break;

    case STATE_INIT_PINS:
      initPins();
      state = STATE_SERIAL_INIT;
      break;

    case STATE_SERIAL_INIT:
      initSerial();
      state = STATE_INIT_FS;
      break;

    case STATE_INIT_FS:
      DEBUG_PRINT("State is 1");
      state = STATE_READY;
      break;
    case STATE_READY:
      DEBUG_PRINT("State is 2");
      initialized = true;
      break;

    case STATE_ERROR:
      DEBUG_PRINT("ERROR");
      delay(100000);
      ESP.restart();
      break;
    default:
      DEBUG_PRINT("Unknown State");
      break;
    }
  }

  DEBUG_PRINT("INIT DONE");
}

void loop()
{
  
  buttonHandler();
  changeRedLedState(ledState);
}
