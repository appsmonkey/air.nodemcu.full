#include <DHT_SIM.h>

DHT_SIM::DHT_SIM()
{
    in.temperature      = 0;
    in.humidity         = 0;
    in.temperature_feel = 0;

    sense(config["AIR_TEMPERATURE"]);
    sense(config["AIR_HUMIDITY"]);
    sense(AIR_TEMPERATURE_FEEL);

    addToInterval(this);
}

void DHT_SIM::interval()
{
    // Read temperature as Celsius (the default)
    in.temperature = -10 + (rand() % (int) (30 + 10 + 1));

    // Sensor readings may be up to 2 seconds 'old' (slow sensor)
    in.humidity = 0 + (rand() % (int) (80 + 1));

    in.temperature_feel = in.temperature + (-3 + (rand() % 7));

    setSense(config("AIR_TEMPERATURE"), in.temperature);
    setSense(config["AIR_HUMIDITY"], in.humidity);
    setSense(config["AIR_TEMPERATURE_FEEL"], in.temperature_feel);
}
