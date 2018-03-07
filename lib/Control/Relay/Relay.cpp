#include "Relay.h"

Relay::Relay(int p, bool isNormallyOpen)
{
    pin = p;
    normallyOpen = isNormallyOpen;
    pinMode(pin, OUTPUT);
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
