#ifndef SCD_30_H
#define SCD_30_H

/***************************************************************************
*  SCD30 co2, humidity & temperature
*  These sensors use I2C 0x61 address.
***************************************************************************/

#include "CityOS.h"
#include <Wire.h>
#include <SPI.h>
#include <paulvha_SCD30.h>

class SCD_30 : public CityOS {
public:

    SCD_30(int scl, int sda);
    SCD_30(int scl, int sda, int i2c);
    ~SCD_30();

    struct _in {
        // SCD_30 Sensor
        int co2;
    } in;

    void interval();
    private:
    SCD30 * _scd;
    TwoWire * _wire;
    void setup(int scl, int sda, int i2c);
};

#endif /* ifndef SCD_30_H */
