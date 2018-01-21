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

    int pin; // DS_18B20 Sensor


    void interval();
private:
    OneWire * _ds;

    // indicates which chip is used DS1820, DS18S20, DS18B20 or DS1822
    byte type_s;

    byte addr[8];
    byte i;
    byte present = 0;

    byte data[12];
};
#endif /* ifndef DS_18B22_H */
