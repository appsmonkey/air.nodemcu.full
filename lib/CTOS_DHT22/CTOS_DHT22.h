#ifndef CTOS_DHT22_H
#define CTOS_DHT22_H

#include <CityOS.h>

class CTOS_DHT22: public CityOS {
public:
    CTOS_DHT22(int cpin);

    void setup();
    void loop();
private:
    int pin;
};

#endif /* ifndef CTOS_DHT22_H */
