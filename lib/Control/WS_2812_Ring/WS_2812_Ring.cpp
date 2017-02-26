#include <WS_2812_Ring.h>

WS_2812_Ring::WS_2812_Ring(int pin, int count) : ring(count, pin)
{
    control("led ring - 6 levels");
    listen = "air aqi range";
    addToLoop(this);

    ring.begin();
    // Loop rainbow until first reading
    ring.RainbowCycle(5);
    for (int i = 0; i < 2000; i++) {
        ring.RainbowCycleUpdate();
        delay(100);
    }
};

void WS_2812_Ring::loop()
{
    static int intensity      = 255;
    static int last_intensity = -1;
    static int last_range     = -1;
    static int breath         = 0;
    float brightness = 255;
    static int step  = 1;

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

    // int current_range = 5;
    int current_range = (int) senseValues[listen];

    // When clear breathing deeper
    int depth = (14 - current_range) * 10;

    // When clear breathing slower
    int speed = (5 - current_range) * 10000;

    if (breath > speed || breath < 0)
        step = -step;

    breath += step;

    // if (current_range > 1)
    intensity = (int) round(map(breath, 1, speed, depth, 255));
    // else
    //  intensity = 200;

    // no need to update if same as last
    if (false && last_intensity == intensity && current_range == last_range) {
        if (debug) {
            Serial
                << "LED RING | Range " << listen
                << "same as last time - nothing to do" << endl;
        }
        return;
    }

    last_intensity = intensity;

    int colors[6][3] = {
        {    1, 200, 255 }, // blue
        {   60, 255,   1 }, // green
        {  255, 120,   1 }, // yellow
        {  255,  40,   1 }, // orange
        {  255,   1, 170 }, // purple
        {  255,   1,   1 }  // red
    };

    if (current_range >= sizeof(colors))
        current_range = sizeof(colors) - 1;

    uint32_t color =
      ring.Color(colors[current_range][0], colors[current_range][1], colors[current_range][2], 255);

    ring.ColorSet(color);
    ring.show();
} // WS_2812_Ring::loop
