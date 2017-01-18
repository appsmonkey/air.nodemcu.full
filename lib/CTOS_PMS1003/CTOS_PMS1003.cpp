#include <CTOS_PMS1003.h>

CTOS_PMS1003::CTOS_PMS1003(int rx, int tx)
{
    input("air.pm_1");
    input("air.pm_2_5");
    input("air.pm_10");
    sensor(this);
}

void CTOS_PMS1003::setup()
{ }

void CTOS_PMS1003::loop()
{ }
