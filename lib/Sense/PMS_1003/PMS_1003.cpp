#include <PMS_1003.h>


PMS_1003::PMS_1003(int rx, int tx)
{
    pin.rx = rx;
    pin.tx = tx;

    input("air pm 1");
    input("air pm 2.5");
    input("air pm 10");

    output("air aqi range");
    output("air pm 2.5 range");
    output("air pm 10 range");

    sense(this);
}

void PMS_1003::setup()
{ }

void PMS_1003::loop()
{
    SoftwareSerial * _swSer;

    _swSer = new SoftwareSerial(pin.rx, pin.tx);
    _swSer->begin(9600); // PM Serial
    // set the Timeout to 1500ms
    // longer than the data transmission periodic time of the sensor
    // which is 1000ms
    _swSer->setTimeout(1500);

    // start to read when detect 0x42 (PM)
    if (!_swSer->find(0x42)) {
        if (debug.errors) Serial
                << "ERROR: PM Sensor not functional" << endl;
        return;
    }

    unsigned int LENG = 31;
    unsigned char buf[LENG]; // 0x42 + 31 bytes equal to 32 bytes

    _swSer->readBytes(buf, LENG);
    delete _swSer;

    if (buf[0] == 0x4d) {
        if (checkValue(buf, LENG)) {
            setInputValue("air pm 1", read16Bits(buf, 3));
            setInputValue("air pm 2.5", read16Bits(buf, 5));
            setInputValue("air pm 10", read16Bits(buf, 7));

            setWorstRange();
        }
    }
} // PMS_1003::loop

char PMS_1003::checkValue(unsigned char * thebuf, char leng)
{
    char receiveflag = 0;
    int receiveSum   = 0;

    for (int i = 0; i < (leng - 2); i++) {
        receiveSum = receiveSum + thebuf[i];
    }
    receiveSum = receiveSum + 0x42;

    // check the serial data
    if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) {
        receiveSum  = 0;
        receiveflag = 1;
    }
    return receiveflag;
}

int PMS_1003::read16Bits(unsigned char * thebuf, int offset)
{
    unsigned int PMVal;

    PMVal = ((thebuf[offset] << 8) + thebuf[offset + 1]);
    return PMVal;
}

// 0-5
void PMS_1003::setPM2_5Range()
{
    // USA
    int ranges[5] = { 13, 36, 56, 151, 251 };

    // Asia
    // int ranges[5] = { 30, 60, 90, 120, 250 };

    for (int i = 0; i < sizeof(ranges); i++)
        if (inputValues["air pm 2.5"] < ranges[i]) {
            setOutputValue("air pm 2.5 range", i);
            return;
        }
    setOutputValue("air pm 2.5 range", (int) sizeof(ranges));
}

void PMS_1003::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };

    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < sizeof(ranges); i++)
        if (inputValues["air pm 10"] < ranges[i]) {
            setOutputValue("air pm 10 range", i);
            return;
        }

    setOutputValue("air pm 10 range", (int) sizeof(ranges));
}

void PMS_1003::setWorstRange()
{
    setPM2_5Range();
    setPM10Range();

    int r2_5 = outputValues["air pm 2.5 range"];
    int r10  = outputValues["air pm 10 range"];

    int range = r2_5 > r10 ? r2_5 : r10;

    setOutputValue("air aqi range", range);

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
}
