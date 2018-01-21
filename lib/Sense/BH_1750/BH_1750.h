#ifndef BH_1750_H
#define BH_1750_H

#include "CityOS.h"
#include "Wire.h"
#include "BH1750.h"

class BH_1750 : public CityOS {
public:
    BH_1750(int scl, int sda);
    ~BH_1750();

    float lux;

    void interval();
private:

    int pin; // Sensor pin

    BH1750 * _lightMeter;
};
#endif /* ifndef BH_1750_H */
