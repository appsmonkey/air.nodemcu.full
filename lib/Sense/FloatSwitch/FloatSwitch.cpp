#include "FloatSwitch.h"

FloatSwitch::FloatSwitch(int fs_pin)
{
    pin = fs_pin;
    on  = 0;

    pinMode(pin, INPUT_PULLUP);

    sense(config["WATER_LEVEL_SWITCH"]);

    addToInterval(this);
}

void FloatSwitch::interval()
{
    if (digitalRead(pin) == HIGH)
        on = 0;
    else
        on = 1;

    setSense(config["WATER_LEVEL_SWITCH"], on);

}
