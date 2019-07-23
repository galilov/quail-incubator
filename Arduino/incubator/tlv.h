#ifndef __TLV_H__
#define __TLV_H__

// Alexander Galilov, alexander.galilov@gmail.com

#include <Arduino.h>

enum TLVType {
  Reset = 0, // reset arduino input.
  Pause,
  Start,
  Continue,
  GetCurrentState, //  Host asks the array of {Int16(temperature in 1/10 C), Int16(humidity, percents), Char(Tray position: L,R or N)
};


class TLV {
    static const size_t _maxDataLen = 32;
  private:
    enum TLVState {ReadType, ReadLength, ReadData};
  public:
    TLVType  type;
    byte  len;
    byte  value[_maxDataLen];

    TLV() : _step(ReadType), _n(0), _offset(0), len(0), type(Reset) {
      // empty
    }

    ~TLV() {
      // empty
    }

    void reset() {
      len = 0;
      type = Reset;
      _n = 0;
      _offset = 0;
      _step = ReadType;
    }

    void setInt16Value(int val) {
      len += 2;
      value[_offset++] = (byte)(val & 0x0ff);
      value[_offset++] = (byte)((val >> 8) & 0x0ff);
    }

    void setInt32Value(int val) {
      len += 4;
      value[_offset++] = (byte)(val & 0x0ff);
      value[_offset++] = (byte)((val >> 8) & 0x0ff);
      value[_offset++] = (byte)((val >> 16) & 0x0ff);
      value[_offset++] = (byte)((val >> 24) & 0x0ff);
    }

    void setByteValue(byte val) {
      len += 1;
      value[_offset++] = val;
    }

    int getIntValue() {
      int result = (value[_offset + 1] << 8) | value[_offset];
      _offset += 2;
      return result;
    }

    byte getByteValue() {
      return value[_offset++];
    }

    void write(Stream& stream) {
      stream.write((byte)type);
      stream.write(len);
      stream.write(value, len);
    }

    bool readByte(Stream& stream) {
      if (!stream.available()) {
        return false;
      }
      _offset = 0;
      int b = stream.read();
      switch (_step) {
        case ReadType:
          type = (TLVType)b;
          if (type == Reset) {
            return true;
          }
          _step = ReadLength;
          break;
        case ReadLength:
          len = (byte) b;
          _n = len;
          if (len == 0) {
            _step = ReadType;
            return true;
          }
          _step = ReadData;
          break;
        case ReadData:
          value[len - (_n--)] = (byte) b;
          if (_n == 0) {
            _step = ReadType;
            return true;
          }
          _step = ReadData;
          break;
      }
      return false;
    }
  private:
    int _n, _offset;
    TLVState _step;

};
#endif
