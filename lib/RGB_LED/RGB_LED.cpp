#include <RGB_LED.h>

RGB_LED::RGB_LED(int red, int green, int blue)
{
    pin.red   = red;
    pin.green = green;
    pin.blue  = blue;

    pinMode(pin.red, OUTPUT);
    pinMode(pin.green, OUTPUT);
    pinMode(pin.blue, OUTPUT);

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

    // no need to update if same as last
    // if (range.worst == last_range)
    // return;

    int colors[6][3] = {
        {    1, 200, 255 }, // blue
        {  175, 255,   1 }, // green
        {  245,  90,   1 }, // yellow
        {  255,  40,   1 }, // orange
        {  255,   1, 170 }, // purple
        {  255,   1,   1 }  // red
    };

    if (inputValues["air|aqi"] >= sizeof(colors))
        inputValues["air|aqi"] = sizeof(colors) - 1;

    int current_range = inputValues["air|aqi"];
    int red   = map(colors[current_range][0], 0, 255, 0, 1023);
    int green = map(colors[current_range][1], 0, 255, 0, 1023);
    int blue  = map(colors[current_range][2], 0, 255, 0, 1023);

    if (debug.led) {
        Serial
            << "LED INFO: Printing for range: " << current_range << endl
            << "red: " << red << "[" << colors[current_range][0] << "]"
            << " green: " << green << "[" << colors[current_range][1] << "]"
            << " blue: " << blue << "[" << colors[current_range][2] << "]" << endl;
    }

    analogWrite(pin.red, red);
    analogWrite(pin.green, green);
    analogWrite(pin.blue, blue);

    last_range = current_range;
} // RGB_LED::loop
