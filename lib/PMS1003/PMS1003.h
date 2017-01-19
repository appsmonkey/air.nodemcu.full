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

/*
 * {
 * { "Good (0-50)", "Minimal impact" },
 * { "Satisfactory  (51-100)", "May cause minor breathing discomfort to sensitive people." },
 * { "Moderately polluted (101–200)", "May cause breathing discomfort to people with lung disease such as asthma, and discomfort to people with heart disease, children and older adults." },
 * { "Poor (201-300)", "May cause breathing discomfort to people on prolonged exposure, and discomfort to people with heart disease." },
 * { "Very poor (301-400)", "May cause respiratory illness to the people on prolonged exposure. Effect may be more pronounced in people with lung and heart diseases." },
 * { "Severe (401-500)", "May cause respiratory impact even on healthy people, and serious health impacts on people with lung/heart disease. The health impacts may be experienced even during light physical activity." }
 * }
 */
