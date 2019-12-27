#ifndef CITYOS_h
#define CITYOS_h

#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
# include "pins_arduino.h"
# include "WConstants.h"
#endif

#include <ESP8266WiFi.h>
#include <map>
#include <vector>
#include <string>
#include "Utils.h"
#include <AwsMqttClient.h>
#include <Config.h>
#include <Constants.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>


/**
 * Struct to hold old and current value for senses
 */
struct SenseValue {
    double oldValue = 0.00;
    double value = 0.00;

    double min = std::numeric_limits<double>::min();
    double max = std::numeric_limits<double>::max();

    double threshold = DEFAULT_THRESHOLD;
    String shadowKey = "";

    bool operator<(const SenseValue& s) const{
        return (this->value < s.value);
    }

    bool isValid(){
        return ((this->value>=min)&&(this->value<=max));
    }
    bool isOverThreshold(){
        return(std::abs(this->value-this->oldValue) >= threshold);
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
        bool active = true;
        int  interval; //time interval to check sensor readings
        int  heartbeatInterval; 
    } sensing;

    struct _DEVICE{
        String thing;
        String location;
    } device;   

    // Arduino loop() func every time
    virtual void setup();
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

protected:

    static std::vector<CityOS *> loops;
    static std::vector<CityOS *> intervals;
    static std::vector<CityOS *> setups;

    static std::vector<String> senses;
    static std::vector<String> controls;

    static std::map<String, SenseValue> senseValues;
    static std::map<String, double> controlValues;


    // Debug Info function
    String getMacHEX();

    int getSensorCount();

    // physical sensors and controlables
    int sense(String sense);
    int control(String control);

    virtual void addToLoop(CityOS *);
    virtual void addToInterval(CityOS *);
    virtual void addToSetup(CityOS *);

    //Returns data
    String getData();    
    //Returns only changed data
    String getChangedData();
    //Returns data from all senses
    String getAllData();

    void publishData();
    void wifiAndAwsReconnect();

    void requestControls();
    void handleControls();
    //callback for handling response from AWS IOT
    void handleMessages(const char* topic, const char* msg);
    //connect to NTP Server
    void ntpConnect();

    String getTopic();

    AwsMqttClient* _awsMqttClient; 

    NTPClient ntpClient;
    WiFiUDP wifiUDP;

    unsigned long timeStamp;
    unsigned long nextHourSendTime;

    void serveHTML();
    const char * HTMLHead();
    const char * HTMLFoot();

    ////Debug Info Functions
    void printWifiStatus();
    void printHeapSize();
    void printSenses();
    void printSchema();
    void printControls();  

    //conncects Boxy to WIFI
    void connectToWiFi(bool useWiFiManager);   

};



#endif /* ifndef CITYOS_h */
