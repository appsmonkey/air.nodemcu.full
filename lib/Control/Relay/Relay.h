#ifndef RELAY_H
#define RELAY_H

#include "CityOS.h"
class Relay : public CityOS {
protected:
    int pin;
    bool state;
    bool normallyOpen;
    static int count;
    String name;

public:

    Relay(int p, bool isNormallyOpen);
    Relay(int p, bool isNormallyOpen, String _name);
    bool getState();
    void on();
    void off();
    void interval();
};

#endif /* ifndef RELAY_H */
