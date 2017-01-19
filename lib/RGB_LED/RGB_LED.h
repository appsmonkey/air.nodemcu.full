#ifndef RGB_LED_CTOS_H
#define RGB_LED_CTOS_H

#include <CityOS.h>

class RGB_LED: public CityOS {
public:
    bool debug = false;

    struct _PIN {
        int red;   // Led PIN
        int green; // Led PIN
        int blue;  // Led PIN
    } pin;

    struct _OUT {
        // PM Sensor
        int range;
    } our;

    RGB_LED(int red, int green, int blue);

    void setup();
    void loop();
};

#endif /* ifndef RGB_LED_CTOS_H */
