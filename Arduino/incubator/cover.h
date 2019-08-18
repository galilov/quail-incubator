#ifndef __COVER_H__
#define __COVER_H__

#include <Arduino.h>

#define WATER_COVER_PIN 4

class WaterCover {
  private:
    int _coverState;
  public:
    WaterCover() : _coverState(LOW) { // LOW == OPEN
      //
    }

    void setOpened() {
      if (_coverState != LOW) {
        _coverState = LOW;
        digitalWrite(WATER_COVER_PIN, LOW);
      }
    }

    void setClosed() {
      if (_coverState != HIGH) {
        _coverState = HIGH;
        digitalWrite(WATER_COVER_PIN, HIGH);
      }
    }

    bool isOpened() {
      return _coverState == LOW;
    }

    bool isClosed() {
      return !isOpened();
    }

};
#endif
