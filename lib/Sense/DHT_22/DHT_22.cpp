#include <DHT_22.h>

DHT_22::DHT_22(int cpin = D1)
{
    pin.dht = cpin;
    _dht    = new DHT(pin.dht, DHT22);

    input("air temperature");
    input("air humidity");
    input("air temperature_feel");
    sense(this);
}

DHT_22::~DHT_22()
{
    delete _dht;
}

void DHT_22::setup()
{
    _dht->begin();
    in.temperature      = 0;
    in.humidity         = 0;
    in.temperature_feel = 0;
}

void DHT_22::loop()
{
    // Read temperature as Celsius (the default)
    in.temperature = _dht->readTemperature();

    // Sensor readings may be up to 2 seconds 'old' (slow sensor)
    in.humidity = _dht->readHumidity();

    // Check if any reads failed and exit early (will try again).
    if (isnan(in.humidity) || isnan(in.temperature)) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from DHT sensor (humidity & temperature set to 0)!" << endl;
    } else {
        // Computes temperature values in Celsius and Humidity
        in.temperature_feel = _dht->computeHeatIndex(in.temperature, in.humidity, false);
    }
    setInputValue("air temperature", in.temperature);
    setInputValue("air humidity", in.humidity);
    setInputValue("air temperature_feel", in.temperature_feel);
}
