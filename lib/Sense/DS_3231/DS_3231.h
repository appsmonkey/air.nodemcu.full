#ifndef FloatSwitch_H
#define FloatSwitch_H

#include "CityOS.h"
#include "FloatSwitch.h"

class FloatSwitch : public CityOS {
public:
    FloatSwitch(int pin);

    int on;

    void interval();
private:

    int pin; // Sensor pin
};
#endif /* ifndef FloatSwitch_H */
