#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <Arduino.h>
#include <EEPROM.h>
#include "RTClib.h"

struct EEPROMDate {
  uint16_t _year;
  uint8_t _month;
  uint8_t _day;
  uint8_t _hour;
  uint8_t _minute;
  uint8_t _second;

  EEPROMDate(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) :
    _year(year), _month(month), _day(day), _hour(hour), _minute(minute), _second(second)
  {}

  EEPROMDate() :
    _year(0), _month(0), _day(0), _hour(0), _minute(0), _second(0)
  {}

  void save() {
    EEPROM.put(0, *this);
  }

  void load() {
    EEPROM.get(0, *this);
  }
};
//http://www.inkubator-info.ru/pro/inkubaciya-perepelinykh-yaic-rezhim-inkubacii-tablica/
class Schedule {
  private:
    PCF8563 _rtc;
    EEPROMDate _startDate;
    DateTime _start;
    DateTime _now;
    uint32_t _s, _r;
    uint16_t _d;

    void _load() {
      _startDate.load();
      if (_startDate._month == 255) {
        _start = _rtc.now();
      }
      _start.setyear(_startDate._year);
      _start.setmonth(_startDate._month);
      _start.setday(_startDate._day);
      _start.sethour(_startDate._hour);
      _start.setminute(_startDate._minute);
      _start.setsecond(_startDate._second);
    }
  public:

    void init() {
      _rtc.begin();
      //      if (! rtc.isrunning()) {
      //        // following line sets the RTC to the date & time this sketch was compiled
      //        rtc.adjust(DateTime(__DATE__, __TIME__));
      //      }
      _load();
      update();
    }

    void update() {
      _now = _rtc.now();
      _s = _now.unixtime() - _start.unixtime();
      _d = (uint16_t)(_s / 86400L);
      _r = (uint32_t)(_s % 86400L);
    }

    void setStartDate(uint16_t year, uint8_t month, uint8_t day, uint8_t hour) {
      _startDate._year = year;
      _startDate._month = month;
      _startDate._day = day;
      _startDate._hour = hour;
      _startDate._minute = 0;
      _startDate._second = 0;
      _startDate.save();
      _load();
    }

    void setStartDateToNow() {
      DateTime now = _rtc.now();
      setStartDate(now.year(), now.month(), now.day(), now.hour());
    }

    float getTargetTemperature() {
      if (_d < 7) {
        return 37.8;
      }
      if (_d < 14) {
        if (_r % 43200L < 20 * 60L) {
          return 34.0;
        }
        return 37.8;
      }
      return 37.5;
    }

    int getTargetHumidity() {
      if (_d < 7) {
        return 52;
      }
      if (_d < 14) {
        return 45;
      }
      return 67;
    }

    char getTargetTrayPosition() {
      if (_d < 7) {
        if ((_r / 21600L) % 2 == 0) {
          return 'L';
        } else {
          return 'R';
        }
      }
      if (_d < 14) {
        if ((_r / 14400L) % 2 == 0) {
          return 'L';
        } else {
          return 'R';
        }
      }
      return 'N';
    }

    int getHourOfIncubation() {
      return _s / 3600;
    }

};
#endif
