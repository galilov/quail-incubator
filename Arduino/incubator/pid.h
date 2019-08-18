#ifndef __PID_H__
#define __PID_H__

// This code is based on https://www.embeddedrelated.com/showarticle/943.php , (c) Tim Wescott
// https://wiki.roboforum.ru/index.php -- in russian
// Alexander Galilov, alexander.galilov@gmail.com

#include <Arduino.h>

class PIDRegulator {
  private:
    double _dState;      // Last position input
    double _iState;      // Integrator state
    double _output;
    unsigned long _prevMsec;
  public:
    unsigned int TimeStepMsec = 15000;
    double MaxOutput = 100, MinOutput = -100;
    double IntegralGain = 0.6,      // integral gain
           ProportionalGain = 50.0,      // proportional gain
           DerivativeGain = 500;     // derivative gain
    double IntegratorMaximum = 200, IntegratorMinimum = -200;  // Maximum and minimum allowable integrator state
  public:
    PIDRegulator(): _prevMsec(0), _dState(0), _iState(0) {
      //
    }

    inline double getOutput() {
      return _output;
    }

    void init() {
      _dState = 0;
      _iState = 0;
      _output = 0;
      _prevMsec = 0;
    }

    bool update(double error, double currentValue) {
      unsigned long msec = millis();
      if (_prevMsec > msec) {
        _prevMsec = 0;
      }
      unsigned long d = msec - _prevMsec;
      if (d > TimeStepMsec) {
        _prevMsec = msec;
        //double timeStepSec = d / 1000.0;
        double pTerm, dTerm, iTerm;
        pTerm = ProportionalGain * error;    // calculate the proportional term
        if (pTerm < MaxOutput && pTerm > MinOutput) _iState += error;                    // calculate the integral state with appropriate limiting
        if (_iState > IntegratorMaximum)
          _iState = IntegratorMaximum;
        else if (_iState < IntegratorMinimum)
          _iState = IntegratorMinimum;
        iTerm = IntegralGain * _iState;       // calculate the integral term
        dTerm = DerivativeGain * (currentValue - _dState);
        _dState = currentValue;
        _output = (pTerm + iTerm - dTerm);
        if (_output > MaxOutput) _output = MaxOutput; else if (_output < MinOutput) _output = MinOutput;
        //        Serial.print("output:");
        //        Serial.print(_output);
        //        Serial.print(", t:");
        //        Serial.print(currentValue);
        //        Serial.print(", err:");
        //        Serial.print(error);
        //        Serial.print(", iTerm:");
        //        Serial.print(iTerm);
        //        Serial.print(", _iState:");
        //        Serial.print(_iState);
        //        Serial.println();
        return true;
      }
      return false;
    }

};

#endif
