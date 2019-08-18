namespace Incubator
{
    enum TlvType
    {
        Reset = 0, // reset arduino input.
        Pause,
        Start,
        Continue,
        GetCurrentState, //  Host asks the array of {Int16(temperature in 1/10 C), Int16(humidity, percents), Char(Tray position: L,R or N)
    }

}
