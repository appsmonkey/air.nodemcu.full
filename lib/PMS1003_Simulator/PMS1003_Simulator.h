#ifndef PMS1003_SIM_H
#define PMS1003_SIM_H

#include <CityOS.h>
#include "SoftwareSerial.h"


class PMS1003_SIM: public CityOS {
public:

    PMS1003_SIM();

private:

    void setup();
    void loop();

    struct _IN {
        // PM Sensor
        int pm1;
        int pm2_5;
        int pm10;
    } in;

    char checkValue(unsigned char * thebuf, char leng);
    int read16Bits(unsigned char * thebuf, int offset);

    void setPM2_5Range();
    void setPM10Range();
    void setWorstRange();
};

#endif /* ifndef PMS1003_CTOS_H */
