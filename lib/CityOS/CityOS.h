#ifndef CITYOS_h
#define CITYOS_h

#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
# include "pins_arduino.h"
# include "WConstants.h"
#endif

// #include "Config.ctos.h"
#include <ESP8266WiFi.h>
#include <map>
#include <vector>
#include <string>
#include "Utils.h"
#include <AwsMqttClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>

//capacity of DynamicJsonDocument generated on 
//https://arduinojson.org/v5/assistant/ for 
//config.json
static const size_t JSON_CAPACITY= JSON_OBJECT_SIZE(28) + 590;
#define CONFIG_PORTAL_TIMEOUT 40


struct _SENSEVALUE {
    double oldValue = 0;
    double value =0;
    bool operator<(const _SENSEVALUE& s) const{
        return (this->value < s.value);
    }
};

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
        bool config    = false; 
    } debug;



    struct _SENSING {
        bool active = false;
        int  interval;
    } sensing;



    struct _DEVICE
    {
        String thing;
        String location;
    } device;
    

    // Arduino loop() func every time
    void setup();
    virtual void loop();

    // loop() on interval timing
    virtual void interval();

    // Used in interval, returns old value
    double setSense(String sense, int value);
    double setSense(String sense, long value);
    double setSense(String sense, double value);

    static int setControl(String control, int value);
    static double setControl(String control, double value);

    void rest(String method, String url, String json);
    //loads config from SPIFFS
    bool loadConfig();
protected:

    static std::vector<CityOS *> loops;
    static std::vector<CityOS *> intervals;

    static std::vector<String> senses;
    static std::vector<String> controls;

    static std::map<String, _SENSEVALUE> senseValues;
    static std::map<String, double> controlValues;

    static std::map<String, double> old_senseValues;
    //map to hold config value from SPIFFS config.json
    static std::map<String, String> config;

    // Debug Info function
    String getMacHEX();

    int getSensorCount();

    // physical sensors and controlables
    int sense(String sense);
    int control(String control);

    virtual void addToLoop(CityOS *);
    virtual void addToInterval(CityOS *);

    // Used in Setup - Parse schema JSON and send it to server
    String getSchema();
    void sendSchema();
    void sendSenses();
    String getChangedData();
    String getAllData();
    void requestControls();
    void handleControls();
    void handleMessages(const char* topic, const char* msg);
    void ntpConnect();
    String getTopic();

    AwsMqttClient* _awsMqttClient; 

    NTPClient ntpClient;
    WiFiUDP wifiUDP;

    unsigned long timeStamp;

    void serveHTML();
    const char * HTMLHead();
    const char * HTMLFoot();

    ////Debug Info Functions
    void printWifiStatus();
    void printHeapSize();
    void printSenses();
    void printControls();  

    //WiFi
    void connectToWiFi(bool useWiFiManager);   

};



#endif /* ifndef CITYOS_h */
