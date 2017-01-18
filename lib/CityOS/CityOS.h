#ifndef CITYOS_h
#define CITYOS_h

#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
# include "pins_arduino.h"
# include "WConstants.h"
#endif

#include "Config.h"
#include <ESP8266WiFi.h>
#include <map>
#include <vector>
#include <string>

class CityOS {
public:
    CityOS();
    ~CityOS();

    struct _DEBUG {
        bool errors    = false;
        bool readings  = false;
        bool schema    = false;
        bool api       = false;
        bool webserver = false;
        bool led       = false;
        bool wifi      = false;
        bool json      = false;
        bool memory    = false;
    } debug;

    struct _API {
        bool   active = false;
        String host;
        int    port;
        String token;    // GRID TOKEN
        int    deviceID; // GRID Device ID
        int    timeout;  // Before giving up on requests
    } api;

    struct _READINGS {
        bool active = false;
        int  interval;
    } readings;

    struct _WIFI {
        bool   active = false; // WIFI SSID
        String ssid;           // WIFI SSID
        String pass;           // WIFI PASSWORD
    } wifi;

    struct _WEBSERVER {
        bool active = false;
        int  port;
    } webserver;

    // Arduino setup() func
    virtual void setup();

    // Arduino loop() func
    virtual void loop();

protected:

    static int inputCount;
    static int outputCount;

    static std::vector < CityOS * > sensors;
    static std::map < int, String > inputs;
    static std::map < int, String > outputs;
    static std::map < int, float > values;

    std::map < int, float > getAndResetValues();

    void readSensors();

    // Debug Info function
    String getMacHEX();
    int getMacINT();

    int getSensorCount();

    // Every reading is one data point
    int input(String type);
    int output(String type);

    // physical sensor
    virtual int sensor(CityOS *);

    // Used in Setup - Parse schema JSON and send it to server
    void sendSchema();

    // Use in Loop, returns old value
    static float setValue(int position, float newValue);
    static std::map < int, float > getValues();

    WiFiServer * _server;
    WiFiClient _client;

    void serveHTML();
    const char * HTMLHead();
    const char * HTMLFoot();

    void sendData();

    ////Debug Info Functions
    void printWifiStatus();
    void printHeapSize();
};

// - - - - - - - - - - - - - //


// Generic template to enable << operator on streams
template < class T >
inline Print &operator << (Print & stream, T arg){
    stream.print(arg);
    return stream;
    }

    struct _BASED {
    long val;
    int  base;
    _BASED(long v, int b) : val(v), base(b){ }
    };

struct _BYTE_CODE {
    byte val;
    _BYTE_CODE(byte v) : val(v)
    { }
};

#define _BYTE(a) _BYTE_CODE(a)

inline Print &operator << (Print & obj, const _BYTE_CODE &arg){
    obj.write(arg.val);
    return obj;
    }

#define _HEX(a) _BASED(a, HEX)
#define _DEC(a) _BASED(a, DEC)
#define _OCT(a) _BASED(a, OCT)
#define _BIN(a) _BASED(a, BIN)

inline Print &operator << (Print & obj, const _BASED &arg) {
    obj.print(arg.val, arg.base);
    return obj;
    }

    struct _FLOAT {
    float val;
    int   digits;
    _FLOAT(double v, int d) : val(v), digits(d){ }
    };

inline Print &operator << (Print & obj, const _FLOAT &arg){
    obj.print(arg.val, arg.digits);
    return obj;
    }

    enum _EndLineCode { endl };

inline Print &operator << (Print & obj, _EndLineCode arg) {
    obj.println();
    return obj;
    }

#endif /* ifndef CITYOS_h */
