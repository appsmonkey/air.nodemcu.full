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

    Serial << "Value: " << value << " - calculated to " << (1023 - value) << " - Moisture: " << moisture << endl;
    //             << "ERROR| Failed to read from DHT sensor (humidity & temperature set to 0)!"
    // Check if any reads failed and exit early (will try again).
    if (isnan(value)) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from YL moisture sensor!" << endl;
        return;
    }

    if (value < 100) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from YL moisture sensor - YL 39 probably not connected!" << endl;
        return;
    }

    moisture = map(value, 940, 553, 0, 100);
    setSense("soil moisture", moisture);
}
