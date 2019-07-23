#ifndef __HEATER_H__
#define __HEATER_H__

// Alexander Galilov, alexander.galilov@gmail.com

#include <Arduino.h>

#define HEATER_PIN  5

class Heater {
  private:
    int _currentState;
  public:
    Heater() {

    }

    inline void init() {
      pinMode(HEATER_PIN, OUTPUT);
      digitalWrite(HEATER_PIN, LOW);
      _currentState = LOW;
    }

    void update(int targetPowerPercent) {
      unsigned long msec = millis();
      bool needHeat = (msec  % 500) < 5*targetPowerPercent;
      if (needHeat && _currentState != HIGH) {
        digitalWrite(HEATER_PIN, HIGH);
        _currentState = HIGH;
      } else if (!needHeat && _currentState != LOW) {
        digitalWrite(HEATER_PIN, LOW);
        _currentState = LOW;
      }
    }
};

#endif
