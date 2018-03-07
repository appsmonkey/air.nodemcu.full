#include "Relay.h"

int Relay::count = 0;

Relay::Relay(int p, bool isNormallyOpen)
{
    pin = p;
    normallyOpen = isNormallyOpen;
    pinMode(pin, OUTPUT);
    count++;

    name = "relay ";
    name.concat(count);
    sense(name);

    addToInterval(this);
}

Relay::Relay(int p, bool isNormallyOpen, String _name)
{
    pin = p;
    normallyOpen = isNormallyOpen;
    pinMode(pin, OUTPUT);
    count++;

    name = _name;
    sense(name);

    addToInterval(this);
}

bool Relay::getState()
{
    if (normallyOpen) {
        return !state;
    } else {
        return state;
    }
}

void Relay::on()
{
    if (normallyOpen) {
        if (state == !true) return;
        state = !true;
    } else {
        if (state == true) return;
        state = true;
    }
    digitalWrite(pin, state);
}

void Relay::off()
{
    if (normallyOpen) {
        if (state == !false) return;
        state = !false;
    } else {
        if (state == false) return;
        state = false;
    }
    digitalWrite(pin, state);
}

void Relay::interval()
{
    if (getState())
        setSense(name, 1);
    else
        setSense(name, 0);
}
