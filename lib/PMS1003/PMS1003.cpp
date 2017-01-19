#include <PMS1003.h>


PMS1003::PMS1003(int rx, int tx)
{
    input("air pm 1");
    input("air pm 2.5");
    input("air pm 10");

    output("air aqi range");
    output("air pm 2.5 range");
    output("air pm 10 range");

    sense(this);

    pin.pmRX = D7;
    pin.pmTX = D8;
}

void PMS1003::setup()
{ }

void PMS1003::loop()
{
    SoftwareSerial * _swSer;

    _swSer = new SoftwareSerial(pin.pmRX, pin.pmTX);
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
            // count PM1.0 value of the air detector module
            in.pm1 = read16Bits(buf, 3);
            setInputValue("air pm 1", in.pm1);

            // count PM2.5 value of the air detector module
            in.pm2_5 = read16Bits(buf, 5);
            setInputValue("air pm 2.5", in.pm2_5);
            setPM2_5Range();

            // count PM10 value of the air detector module
            in.pm10 = read16Bits(buf, 7);
            setInputValue("air pm 10", in.pm10);
            setPM10Range();
            setWorstRange();
        }
    }
} // PMS1003::loop

char PMS1003::checkValue(unsigned char * thebuf, char leng)
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

int PMS1003::read16Bits(unsigned char * thebuf, int offset)
{
    unsigned int PMVal;

    PMVal = ((thebuf[offset] << 8) + thebuf[offset + 1]);
    return PMVal;
}

// 0-5
int PMS1003::setPM2_5Range()
{
    // USA
    int ranges[5] = { 13, 36, 56, 151, 251 };

    // Asia
    // int ranges[5] = { 30, 60, 90, 120, 250 };

    for (int i = 0; i < sizeof(ranges); i++) {
        if (in.pm2_5 < ranges[i]) {
            setOutputValue("air pm 2.5 range", i);
            return i;
        }
    }
    in.pm2_5 = sizeof(ranges);
    return in.pm2_5;
}

int PMS1003::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };

    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < sizeof(ranges); i++) {
        if (in.pm10 < ranges[i]) {
            setOutputValue("air pm 10 range", i);
            return i;
        }
    }

    range.pm10 = sizeof(ranges);
    return range.pm10;
}

int PMS1003::setWorstRange()
{
    int r2_5 = outputValues["air pm 2.5 range"];
    int r10  = outputValues["air pm 10 range"];

    int range = r2_5 > r10 ? r2_5 : r10;

    setOutputValue("air aqi range", range);
    return range;
}
