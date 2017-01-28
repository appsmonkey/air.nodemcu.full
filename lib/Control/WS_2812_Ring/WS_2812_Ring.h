#ifndef WS_2812_RING_H
#define WS_2812_RING_H

#include "CityOS.h"
#include <Adafruit_NeoPixel.h>
#include <NeoPatterns.h>

class WS_2812_Ring: public CityOS {
public:

    WS_2812_Ring(int pin, int count);

    bool debug = false;
    String listen;

    int pin;

    void setup();
    void loop();
    void looper();

private:
    NeoPatterns ring;
};

#endif /* ifndef WS_2812_RING_H */
