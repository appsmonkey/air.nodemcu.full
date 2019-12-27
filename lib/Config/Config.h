/**
 * Config.h
 * 
 * Config class for loading config files from SPIFFS
 * 
 * @author Creator Zarko Runjevac
 * @version 1.0.0
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <string>
#include <FS.h>
#include "Utils.h"
#include <Constants.h>

//capacity of DynamicJsonDocument generated on 
//https://arduinojson.org/v5/assistant/ for config.json

const size_t JSON_CAPACITY = 4*JSON_OBJECT_SIZE(2) + 16*JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(30) + 1490;

//struct for keeping config for senses
struct SenseConfig {
    double min;
    double max;
    double threshold;
    String shadowKey;
};

class Config{
public:    
    struct AwsConfig{
        String mqttHost;
        String thing;
        String mqttPort;
        String amazonCa;
        String deviceCa;
        String rootCa;
        String privateKey;
        String location;
    } awsConfig;

    std::map<String, SenseConfig> sensesConfig;

    bool loadConfig();
private:        
    void loadAwsConfig(JsonObject aws);//load aws config
    void loadSensesConfig(JsonObject senses);//load senses config
    bool debug = false;
};
#endif