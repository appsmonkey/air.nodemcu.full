#ifndef DHT22_H
#define DHT22_H

#include "CityOS.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"

class DHT22_CityOS: public CityOS {
public:
    DHT22_CityOS(int cpin);
    ~DHT22_CityOS();

    struct _in {
        // DHT22 Sensor
        float temperature;
        float humidity;
        float temperature_feel;
    } in;

    struct _PIN {
        int dht; // DHT Sensor
    } pin;

    void setup();
    void loop();
private:
    DHT * _dht;
};

#endif /* ifndef CTOS_DHT22_H */
