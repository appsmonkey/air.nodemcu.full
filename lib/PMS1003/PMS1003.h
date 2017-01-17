#ifndef PMS1003_H
#define PMS1003_H

#include <CityOS.h>

class PMS1003: public CityOS {
public:
    PMS1003(int rx, int tx);
    ~PMS1003();
};

#endif /* ifndef PMS1003_H */
