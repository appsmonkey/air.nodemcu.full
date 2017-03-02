#include <DHT_SIM.h>

DHT_SIM::DHT_SIM()
{
    in.temperature      = 0;
    in.humidity         = 0;
    in.temperature_feel = 0;

    sense("air temperature");
    sense("air humidity");
    sense("air temperature_feel");

    addToInterval(this);
}

void DHT_SIM::interval()
{
    // Read temperature as Celsius (the default)
    in.temperature = -10 + (rand() % (int) (30 + 10 + 1));

    // Sensor readings may be up to 2 seconds 'old' (slow sensor)
    in.humidity = 0 + (rand() % (int) (80 + 1));

    in.temperature_feel = in.temperature + (-3 + (rand() % 7));

    setSense("air temperature", in.temperature);
    setSense("air humidity", in.humidity);
    setSense("air temperature_feel", in.temperature_feel);
}
