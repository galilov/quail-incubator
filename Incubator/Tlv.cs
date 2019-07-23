using System;
using System.IO;

namespace Incubator
{
    class Tlv
    {
        private enum TlvState { ReadType, ReadLength, ReadData };
        private TlvState _step = TlvState.ReadType;
        private int _n, _offset;

        public const int MaxDataLen = 32;
        public TlvType Type { get; set; }
        private int _len;

        public int Len
        {
            get => _len;
            set => _len = value > MaxDataLen ? MaxDataLen : value;
        }

        public readonly byte[] ValueBytes = new byte[MaxDataLen];

        public void Reset()
        {
            Type = TlvType.Reset;
            _n = 0;
            _offset = 0;
            _step = TlvState.ReadType;
            _len = 0;
        }

        public void SetInt16Value(short val)
        {
            ValueBytes[_offset++] = (byte)(val & 0x0ff);
            ValueBytes[_offset++] = (byte)((val >> 8) & 0x0ff);
            Len += 2;
        }

        public void SetInt8Value(byte val)
        {
            ValueBytes[_offset++] = val;
            Len += 1;
        }

        public short GetInt16Value()
        {
            short result = ValueBytes[_offset++];
            result |= (short)(ValueBytes[_offset++] << 8);
            return result;
        }
        public int GetInt32Value()
        {
            int result = ValueBytes[_offset++];
            result |= ValueBytes[_offset++] << 8;
            result |= ValueBytes[_offset++] << 16;
            result |= ValueBytes[_offset++] << 24;
            return result;
        }

        public byte GetInt8Value()
        {
            return ValueBytes[_offset++];
        }

        public void Write(Stream to)
        {
            to.WriteByte((byte)Type);
            to.WriteByte((byte)Len);
            to.Write(ValueBytes, 0, Len);
        }

        public bool ReadByte(int b)
        {
            Console.WriteLine(b);

            if (b == -1) return false;
            switch (_step)
            {
                case TlvState.ReadType:
                    Type = (TlvType)b;
                    if (Type == TlvType.Reset)
                    {
                        return true;
                    }

                    _step = TlvState.ReadLength;
                    break;
                case TlvState.ReadLength:
                    Len = b;
                    _n = b;
                    if (Len == 0)
                    {
                        _step = TlvState.ReadType;
                        return true;
                    }
                    _step = TlvState.ReadData;
                    break;
                case TlvState.ReadData:
                    var idx = Len - _n--;
                    ValueBytes[idx] = (byte)b;
                    if (_n == 0)
                    {
                        _step = TlvState.ReadType;
                        _offset = 0;
                        return true;
                    }

                    _step = TlvState.ReadData;
                    break;
            }

            return false;
        }
    }
}
