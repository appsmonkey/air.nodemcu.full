#ifndef DHT_22_H
#define DHT_22_H

#include "CityOS.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"

class DHT_22: public CityOS {
public:
    DHT_22(int pin);
    ~DHT_22();

    struct _in {
        // DHT_22 Sensor
        float temperature;
        float humidity;
        float temperature_feel;
    } in;

    int pin; // DHT Sensor

    void interval();
private:
    DHT * _dht;
};
#endif /* ifndef DHT_22_H */
