#ifndef __SHT_31_H__
#define __SHT_31_H__

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

class Sht31 {
    float temperatures[10];
    float humidities[3];
    int temperatureIndex = 0;
    int humidityIndex = 0;
    unsigned long prevTemperatureMeasurement = 0;
    unsigned long prevHumidityMeasurement = 0;
    float currentTemperature = 0;
    int currentHumidity = 0;
    bool isTemperatireError, isHumidityError;
    Adafruit_SHT31 sht31;
  public:
    Sht31() : isTemperatireError(false), isHumidityError(false) {
      
    }

    inline bool init() {
      if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
        return false;
      }
      return true;
    }

    inline float getCurrentTemperature() {
      return currentTemperature;
    }

    inline float getCurrentHumidity() {
      return currentHumidity;
    }

    bool updateTemperatureMeasurement() {
      unsigned long msec = millis();
      if (prevTemperatureMeasurement > msec) {
        prevTemperatureMeasurement = 0;
      }
      if (msec - prevTemperatureMeasurement > 100) {
        prevTemperatureMeasurement = msec;
        float t = _getTemperature();
        isTemperatireError = isnan(t);
        if (!isTemperatireError) {
          temperatures[temperatureIndex++] = t;
          if (temperatureIndex == sizeof(temperatures) / sizeof(temperatures[0])) {
            float sum = 0;
            for (int i = 0; i < temperatureIndex; i++) {
              sum += temperatures[i];
            }
            currentTemperature = sum / temperatureIndex;
            temperatureIndex = 0;
            return true;
          }
        }
      }
      return false;
    }

    bool updateHumidityMeasurement() {
      unsigned long msec = millis();
      if (prevHumidityMeasurement > msec) {
        prevHumidityMeasurement = 0;
      }
      if (msec - prevHumidityMeasurement > 300) {
        prevHumidityMeasurement = msec;
        float h = _getHumidity();
        isHumidityError = isnan(h);
        if (!isHumidityError) {
          humidities[humidityIndex++] = h;
          if (humidityIndex == sizeof(humidities) / sizeof(humidities[0])) {
            int sum = 0;
            for (int i = 0; i < humidityIndex; i++) {
              sum += humidities[i];
            }
            currentHumidity = sum / humidityIndex;
            humidityIndex = 0;
            return true;
          }
        }
      }
      return false;
    }

    inline bool isError() {
      return isTemperatireError | isHumidityError;
    }

  private:
    float _getTemperature() {
      return sht31.readTemperature();
    }

    float _getHumidity() {
      return sht31.readHumidity();
    }
};

#endif
