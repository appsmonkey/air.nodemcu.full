#include <YL_39_69.h>

YL_39_69::YL_39_69(int power)
{
    setup(power, A0);
}

YL_39_69::YL_39_69(int power, int read)
{
    setup(power, read);
}

void YL_39_69::setup(int power, int read)
{
    moisture = 0;

    pin.power = power;
    pin.read  = read;

    // Init the humidity sensor board
    pinMode(pin.power, OUTPUT);
    digitalWrite(pin.power, LOW);

    sense("soil moisture");

    addToInterval(this);
}

void YL_39_69::interval()
{
    // Read moisture in percentages - turn it on with digital pin
    digitalWrite(pin.power, HIGH);
    delay(500);
    // Read moisture in percentages
    int value = analogRead(pin.read);
    // Turn it off with digital pin
    digitalWrite(pin.power, LOW);

    if (isnan(value)) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from YL moisture sensor!" << endl;
        return;
    }

    moisture = map(value, top, bottom, 0, 100); // These values need to be finetuned
    // moisture = map(value, 1024, 0, 0, 100); // 1024 and 0 are very unlikely

    if (info) {
        Serial
            << "Value (1024 - 0): " << value << endl
            << "Reversed (0 - 1024): " << (1024 - value) << endl
            << "Moisture: " << moisture << endl;
    }


    if (value < 0 || moisture > 100) {
        if (debug.errors) {
            Serial
                << "ERROR| Value on YL 39/69 too big" << endl
                << "moisture: " << moisture << endl
                << "reseting to 100";
        }
        moisture = 100;
    }

    if (value > 1024 || moisture < 0) {
        if (debug.errors) {
            Serial
                << "ERROR| Value on YL 39/69 negative" << endl
                << "moisture: " << moisture << endl
                << "reseting to 0";
        }
        moisture = 0;
    }

    setSense("soil moisture", moisture);
} // YL_39_69::interval
