#include <SCD_30.h>


SCD_30::SCD_30(int scl, int sda, int i2c)
{
    setup(scl, sda, i2c);
};

SCD_30::SCD_30(int scl, int sda)
{
    setup(scl, sda, 0x61);
};

void SCD_30::setup(int scl = D3, int sda = D4, int i2c = 0x61)
{
    _scd  = new SCD30();
    _wire = new TwoWire();
    _wire->begin(sda, scl);
    _scd->begin(*_wire);

    in.co2 = 0;

    sense("air co2");
    addToInterval(this);
}

SCD_30::~SCD_30()
{
    delete _scd;
}

void SCD_30::interval()
{
  if (_scd->dataAvailable())
    in.co2 = _scd->getCO2();

    // Check if any reads failed and exit early (will try again).
    if (isnan(in.co2) || in.co2 <= 0) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from SCD (CO2) sensor (CO2 set to 0)!" << endl;
        return;
    }
    
    setSense("air co2", in.co2);
  } // SCD_30::interval
