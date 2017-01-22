#include <RGB_LED.h>

RGB_LED::RGB_LED(int red, int green, int blue)
{
    pin.red   = red;
    pin.green = green;
    pin.blue  = blue;

    pinMode(pin.red, OUTPUT);
    pinMode(pin.green, OUTPUT);
    pinMode(pin.blue, OUTPUT);

    listen = "air aqi range";

    sense(this);
}

void RGB_LED::setup()
{
    // Turn it on
    pinMode(pin.red, OUTPUT);
    pinMode(pin.green, OUTPUT);
    pinMode(pin.blue, OUTPUT);
}

void RGB_LED::loop()
{
    static int last_range = -1;

    if (!listen.length()) {
        if (debug) {
            Serial
                << "RGB LED | Listening not set " << endl
                << "RGB LED | Use led.listen = \"range name\" to set it." << endl;
        }
        return;
    }

    if (debug) {
        Serial
            << "RGB LED | Listening set to: " << listen << endl;
    }

    int current_range = (int) outputValues[listen];

    // no need to update if same as last
    if (current_range == last_range) {
        if (debug) {
            Serial
                << "RGB LED | Range " << listen
                << "same as last time - nothing to do" << endl;
        }
        return;
    }

    int colors[6][3] = {
        {    1, 200, 255 }, // blue
        {  175, 255,   1 }, // green
        {  245,  90,   1 }, // yellow
        {  255,  40,   1 }, // orange
        {  255,   1, 170 }, // purple
        {  255,   1,   1 } // red
    };

    if (current_range >= sizeof(colors))
        current_range = sizeof(colors) - 1;

    int red   = map(colors[current_range][0], 0, 255, 0, 1023);
    int green = map(colors[current_range][1], 0, 255, 0, 1023);
    int blue  = map(colors[current_range][2], 0, 255, 0, 1023);

    if (debug) {
        Serial
            << "RGB LED | Printing for range: " << current_range << endl
            << "red: " << red << "[" << colors[current_range][0] << "]"
            << " green: " << green << "[" << colors[current_range][1] << "]"
            << " blue: " << blue << "[" << colors[current_range][2] << "]" << endl;
    }

    analogWrite(pin.red, red);
    analogWrite(pin.green, green);
    analogWrite(pin.blue, blue);

    last_range = current_range;
} // RGB_LED::loop
