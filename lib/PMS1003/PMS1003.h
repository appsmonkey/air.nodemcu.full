#ifndef PMS1003_CTOS_H
#define PMS1003_CTOS_H

#include <CityOS.h>
#include "SoftwareSerial.h"


class PMS1003: public CityOS {
public:

    PMS1003(int rx, int tx);

private:

    void setup();
    void loop();

    struct _PIN {
        int rx; // PM Sensors RX PIN
        int tx; // PM Sensors TX PIN
    } pin;

    char checkValue(unsigned char * thebuf, char leng);
    int read16Bits(unsigned char * thebuf, int offset);

    void setPM2_5Range();
    void setPM10Range();
    void setWorstRange();
};

#endif /* ifndef PMS1003_CTOS_H */
