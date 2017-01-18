#ifndef CTOS_PMS1003_H
#define CTOS_PMS1003_H

#include <CityOS.h>

class CTOS_PMS1003: public CityOS {
public:
    CTOS_PMS1003(int rx, int tx);
    void setup();
    void loop();
};

#endif /* ifndef PMS1003_H */
