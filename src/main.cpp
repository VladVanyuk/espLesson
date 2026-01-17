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
uint8_t buttonState;            // the current reading from the input pin
uint8_t buttonPressed = 0;     // whether the button is pressed
uint8_t lastButtonState = LOW;  // the previous reading from the input pin

uint16_t adcValue = 0;
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
  pinMode(button_pin, INPUT);
  pinMode(adc_read_pin, INPUT);

  pinMode(led_rgb_pin, OUTPUT);
  digitalWrite(led_rgb_pin, HIGH);

  pinMode(red_led_pin, OUTPUT);
  digitalWrite(red_led_pin, LOW);
}

void initSerial()
{
#if (DEBUG == 1) 
  Serial.begin(UART_SPD);
  // delay(10);
  // Serial.flush();
  Serial.println();
  
#endif
}


bool changeRedLedState(uint16_t new_val) // 0 - 1023
{
  if (new_val > 20 &&  buttonPressed)
  {
    analogWrite(red_led_pin, new_val / 4); // write pwm duty cycle with new value as 0 - 255 
    return true;
  }
  else
  {
    analogWrite(red_led_pin, LOW);
  }
  return false;
}


void buttonHandler()
{
  // read the state of the switch into a local variable:
  bool reading = digitalRead(button_pin);  
  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > debounceDelay) {
   
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        buttonPressed = !buttonPressed;
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


uint16_t adcReadHandler(pin_t pin)
{
  static uint32_t lastReadADC = 0;
 
  if(!buttonPressed)
  {
    DEBUG_PRINT("ADC read skipped, button not pressed");
    return 0;
  }

  if(now - lastReadADC > 10)
  {
    //uint16_t adcSamples[10] = {};
    uint16_t adcValueSum = 0;
    for (int i = 0; i < 10; i++) {
      adcValue = analogRead((uint8_t)pin);
      //adcSamples[i] = adcValue;
      adcValueSum += adcValue;
    }
    adcValue = adcValueSum / 10;
    // uint16_t adcValue = 0;
    // for (int i = 0; i < 10; i++) {
    //   adcValue += adcSamples[i];
    // }
    // adcValue /= 10;
    lastReadADC = now;
    DEBUG_PRINT("ADC Value: " + String(adcValue));
    return adcValue;
  }
  return adcValue;
}


void setup()
{

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
      //TODO addc web fS init
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
  now = millis();
}


void loop()
{
  static uint32_t lastPrint = 0;
  now = millis();

  buttonHandler();
  uint16_t adcValue = adcReadHandler(adc_read_pin);
  bool status = changeRedLedState(adcValue);

#if (DEBUG == 1) 
  if(now - lastPrint > 1000) {
    DEBUG_PRINT("buttonPressed: ");
    DEBUG_PRINT(buttonPressed);

    DEBUG_PRINT("Led value func status: ");
    DEBUG_PRINT(status);

    DEBUG_PRINT("ADC Value: ");
    DEBUG_PRINT(adcValue);

    lastPrint = now;
  }
#endif
}
