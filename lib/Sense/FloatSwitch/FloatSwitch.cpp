#include "FloatSwitch.h"

FloatSwitch::FloatSwitch(int fs_pin)
{
    pin = fs_pin;
    on  = 0;

    pinMode(pin, INPUT_PULLUP);

    sense("water level switch");

    addToInterval(this);
}

void FloatSwitch::interval()
{
    if (digitalRead(pin) == HIGH)
        on = 0;
    else
        on = 1;

    setSense("water level switch", on);
}
