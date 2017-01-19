#ifndef PMS1003_CTOS_H
#define PMS1003_CTOS_H

#include <CityOS.h>
#include "SoftwareSerial.h"


class PMS1003: public CityOS {
public:

    struct _PIN {
        int pmRX; // PM Sensors RX PIN
        int pmTX; // PM Sensors TX PIN
    } pin;

    struct _IN {
        // PM Sensor
        int pm1;
        int pm2_5;
        int pm10;
        int range;
    } in;

    struct _RANGE {
        int pm2_5;
        int pm10;
        int worst;
    } range;

    PMS1003(int rx, int tx);
    void setup();
    void loop();

private:
    char checkValue(unsigned char * thebuf, char leng);
    int read16Bits(unsigned char * thebuf, int offset);

    int setPM2_5Range();
    int setPM10Range();
    int setWorstRange();
};

#endif /* ifndef PMS1003_CTOS_H */
