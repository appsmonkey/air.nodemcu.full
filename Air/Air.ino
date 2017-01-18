#include "Air.h"
#include "CTOS_NodeMCU.h"
#include "CTOS_DHT22.h"
#include "CTOS_PMS1003.h"

CTOS_DHT22 dht(D5);
CTOS_PMS1003 pm(D7, D8);

Air air;

// runs once
void setup()
{
    air.debug.errors = true;
    // air.debug.readings = true;
    // air.debug.led      = true;
    // air.debug.wifi = true;
    // air.debug.api       = true;
    air.debug.schema = true;
    air.debug.json   = true;
    // air.debug.webserver = true;
    // air.debug.memory = true;

    // air.setup();
    // dht.setup();
    // pm.setup();
}

// runs over and over again
void loop()
{
    // air.loop();
    // dht.loop();
    // pm.loop();
}
