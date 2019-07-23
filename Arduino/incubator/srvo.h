#ifndef __SRVO_H__
#define __SRVO_H__

// Alexander Galilov, alexander.galilov@gmail.com

#include <Servo.h>
#include <Arduino.h>

class Srvo {
  private:
    Servo _srv;
    int _currentPosition;
    unsigned long _prevUpdate = 0;
  public:
    const int SrvoPin = 9;
    const int NPosition = 69; // degrees
    const int LPosition = NPosition - 23; // degrees
    const int RPosition = NPosition + 30; // degrees;

    void init() {
      _srv.attach(SrvoPin);
      _currentPosition = NPosition;
      _srv.write(_currentPosition);
    }

    bool updatePositionToTarget(char posCode) {
      unsigned long msec = millis();
      if (_prevUpdate > msec) {
        _prevUpdate = 0;
      }
      if (msec - _prevUpdate <= 30) {
        return false;
      }
      _prevUpdate = msec;
      int targetPosition = _currentPosition;
      switch (posCode) {
        case 'N':
          targetPosition = NPosition;
          break;
        case 'L':
          targetPosition = LPosition;
          break;
        case 'R':
          targetPosition = RPosition;
          break;
      }
      if (_currentPosition > targetPosition) {
        _currentPosition--;
        _srv.write(_currentPosition);
      } else if (_currentPosition < targetPosition) {
        _currentPosition++;
        _srv.write(_currentPosition);
      } else {
        _srv.write(_currentPosition);
        return true;
      }
      return false;
    }
};


#endif
