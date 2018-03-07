#ifndef YL_39_69_H
#define YL_39_69_H

#include "CityOS.h"

class YL_39_69 : public CityOS {
public:

    // Fine tune using these two
    int top    = 1024;
    int bottom = 620;
    bool info  = false;

    YL_39_69(int power, int read);
    YL_39_69(int power);

    int moisture;

    struct _pin {
        int power;
        int read;
    } pin;

    void interval();

private:
    void setup(int power, int read);
};
#endif /* ifndef DHT_22_H */
