// Device.h contains configuration of device
#include <Device.h>

// setup runs once
void setup()
{
    // ctos.sensing.interval = 5;
    // led.listen = "air aqi range";
    // led.debug  = true;

    // ctos.debug.memory   = true;
    // ctos.debug.wifi     = true;
    ctos.debug.api      = true;
    ctos.debug.senses   = true;
    ctos.debug.controls = true;
    // ctos.debug.schema   = true;
}

// runs over and over again
void loop()
{
    ctos.loop();
}
