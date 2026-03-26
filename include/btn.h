#ifndef BTN_H
#define BTN_H

#include <Arduino.h>
#include "pinsSetup.h"

typedef struct
{
  uint8_t buttonState;
  uint8_t buttonPressed;
  volatile uint8_t lastButtonState;
  uint8_t selfPin;
  uint32_t lastDebounceTime;
  uint32_t debounceDelay;
  ButtonControl *next;
} ButtonControl;

ButtonControl btnPower = { // btn2
    .buttonState = 0,
    .buttonPressed = 0,
    .lastButtonState = LOW,
    .selfPin = button_pin,
    .lastDebounceTime = 0,
    .debounceDelay = 50,
    .next = nullptr};

// ButtonControl btn1 = {
//   .buttonState = 0,
//   .buttonPressed = 0,
//   .lastButtonState = LOW,
//   .selfPin = button_pin,
//   .lastDebounceTime = 0,
//   .debounceDelay = 50,
//  .next = &btnPower
// };

volatile bool setupInterruptFlag = false;

volatile bool isrHappened = false;
volatile uint32_t lastTimeInterrupt = 0;

ICACHE_RAM_ATTR void isrButtonPress()
{
  //btnPower.buttonPressed = !btnPower.buttonPressed;
  isrHappened  = true;
  lastTimeInterrupt = millis();
  btnPower.lastButtonState = digitalRead(btnPower.selfPin); // !btn->lastButtonState
}

//IRAM_ATTR
ICACHE_RAM_ATTR void isrButtonPressSetup()
{
    setupInterruptFlag = true; 
    detachInterrupt(digitalPinToInterrupt(button_pin));
   // attachInterrupt(digitalPinToInterrupt(button_pin), isrButtonPress, CHANGE);
}



void buttonHandler(uint32_t time_now, ButtonControl *btn)
{
  if(isrHappened)
  {
    DEBUG_PRINT("ISR happened");
   if (btn == nullptr)
    return;

  

    if ((time_now - lastTimeInterrupt) > btn->debounceDelay)
    {
      // try one more reading

      if (btnPower.lastButtonState != btn->buttonState)
      {
        btn->buttonState = btnPower.lastButtonState;

        if (btn->buttonState == HIGH)
        {
          btn->buttonPressed = !btn->buttonPressed;
        }
      }
    }
    isrHappened = false;
    DEBUG_PRINT("Button state: " + String(btn->buttonState));
  }
}

void buttonsHandler(uint32_t time_now, ButtonControl *btn)
{
  if (btn == nullptr)
    return;

  ButtonControl *firstBtn = btn;

  do
  {

    // read the state of the switch into a local variable:
    bool reading = digitalRead(btn->selfPin);
    if (reading != btn->lastButtonState)
    {
      btn->lastDebounceTime = time_now;
    }

    if ((time_now - btn->lastDebounceTime) > btn->debounceDelay)
    {

      if (reading != btn->buttonState)
      {
        btn->buttonState = reading;

        if (btn->buttonState == HIGH)
        {
          btn->buttonPressed = !btn->buttonPressed;
        }
      }
    }

    // save the reading. Next time through the loop, it'll be the lastButtonState:
    btn->lastButtonState = reading;

    // if (btn->next == nullptr)
    //   {
    //     btn = firstBtn;
    //   }
    // else
    // {
    //   btn = btn->next;
    // }

  } while (btn != nullptr && btn->next != nullptr);
}

#endif // BTN_H