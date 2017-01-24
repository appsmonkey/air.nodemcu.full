// Device.h contains configuration of device
#include <Device.h>

// setup runs once
void setup()
{
    led.listen = "air aqi range";
    // led.debug = true;

    ctos.debug.api     = true;
    ctos.debug.inputs  = true;
    ctos.debug.outputs = true;

    // Initialize framework setups
    ctos.setup();
}

// runs over and over again
void loop()
{
    // Initialize framework loops
    ctos.loop();
}
