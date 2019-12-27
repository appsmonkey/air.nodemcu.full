#include "BH_1750.h"

BH_1750::BH_1750(int scl, int sda)
{
    _lightMeter = new BH1750(0x23);
    Wire.begin(sda, scl);

    // _lightMeter->begin(BH1750::ONE_TIME_HIGH_RES_MODE);
    // if (!_lightMeter->begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    //     if (debug.errors) Serial
    //             << "Error initialising BH1750" << endl;
    //     // return;
    // }
    lux = 0;
    _lightMeter->begin();
    _lightMeter->configure(BH1750::ONE_TIME_HIGH_RES_MODE);

    sense("LIGHT_INTENSITY");

    addToInterval(this);
}

BH_1750::~BH_1750()
{
    delete _lightMeter;
}

void BH_1750::interval()
{
    // Read lux
    lux = _lightMeter->readLightLevel();
    if (lux == 0) {
        delay(1000);
        lux = _lightMeter->readLightLevel();
    }
    setSense("LIGHT_INTENSITY", lux);
}
