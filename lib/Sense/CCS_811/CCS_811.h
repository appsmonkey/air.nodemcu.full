#ifndef CCS_811_H
#define CCS_811_H

/***************************************************************************
*  CCS_811 ESO2, TVOC & temperature sensor
*  These sensors use I2C address either 0x5A or 0x5B.
***************************************************************************/

#include "CityOS.h"

#include <Wire.h>
#include <SparkFunCCS811.h>

class CCS_811 : public CityOS {
public:

    CCS_811(int scl, int sda);
    CCS_811(int scl, int sda, int i2c);
    ~CCS_811();

    struct _in {
        // CCS_811 Sensor
        float eco2;
        float tvoc;
    } in;

    void interval();

private:
    CCS811 * _ccs;
    TwoWire * _wire;
    void setup(int scl, int sda, int i2c);
};

#endif /* ifndef CCS_811_H */
