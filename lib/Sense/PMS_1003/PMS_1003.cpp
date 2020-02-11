#include <PMS_1003.h>


PMS_1003::PMS_1003(int rx, int tx)
{
    
    pin.rx = rx;
    pin.tx = tx;

    _swSer = new SoftwareSerial(pin.rx, pin.tx);
    
    sense("AIR_PM1");
    sense("AIR_PM2P5");
    sense("AIR_PM10");

    sense("AIR_AQI_RANGE");
    sense("AIR_PM2P5_RANGE");
    sense("AIR_PM10_RANGE");

    addToInterval(this);
    addToSetup(this);
}

PMS_1003::~PMS_1003()
{
    delete _swSer;
}

void PMS_1003::setup(){
    
   //set up and read pm before wifi 
    _swSer->setTimeout(2000);

    _swSer->begin(9600); // PM Serial

    unsigned int LENG = 31;
    unsigned char buf[LENG]; // 0x42 + 31 bytes equal to 32 bytes

    bool found = readPM(buf, LENG);
    
    if (!found) {
        if (debug.errors) Serial
                << "ERROR: PM Sensor not functional" << endl;       
    }

}

void PMS_1003::interval()
{     
    
    unsigned int LENG = 31;
    unsigned char buf[LENG]; // 0x42 + 31 bytes equal to 32 bytes
   
    bool found = readPM(buf, LENG);

    
    if (!found) {
        if (debug.errors) Serial
                << "ERROR: PM Sensor not functional" << endl;               
        return;
    }

    if (buf[0] == 0x4d) {
        if (checkValue(buf, LENG)) {
            setSense("AIR_PM1", read16Bits(buf, 9));
            setSense("AIR_PM2P5", read16Bits(buf, 11));
            setSense("AIR_PM10", read16Bits(buf, 13));
            
            setWorstRange();
        }
    }   
    
    
} // PMS_1003::loop

bool PMS_1003::readPM(unsigned char *buf, unsigned int length){

    uint32_t start_ms = millis();   // start waiting time
     while((millis() - start_ms) < 3000){
        if(_swSer->available()>0){
            // start to read when detect 0x42 (PM)
           if (_swSer->find(0x42)){
               _swSer->readBytes(buf, length);
               return true;
            }     
        }
        yield();
    }
    return false;
}

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

    for (int i = 0; i < (int)(sizeof(ranges) / sizeof(int)); i++)
        if (senseValues["AIR_PM2P5"].value < ranges[i]) {
            setSense("AIR_PM2P5_RANGE", i);
            return;
        }
    setSense("AIR_PM2P5_RANGE", (int) (sizeof(ranges) / sizeof(int)));
}

void PMS_1003::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };
    
    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < (int)(sizeof(ranges) / sizeof(int)); i++)
        if (senseValues["AIR_PM10"].value < ranges[i]) {
            
            setSense("AIR_PM10_RANGE", i);
            
            return;
        }
    
    setSense("AIR_PM10_RANGE", (int) (sizeof(ranges) / sizeof(int)));
}

void PMS_1003::setWorstRange()
{
    setPM2_5Range();
    setPM10Range();

    int r2_5 = senseValues["AIR_PM2P5_RANGE"].value;
    int r10  = senseValues["AIR_PM10_RANGE"].value;

    int range = r2_5 > r10 ? r2_5 : r10;

    setSense("AIR_AQI_RANGE", range);

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
