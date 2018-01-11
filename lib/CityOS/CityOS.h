#ifndef CITYOS_h
#define CITYOS_h

#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
# include "pins_arduino.h"
# include "WConstants.h"
#endif

#include "Config.ctos.h"
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
        bool schema    = false;
        bool api       = false;
        bool webserver = false;
        bool wifi      = false;
        bool json      = false;
        bool memory    = false;
        bool senses    = false;
        bool controls  = false;
    } debug;

    struct _API {
        bool   active = false;
        String host;
        int    port;
        String token;    // GRID TOKEN
        String deviceID; // GRID Device ID
        int    timeout;  // Before giving up on requests
    } api;

    struct _SENSING {
        bool active = false;
        int  interval;
    } sensing;

    struct _WIFI {
        bool   active = false; // WIFI SSID
        String ssid;           // WIFI SSID
        String pass;           // WIFI PASSWORD
    } wifi;

    struct _WEBSERVER {
        bool active = false;
        int  port;
    } webserver;

    // Arduino loop() func every time
    void setup();
    virtual void loop();

    // loop() on interval timing
    virtual void interval();

    // Used in interval, returns old value
    static int setSense(String sense, int value);
    static float setSense(String sense, float value);

    static int setControl(String control, int value);
    static float setControl(String control, float value);

    void rest(String method, String url, String json);
protected:

    static std::vector<CityOS *> loops;
    static std::vector<CityOS *> intervals;

    static std::vector<String> senses;
    static std::vector<String> controls;

    static std::map<String, float> senseValues;
    static std::map<String, float> controlValues;

    // Debug Info function
    String getMacHEX();

    int getSensorCount();

    // physical sensors and controlables
    int sense(String sense);
    int control(String control);

    virtual void addToLoop(CityOS *);
    virtual void addToInterval(CityOS *);

    // Used in Setup - Parse schema JSON and send it to server
    void sendSchema();
    void sendSenses();
    void requestControls();
    void handleControls();

    WiFiServer * _server;
    WiFiClient _client;

    void serveHTML();
    const char * HTMLHead();
    const char * HTMLFoot();

    ////Debug Info Functions
    void printWifiStatus();
    void printHeapSize();
    void printSenses();
    void printControls();
};

// - - - - - - - - - - - - - //


// Generic template to enable << operator on streams
template <class T>
inline Print &operator << (Print & stream, T arg)
{
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

inline Print &operator << (Print & obj, const _BYTE_CODE &arg)
{
    obj.write(arg.val);
    return obj;
}

#define _HEX(a) _BASED(a, HEX)
#define _DEC(a) _BASED(a, DEC)
#define _OCT(a) _BASED(a, OCT)
#define _BIN(a) _BASED(a, BIN)

inline Print &operator << (Print & obj, const _BASED &arg)
{
    obj.print(arg.val, arg.base);
    return obj;
}

struct _FLOAT {
    float val;
    int   digits;
    _FLOAT(double v, int d) : val(v), digits(d){ }
};

inline Print &operator << (Print & obj, const _FLOAT &arg)
{
    obj.print(arg.val, arg.digits);
    return obj;
}

enum _EndLineCode { endl };

inline Print &operator << (Print & obj, _EndLineCode arg)
{
    obj.println();
    return obj;
}

#endif /* ifndef CITYOS_h */
