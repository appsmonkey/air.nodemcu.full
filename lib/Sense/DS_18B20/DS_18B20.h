#ifndef DS_18B20_H
#define DS_18B20_H

#include "CityOS.h"
#include "OneWire.h"

class DS_18B20 : public CityOS {
public:
    DS_18B20(int pin);
    ~DS_18B20();

    bool info;

    float temperature;

    int pin; // DHT Sensor

    void interval();
private:
    OneWire * _ds;
};
#endif /* ifndef DS_18B22_H */
