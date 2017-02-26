#include <WS_2812_Ring.h>

WS_2812_Ring::WS_2812_Ring(int pin, int count) : ring(count, pin)
{
    control("led ring - 6 levels");
    listen = "air aqi range";
    addToLoop(this);

    ring.begin();
    ring.Interval = 100;
    ring.setBrightness(255);
    // Loop rainbow until first reading
    ring.RainbowCycle(0);
    for (int i = 0; i < 1000; i++) {
        ring.RainbowCycleUpdate();
        delay(300);
    }
};

void WS_2812_Ring::loop()
{
    static int lastUpdate = 0;

    if ((millis() - lastUpdate) < ring.Interval) {
        yield();
        return;
    }

    lastUpdate = millis();

    static int step           = 1;
    static int breath         = 0;
    static int last_intensity = -1;
    static int intensity      = 255;
    static int last_range     = -1;

    if (!listen.length()) {
        if (debug) {
            Serial
                << "LED RING | Listening not set " << endl
                << "LED RING | Use ring.listen = \"range name\" to set it." << endl;
        }
        return;
    }

    if (debug) {
        Serial
            << "LED RING | Listening set to: " << listen << endl;
    }

    int current_range = (int) senseValues[listen];
    // current_range = 0;

    if (current_range > 2) {
        // When clear breathing deeper
        int depth = (8 + current_range) * 10;
        // When clear breathing slower
        int speed = (10 - current_range) * 1000;

        if (breath > speed) {
            breath = speed - 1;
            step   = -step;
        }

        if (breath < 0) {
            breath = 1;
            step   = -step;
        }

        breath += step;

        if (debug) {
            Serial << "intensity: " << intensity << endl;
            // Serial << "speed: " << speed << endl;
            // Serial << "breath: " << breath << endl;
        }

        intensity = (int) round(map(breath, 1, speed, depth, 255));
    } else {
        intensity = 255;
    }

    if (last_intensity != intensity) {
        ring.setBrightness(intensity);
        last_intensity = intensity;
    }

    if (last_range != current_range || last_intensity != intensity) {
        int colors[6][3] = {
            {    1, 200, 255 }, // blue
            {   60, 255,   1 }, // green
            {  255, 120,   1 }, // yellow
            {  255,  40,   1 }, // orange
            {  255,   1, 170 }, // purple
            {  255,   1,   1 } // red
        };

        if (current_range >= sizeof(colors))
            current_range = sizeof(colors) - 1;

        for (int i = 0; i < ring.numPixels(); i++) {
            ring.setPixelColor(i, colors[current_range][0], colors[current_range][1], colors[current_range][2]);
        }
        ring.show();
    }
} // WS_2812_Ring::loop
