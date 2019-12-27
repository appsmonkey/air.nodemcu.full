#include <DHT_22.h>

DHT_22::DHT_22(int pin = D1)
{
    _dht = new DHT(pin, DHT22);
    _dht->begin();
    in.temperature      = 0;
    in.humidity         = 0;
    in.temperature_feel = 0;
    
    sense("AIR_TEMPERATURE");
    sense("AIR_HUMIDITY");
    sense("AIR_TEMPERATURE_FEEL");

    addToInterval(this);
}

DHT_22::~DHT_22()
{
    delete _dht;
}

void DHT_22::interval()
{
    // Read temperature as Celsius (the default)
    in.temperature = _dht->readTemperature();

    // Sensor readings may be up to 2 seconds 'old' (slow sensor)
    in.humidity = _dht->readHumidity();

    // Check if any reads failed and exit early (will try again).
    if (isnan(in.humidity) || isnan(in.temperature)) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from DHT sensor (humidity & temperature set to 0)!" << endl;
        return;
    } else {
        // Computes temperature values in Celsius and Humidity
        in.temperature_feel = _dht->computeHeatIndex(in.temperature, in.humidity, false);
    }
    setSense("AIR_TEMPERATURE", in.temperature);
    setSense("AIR_HUMIDITY", in.humidity);
    setSense("AIR_TEMPERATURE_FEEL", in.temperature_feel);
}
