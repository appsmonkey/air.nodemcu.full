#include <PM_SIM.h>

PM_SIM::PM_SIM()
{
    pm = -1;
    sense("air pm 1");
    sense("air pm 2.5");
    sense("air pm 10");

    sense("air aqi range");
    sense("air pm 2.5 range");
    sense("air pm 10 range");

    addToLoop(this);
}

PM_SIM::PM_SIM(int set)
{
    pm = set;

    sense("air pm 1");
    sense("air pm 2.5");
    sense("air pm 10");

    sense("air aqi range");
    sense("air pm 2.5 range");
    sense("air pm 10 range");

    setSense("air pm 1", pm);
    setSense("air pm 2.5", pm);
    setSense("air pm 10", pm);

    setWorstRange();

    addToLoop(this);
}

void PM_SIM::loop()
{
    if (pm > -1)
        return;

    static int lastUpdate = 0;

    if ((millis() - lastUpdate) < 100) {
        yield();
        return;
    }

    lastUpdate = millis();

    static int pm1   = 1;
    static int pm2_5 = 1;
    static int pm10  = 1;

    static int step = 3;

    pm1   += step;
    pm2_5 += step;
    pm10  += step;

    if (pm1 >= 300 || pm1 <= 0)
        step = -step;

    setSense("air pm 1", pm1);
    setSense("air pm 2.5", pm2_5);
    setSense("air pm 10", pm10);

    setWorstRange();
} // PM_SIM::interval

// 0-5
void PM_SIM::setPM2_5Range()
{
    // USA
    int ranges[5] = { 13, 36, 56, 151, 251 };

    // Asia
    // int ranges[5] = { 30, 60, 90, 120, 250 };

    for (int i = 0; i < sizeof(ranges); i++)
        if (senseValues["air pm 2.5"] < ranges[i]) {
            setSense("air pm 2.5 range", i);
            return;
        }

    setSense("air pm 2.5 range", (int) sizeof(ranges));
}

void PM_SIM::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };

    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < sizeof(ranges); i++)
        if (senseValues["air pm 10"] < ranges[i]) {
            setSense("air pm 10 range", i);
            return;
        }

    setSense("air pm 10 range", (int) sizeof(ranges));
}

void PM_SIM::setWorstRange()
{
    setPM2_5Range();
    setPM10Range();

    int r2_5 = senseValues["air pm 2.5 range"];
    int r10  = senseValues["air pm 10 range"];

    int range = r2_5 > r10 ? r2_5 : r10;

    setSense("air aqi range", range);
}
