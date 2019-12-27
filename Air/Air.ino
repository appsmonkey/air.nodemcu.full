// Device.h contains configuration of device
#include <Device.h>

// setup runs once
void setup()
{
    // ctos.sensing.interval = 5;
    // ring.listen = "air aqi range";
    // ring.debug  = true;

    ctos.debug.errors = true;
    // ctos.debug.wifi     = true;
    // ctos.debug.api    = true;
    ctos.debug.senses = true;
    // ctos.debug.controls = true;
    // ctos.debug.schema   = true;

    // Be nice and disable API while building hardware to spare server from junk data
    // ctos.sensing.active = false;
}

// runs over and over again
void loop()
{
    ctos.loop();
}
