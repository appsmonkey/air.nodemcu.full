#ifndef RELAY_H
#define RELAY_H

#include "CityOS.h"
class Relay : public CityOS {
protected:
    int pin;
    bool state;
    bool normallyOpen;

public:
    Relay(int p, bool isNormallyOpen);
    bool getState();
    void on();
    void off();
};

#endif /* ifndef RELAY_H */
