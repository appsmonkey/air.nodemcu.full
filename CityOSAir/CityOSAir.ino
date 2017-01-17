#include "Air.h"

Air air;

// runs once
void setup()
{
    air.debug.errors = true;
    // air.debug.readings = true;
    // air.debug.led      = true;
    // air.debug.wifi = true;
    // air.debug.api       = true;
    air.debug.json = true;
    // air.debug.webserver = true;
    // air.debug.memory = true;

    air.setup();
}

// runs over and over again
void loop()
{
    air.loop();
}
