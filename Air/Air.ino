// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

// Add inputs (sensors) header files
#include "DHT22_CityOS.h"
#include "PMS1003.h"

// Add output
#include "RGB_LED.h"

// Accesss to all data
CityOS ctos;

// Initialize PINS on all inputs and outputs
DHT22_CityOS dht(D1);
PMS1003 pm(D7, D8);
RGB_LED led(D2, D5, D6);

// setup runs once
void setup()
{
    ctos.debug.errors = true;
    // air.debug.readings = true;
    // air.debug.led      = true;
    // ctos.debug.wifi = true;
    // air.debug.api       = true;
    // ctos.debug.schema = true;
    // ctos.debug.json   = true;
    // air.debug.webserver = true;
    // air.debug.memory = true;
    ctos.debug.inputs = true;

    // Initialize framework setups
    ctos.setup();
}

// runs over and over again
void loop()
{
    // Initialize framework loops
    ctos.loop();
}
