#ifndef Air_h
#define Air_h

#include "CityOS.h"
#include "CTOS_NodeMCU.h"
#include "CTOS_PMS1003.h"

#include "Adafruit_Sensor.h"
#include "SoftwareSerial.h"
#include "DHT.h"


class Air: public CityOS {
public:
    Air();
    ~Air();

    struct _SENSOR {
        // DHT22 Sensor
        float humidity;
        float temperature;
        float feelsLike;

        // PM Sensor
        int   pm1;
        int   pm2_5;
        int   pm10;
    } sensor;

    struct _RANGE {
        int pm2_5;
        int pm10;
        int worst;
    } range;

    struct _PIN {
        int dht;   // DHT Sensor
        int red;   // Led PIN
        int green; // Led PIN
        int blue;  // Led PIN
        int pmRX;  // PM Sensors RX PIN
        int pmTX;  // PM Sensors TX PIN
    } pin;

    void s();
    void l();
    void setup();
    void loop();

private:

    void readPM();
    void readDHT();

    void setPMSerial(int rxPin, int txPin);

    DHT * _dht;

    char checkValue(unsigned char * thebuf, char leng);

    // 0-5
    int setPM2_5Range();
    int setPM10Range();
    int setWorstRange();

    // PRIVATE: Move to private when done converting
    // Set color on all available RGB pins based on range provided
    void setLedColors();

    int read16Bits(unsigned char * thebuf, int offset);
};
// END OF Air

#endif /* ifndef Air_h */


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
