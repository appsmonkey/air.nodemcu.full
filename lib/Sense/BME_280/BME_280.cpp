#include <BME_280.h>
//
#define SEALEVELPRESSURE_HPA (1013.25)
//

BME_280::BME_280(int scl, int sda, int i2c)
{
    setup(scl, sda, i2c);
};

BME_280::BME_280(int scl, int sda)
{
    setup(scl, sda, 0x76);
};

void BME_280::setup(int scl = D3, int sda = D4, int i2c = 0x76)
{
    _bme  = new Adafruit_BME280();
    _wire = new TwoWire();
    _wire->begin(sda, scl);
    _bme->begin(i2c, _wire);

    in.temperature      = 0;
    in.humidity         = 0;
    in.temperature_feel = 0;
    in.pressure         = 0;
    in.altitude         = 0;

    sense("air temperature");
    sense("air humidity");
    sense("air temperature_feel");
    // sense("air pressure");
    // sense("air altitude");

    addToInterval(this);
}

BME_280::~BME_280()
{
    delete _bme;
}

void BME_280::interval()
{
    in.temperature = _bme->readTemperature();
    // float tempF = 9.0 / 5.0 * tempC + 32.0;

    in.humidity = _bme->readHumidity();

    // Check if any reads failed and exit early (will try again).
    if (isnan(in.humidity) || isnan(in.temperature)) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from BME sensor (humidity & temperature set to 0)!" << endl;
        return;
    } else {
        // Computes temperature values in Celsius and Humidity
        in.temperature_feel = computeHeatIndex(in.temperature, in.humidity, false);
    }

    in.pressure = _bme->readPressure();
    // float pressureHectoPascals    = pressurePascals / 100.0;
    // float pressureInchesOfMercury = 0.000295299830714 * pressurePascals;

    // // Approximate altitude
    in.altitude = _bme->readAltitude(SEA_LEVEL_PRESSURE_HPA);
    // Feet = 3.28 * altitudeMeters;

    setSense("air temperature", in.temperature);
    setSense("air humidity", in.humidity);
    setSense("air temperature_feel", in.temperature_feel);
    // setSense("air pressure", in.pressure);
    // setSense("air altitude", in.altitude);
} // BME_280::interval

// boolean isFahrenheit: True == Fahrenheit; False == Celcius
float BME_280::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit)
{
    // Using both Rothfusz and Steadman's equations
    // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
    float hi;

    if (!isFahrenheit)
        temperature = convertCtoF(temperature);

    hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

    if (hi > 79) {
        hi = -42.379
          + 2.04901523 * temperature
          + 10.14333127 * percentHumidity
          + -0.22475541 * temperature * percentHumidity
          + -0.00683783 * pow(temperature, 2)
          + -0.05481717 * pow(percentHumidity, 2)
          + 0.00122874 * pow(temperature, 2) * percentHumidity
          + 0.00085282 * temperature * pow(percentHumidity, 2)
          + -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

        if ((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
            hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

        else if ((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
            hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
    }

    return isFahrenheit ? hi : convertFtoC(hi);
}

float BME_280::convertCtoF(float c)
{
    return c * 1.8 + 32;
}

float BME_280::convertFtoC(float f)
{
    return (f - 32) * 0.55555;
}
