#ifndef DHT_SIM_H
#define DHT_SIM_H

#include "CityOS.h"

class DHT_SIM : public CityOS {
public:
    DHT_SIM();

    struct _in {
        // DHT_22 Sensor
        float temperature;
        float humidity;
        float temperature_feel;
    } in;

    void interval();
};
#endif /* ifndef DHT_SIM_H */
