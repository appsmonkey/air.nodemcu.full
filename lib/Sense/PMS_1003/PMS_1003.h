#ifndef PMS_1003_H
#define PMS_1003_H

#include <CityOS.h>
#include "SoftwareSerial.h"


class PMS_1003: public CityOS {
public:

    PMS_1003(int rx, int tx);

private:

    void interval();

    struct _PIN {
        int rx; // PM Sensors RX PIN
        int tx; // PM Sensors TX PIN
    } pin;
    static const uint16_t max_wait_ms = 1000;
    SoftwareSerial * _swSer;
    char checkValue(unsigned char * thebuf, char leng);
    int read16Bits(unsigned char * thebuf, int offset);

    void setPM2_5Range();
    void setPM10Range();
    void setWorstRange();
};

#endif /* ifndef PMS_1003_H */
