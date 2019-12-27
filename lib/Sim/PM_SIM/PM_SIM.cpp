#include <PM_SIM.h>

PM_SIM::PM_SIM()
{
    pm = -1;
    sense("AIR_PM1");
    sense("AIR_PM2P5");
    sense("AIR_PM10");

    sense("AIR_AQI_RANGE");
    sense("AIR_PM2P5_RANGE");
    sense("AIR_PM10_RANGE");

    addToLoop(this);
}

PM_SIM::PM_SIM(int set)
{
    pm = set;

    sense("AIR_PM1");
    sense("AIR_PM2P5");
    sense("AIR_PM10");

    sense("AIR_AQI_RANGE");
    sense("AIR_PM2P5_RANGE");
    sense("AIR_PM10_RANGE");

    setSense("AIR_PM1", pm);
    setSense("AIR_PM2P5", pm);
    setSense("AIR_PM10", pm);

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

    setSense("AIR_PM1", pm1);
    setSense("AIR_PM2P5", pm2_5);
    setSense("AIR_PM10", pm10);

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
        if (senseValues["AIR_PM2P5"].value < ranges[i]) {
            setSense("AIR_PM2P5_RANGE", i);
            return;
        }

    setSense("AIR_PM2P5_RANGE", (int) sizeof(ranges));
}

void PM_SIM::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };

    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < sizeof(ranges); i++)
        if (senseValues["AIR_PM10"].value < ranges[i]) {
            setSense("AIR_PM10_RANGE", i);
            return;
        }

    setSense("AIR_PM10_RANGE", (int) sizeof(ranges));
}

void PM_SIM::setWorstRange()
{
    setPM2_5Range();
    setPM10Range();

    int r2_5 = senseValues["AIR_PM2P5_RANGE"].value;
    int r10  = senseValues["AIR_PM2P5_RANGE"].value;

    int range = r2_5 > r10 ? r2_5 : r10;

    setSense("AIR_AQI_RANGE", range);
}
