#include <Wire.h>
#include <Servo.h>
#include "sht31.h"
#include "led.h"
#include "srvo.h"
#include "heater.h"
#include "pid.h"
#include "schedule.h"
#include "tlv.h"
#include "cover.h"


const int humidityWindow = 1; // +/- % of target humidity


Sht31 sht31;
Srvo srvo;
Heater heater;
PIDRegulator pid;
Led led;
Schedule schedule;
TLV current, fromHost;
WaterCover waterCover;

bool dataFromHostIsReady;
bool isSht31Error;
bool paused;
//int coverState = LOW;

float targetTemperature;
float currentTemperature;
int targetHumidity;
int currentHumidity;
char targetTrayPosition;
char currentTrayPosition = 'N';

int power;

void sendCurrentState() {
  current.reset();
  current.type = GetCurrentState;
  current.setInt16Value((int)(currentTemperature * 100));
  current.setInt16Value((int)(targetTemperature * 100));
  current.setInt16Value(currentHumidity);
  current.setInt16Value(targetHumidity);
  current.setByteValue(currentTrayPosition);
  current.setInt16Value(power);
  current.setByteValue(waterCover.isOpened() ? 'O' : 'C');
  current.setByteValue(isSht31Error ? 'E' : '-');
  current.setByteValue(paused ? 'P' : '-');
  current.setInt16Value(schedule.getHourOfIncubation());
  current.write(Serial);
}

void initState();

void processDataFromHost() {
  switch (fromHost.type) {
    case Pause:
      paused = true;
      break;
    case Continue:
      paused = false;
      break;
    case GetCurrentState:
      sendCurrentState();
      break;
    case Start:
      schedule.setStartDateToNow();
      break;
    case Reset:
      init();
      break;
  }

}



void turnTrayToTarget() {
  if (srvo.updatePositionToTarget(targetTrayPosition)) {
    currentTrayPosition = targetTrayPosition;
  }
}

unsigned long previousLedUpdated = 0;

void updateLed() {
  unsigned long msec = millis();
  if (previousLedUpdated > msec) {
    previousLedUpdated = 0;
  }
  if (msec - previousLedUpdated > 1000) {
    previousLedUpdated = msec;
    // do LED update here
    static char ledInfo0[16];
    static char ledInfo1[16];
    static char t[8], h[8];
    if (isSht31Error) {
      led.print(0, "Error ");
    } else {
      itoa(currentHumidity, h, 10);
      dtostrf(currentTemperature, 4, 1, t);
      sprintf(ledInfo0, "%sh %s@", h, t);
      led.print(0, ledInfo0);
    }
    sprintf(ledInfo1, "%d.%s.%d", schedule.getHourOfIncubation(), waterCover.isOpened() ? "OP" : "CL", power);
    led.print(1, ledInfo1);
  }

}


unsigned long previousScheduleUpdated = 0;

void updateSchedule() {
  unsigned long msec = millis();
  if (previousScheduleUpdated > msec) {
    previousScheduleUpdated = 0;
  }
  if (msec - previousScheduleUpdated > 1000) {
    previousScheduleUpdated = msec;
    schedule.update();
  }
}

void initState() {
  targetTemperature = 38.5;
  currentTemperature = -100;
  targetHumidity = 50;
  targetTrayPosition = 'N';

  int power = 0;

  paused = false;
  isSht31Error = false;
  if (!sht31.init()) {
    isSht31Error = true;
  }
  led.init();
  heater.init();
  srvo.init();
  schedule.init();
  pid.init();
}


void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(WATER_COVER_PIN, OUTPUT);
  initState();
}


void loop() {

  targetTrayPosition = schedule.getTargetTrayPosition();
  targetTemperature = schedule.getTargetTemperature();
  targetHumidity = schedule.getTargetHumidity();
  isSht31Error = sht31.isError();

  if (sht31.updateTemperatureMeasurement()) {
    currentTemperature = sht31.getCurrentTemperature();
  }
  if (currentTemperature != -1) {
    double error = targetTemperature - currentTemperature;
    if (pid.update(error, currentTemperature)) {

      double output = pid.getOutput();

      if (output < 0) power = 0;
      else if (output > 100) power = 100;
      else power = (int) output;
    }
  }
  if (sht31.updateHumidityMeasurement()) {
    currentHumidity = sht31.getCurrentHumidity();
  }
  if (paused) {
    targetTrayPosition = 'N';
  }
  if (currentTrayPosition != targetTrayPosition) {
    turnTrayToTarget();
  }
  if (isSht31Error) {
    waterCover.setOpened();
  }
  else
  {
    if (waterCover.isClosed()) {
      if (currentHumidity <= targetHumidity - humidityWindow) {
        waterCover.setOpened();
      }
    } else if (currentHumidity > targetHumidity + humidityWindow) {
      waterCover.setClosed();
    }
  }
  if (isSht31Error || paused) power = 0;
  heater.update(power);

  if (dataFromHostIsReady) {
    dataFromHostIsReady = false;
    processDataFromHost();
  }
  updateSchedule();
  updateLed();
}

void serialEvent() {
  while (Serial.available() && !dataFromHostIsReady) {
    // get the new byte:
    dataFromHostIsReady = fromHost.readByte(Serial);
  }
}
