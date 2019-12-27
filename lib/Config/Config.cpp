#include "Config.h"

bool Config::loadConfig(){
    if(SPIFFS.begin()){
        if(debug)
            Serial << F("SPIFFS mounted file system --- Loading config") << endl;
        if(SPIFFS.exists(F("/config.json"))){

            File configFile=SPIFFS.open(F("/config.json"), "r");

            if(configFile){
                
                size_t size=configFile.size();

                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);

                DynamicJsonDocument json(JSON_CAPACITY);
                auto error=deserializeJson(json, buf.get());                
                if (!error)
                {        
                    JsonObject obj = json.as<JsonObject>();

                    if (obj.containsKey("aws"))
                    {                        
                        JsonObject aws = obj["aws"];
                        loadAwsConfig(aws);
                      
                    }
                    else
                    {
                        if(debug)
                            Serial << F("Failed to load json config.") << endl;
                        return false;  
                    } 

                    if (obj.containsKey("senses"))
                    {
                        JsonObject senses = obj["senses"];
                        loadSensesConfig(senses);                      
                    }
                    else
                    {
                        if(debug)
                            Serial << F("Failed to load json config.") << endl;
                        return false;  
                    } 
            
                }else
                {
                    if(debug)
                        Serial << F("Failed to load json config.") << endl;
                    return false;  
                } 
            }
        } else{
            if(debug)
                Serial << F("Failed to load config.json.") << endl;
            return false;    
        }   
    }else{
        if(debug)
            Serial << F("Failed to mount file system.") << endl;
        return false;
    }
    return true;
}

void Config::loadAwsConfig(JsonObject aws){
    if (aws.containsKey("mqtt_host"))
    {
        awsConfig.mqttHost = aws["mqtt_host"].as<String>();
    }
    if (aws.containsKey("thing"))
    {
        awsConfig.thing = aws["thing"].as<String>();
    }
    if (aws.containsKey("mqtt_port"))
    {
        awsConfig.mqttPort = aws["mqtt_port"].as<String>();
    }
    if (aws.containsKey("amazon_ca"))
    {
        awsConfig.amazonCa = aws["amazon_ca"].as<String>();
    }
    if (aws.containsKey("device_ca"))
    {
        awsConfig.deviceCa = aws["device_ca"].as<String>();
    }
    if (aws.containsKey("root_ca"))
    {
        awsConfig.rootCa = aws["root_ca"].as<String>();
    }
    if (aws.containsKey("private_key"))
    {
        awsConfig.privateKey = aws["private_key"].as<String>();
    }
    if (aws.containsKey("location"))
    {
        awsConfig.location = aws["location"].as<String>();
    }   
}

void Config::loadSensesConfig(JsonObject senses){
    for (JsonPair p : senses)
    {
        String key(p.key().c_str());

        JsonObject obj = p.value();

        if (obj.containsKey("shadow_key"))
        {
            sensesConfig[key].shadowKey = obj["shadow_key"].as<String>();
        }
        else
        {
            sensesConfig[key].shadowKey =  key;
        }
                            
        if (obj.containsKey("threshold"))
        {
            sensesConfig[key].threshold = obj["threshold"];
        }
        else
        {
            sensesConfig[key].threshold = DEFAULT_THRESHOLD;
        }
                            

        if (obj.containsKey("valid_min"))
        {
            sensesConfig[key].min = obj["valid_min"] | std::numeric_limits<double>::min();
        }
        else
        {
            sensesConfig[key].min = std::numeric_limits<double>::min();
        }                            

        if (obj.containsKey("valid_max"))
        {
            sensesConfig[key].max = obj["valid_max"] | std::numeric_limits<double>::max();
        }
        else
        {
           sensesConfig[key].max = std::numeric_limits<double>::max();
        }
                            
    }
}