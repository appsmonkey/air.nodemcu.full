#include <DHT22_CityOS.h>

DHT22_CityOS::DHT22_CityOS(int cpin = D1)
{
    pin.dht = cpin;
    _dht    = new DHT(pin.dht, DHT22);

    input("air|temperature");
    input("air|humidity");
    input("air|temperature_feel");
    sense(this);
}

DHT22_CityOS::~DHT22_CityOS()
{
    delete _dht;
}

void DHT22_CityOS::setup()
{
    _dht->begin();
}

void DHT22_CityOS::loop()
{
    // Read temperature as Celsius (the default)
    in.temperature = _dht->readTemperature();

    // Sensor readings may be up to 2 seconds 'old' (slow sensor)
    in.humidity = _dht->readHumidity();

    // Check if any reads failed and exit early (will try again).
    if (isnan(in.humidity) || isnan(in.temperature)) {
        in.temperature      = 0;
        in.humidity         = 0;
        in.temperature_feel = 0;
        if (debug.errors) Serial
                << "ERROR| Failed to read from DHT sensor (humidity & temperature set to 0)!" << endl;
    } else {
        // Computes temperature values in Celsius and Humidity
        in.temperature_feel = _dht->computeHeatIndex(in.temperature, in.humidity, false);
        if (debug.readings) {
            Serial << "READINGS| Temperature: " << in.temperature << endl;
            Serial << "READINGS| Humidity: " << in.humidity << endl;
            Serial << "READINGS| Feels Like: " << in.temperature_feel << endl;
        }
        setValue("air|temperature", in.temperature);
        setValue("air|humidity", in.humidity);
        setValue("air|temperature_feel", in.temperature_feel);
    }
}
