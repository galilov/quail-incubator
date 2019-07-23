#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>
#include "LedControl.h"

class Led {
  private:
    LedControl lc;
  public:
    /*
      Now we need a LedControl to work with.
     ***** These pin numbers will probably not work with your hardware *****
      pin 12 is connected to the DataIn
      pin 10 is connected to the CLK
      pin 11 is connected to LOAD
      We have 2 MAX72XX.
    */

    Led(): lc(12, 10, 11, 2) {
      //
    }

    void init() {
      lc.shutdown(0, false);
      lc.shutdown(1, false);
      /* Set the brightness to a medium values */
      lc.setIntensity(0, 1);
      lc.setIntensity(1, 1);
      /* and clear the display */
      lc.clearDisplay(0);
      lc.clearDisplay(1);
    }

    inline void clearDisplay(int addr) {
      lc.clearDisplay(addr);
    }

    void print(int addr, const char* data) {
      int len = strlen(data);
      int nPoints = 0;
      for (int i = 0; i < len; i++) {
        if (data[i] == '.') nPoints++;
      }

      char prev;
      int offset = 0, targetPos;
      for (int i = len - 1; i >= 0; --i) {
        char c = data[len - 1 - i];
        bool dp;
        if (c == '.') {
          dp = true;
          offset++;
        } else {
          dp = false;
        }
        targetPos = i + offset - nPoints;
        setChar(addr, targetPos, dp ? prev : c, dp);
        prev = c;
      }
      for (int j = 7; j >= len - nPoints; --j) {
        setChar(addr, j, ' ', false);
      }
    }

    void setChar(int addr, int digit, char c, bool dp) {
      if (c == '@') {
        // print out "degree" symbol
        lc.setRow(addr, digit, 99);
      } else if (c == 'R' || c == 'r') {
        lc.setRow(addr, digit, 0b00000101);
      } else if (c == 'O') {
        lc.setChar(addr, digit, 0, dp);
      } else if (c == 'C') {
        lc.setRow(addr, digit, 0b01001110);
      } else if (c == 'o') {
        lc.setRow(addr, digit, 0b00011101);
      } else {
        lc.setChar(addr, digit, c, dp);
      }
    }



};

#endif
