#ifndef ESP_IDF_CONFIG
#define ESP_IDF_CONFIG 0
#endif




#if (ESP_IDF_CONFIG == 0)
#include <Arduino.h>
#elif (ESP_IDF_CONFIG == 1)
#endif

#include <LittleFS.h>

#include "pinsSetup.h"
#include "serialDebug.h"
#include "states.h"

#define FORMAT_LITTLEFS_IF_FAILED true


uint8_t state = STATE_START;
uint32_t now = 0;
 
// // Variables will change:

typedef struct {
  uint8_t buttonState;
  uint8_t buttonPressed;
  uint8_t lastButtonState;
  uint8_t selfPin;
  uint32_t lastDebounceTime;
  uint32_t debounceDelay;
 ButtonControl *next;
} ButtonControl;


ButtonControl btnPower = { //btn2
  .buttonState = 0,
  .buttonPressed = 0,
  .lastButtonState = LOW,
  .selfPin = button_pin,
  .lastDebounceTime = 0,
  .debounceDelay = 50,
 .next = nullptr
};



// ButtonControl btn1 = {
//   .buttonState = 0,
//   .buttonPressed = 0,
//   .lastButtonState = LOW,
//   .selfPin = button_pin,
//   .lastDebounceTime = 0,
//   .debounceDelay = 50,
//  .next = &btnPower
// };


uint16_t adcValue = 0;
// struct flagsBtnLed
// {
//   uint8_t buttonState : 1;
//   uint8_t ledState : 1;
//   uint8_t lastButtonState : 1;
// };


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.



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


bool changePWMState(uint16_t new_val) // 0 - 1023
{
  if (new_val > 20 &&  btnPower.buttonPressed)
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


void buttonHandler(ButtonControl *btn)
{
  if(btn == nullptr) return;

  ButtonControl *firstBtn = btn;

  do
  {
    /* code */
  
  

  // read the state of the switch into a local variable:
  bool reading = digitalRead(btn->selfPin);  
  if (reading != btn->lastButtonState) {
    btn->lastDebounceTime = now;
  }

  if ((now - btn->lastDebounceTime) > btn->debounceDelay) {
   
    if (reading != btn->buttonState) {
      btn->buttonState = reading;

      if (btn->buttonState == HIGH) {
        btn->buttonPressed = !btn->buttonPressed;
      }
    }
  }


  // save the reading. Next time through the loop, it'll be the lastButtonState:
  btn->lastButtonState = reading;

  if(btn->next == nullptr)
    btn = firstBtn;
  else
    btn = btn->next;
  
  } while  (btn != nullptr && btn->next != nullptr);

  
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
 
  if(!(btnPower.buttonPressed))
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
      if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        DEBUG_PRINT("LittleFS Mount Failed");
        state = STATE_ERROR;
      }
      else 
      {
        DEBUG_PRINT("LittleFS Mount OK");
         state = STATE_READ_FS_CONFIG;
      }
      break; 
      //TODO addc web fS init

    case STATE_READ_FS_CONFIG:
      DEBUG_PRINT("Reading FS Config file");
      //! VALUE VERSIUON IS NOT INT VALUE
      File f1 = LittleFS.open("/config.json", "r");
      //to do add if statement for creating default config file
      if(f1)
      {
        f1.close();
      }
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
  now = millis();
}


void debugPrintHandler(bool status = false, uint16_t adcValue = 0)
{
#if (DEBUG == 1) 
  static uint32_t lastPrint = 0;
  if(now - lastPrint > 1000) { // todo make time in variable or in define 
    DEBUG_PRINT("buttonPressed: ");
    DEBUG_PRINT(btnPower.buttonPressed);

    DEBUG_PRINT("Led value func status: ");
    DEBUG_PRINT(status);

    DEBUG_PRINT("ADC Value: ");
    DEBUG_PRINT(adcValue);

    lastPrint = now;
  }
#endif 
}

void loop()
{
  now = millis();
  buttonHandler(&btnPower); // 1
  uint16_t adcValue = adcReadHandler(adc_read_pin); // 2
  bool status = changePWMState(adcValue); // 3
  debugPrintHandler(status, adcValue); // 4

}



/*

1) sensor
2) button
3) wifi control = server + cmd client
4) mqtt 
5) telegram bot 

*/