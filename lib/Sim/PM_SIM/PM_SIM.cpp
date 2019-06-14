#include <PM_SIM.h>

PM_SIM::PM_SIM()
{
    pm = -1;
    sense(config["AIR_PM_1"]);
    sense(config["AIR_PM_2P5"]);
    sense(config["AIR_PM_10"]);

    sense(config["AIR_AQI_RANGE"]);
    sense(config["AIR_PM_2P5_RANGE"]);
    sense(config["AIR_PM_10_RANGE"]);

    addToLoop(this);
}

PM_SIM::PM_SIM(int set)
{
    pm = set;

    sense(config["AIR_PM_1"]);
    sense(config["AIR_PM_2P5"]);
    sense(config["AIR_PM_10"]);

    sense(config["AIR_AQI_RANGE"]);
    sense(config["AIR_PM_2P5_RANGE"]);
    sense(config["AIR_PM_10_RANGE"]);

    setSense(config["AIR_PM_1"], pm);
    setSense(config["AIR_PM_2P5"], pm);
    setSense(config["AIR_PM_10"], pm);

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

    setSense(config["AIR_PM_1"], pm1);
    setSense(config["AIR_PM_2P5"], pm2_5);
    setSense(config["AIR_PM_10"], pm10);

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
        if (senseValues[config["AIR_PM_2P5"]].value < ranges[i]) {
            setSense(config["AIR_PM_2P5_RANGE"], i);
            return;
        }

    setSense(config["AIR_PM_2P5_RANGE"], (int) sizeof(ranges));
}

void PM_SIM::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };

    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < sizeof(ranges); i++)
        if (senseValues[config["AIR_PM_10"]].value < ranges[i]) {
            setSense(config["AIR_PM_10_RANGE"], i);
            return;
        }

    setSense(config["AIR_PM_10_RANGE"], (int) sizeof(ranges));
}

void PM_SIM::setWorstRange()
{
    setPM2_5Range();
    setPM10Range();

    int r2_5 = senseValues[config["AIR_PM_2P5_RANGE"]];
    int r10  = senseValues[config["AIR_PM_10_RANGE"]];

    int range = r2_5 > r10 ? r2_5 : r10;

    setSense(config["AIR_AQI_RANGE"], range);
}
