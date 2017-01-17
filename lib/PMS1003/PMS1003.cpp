#include <PMS1003.h>

PMS1003::PMS1003(int rx, int tx)
{
    addSensor("pm_air_1");
    addSensor("pm_air_2.5");
    addSensor("pm_air_10");
}

PMS1003::~PMS1003(){ }
