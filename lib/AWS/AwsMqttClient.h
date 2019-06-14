#ifndef AWSMQTTCLIENT_H
#define AWSMQTTCLIENT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <NTPClient.h>
// #include <WiFiUdp.h>
#include <functional>
#include "Utils.h"

//capacity of DynamicJsonDocument generated on 
//https://arduinojson.org/v5/assistant/ for 
//aws-config.json
// static const size_t JSON_CAPACITY= JSON_OBJECT_SIZE(7) + 220;

#define emptyString String()
#define AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 5

typedef std::function<void (const char*, const char*)> SubscriptionCallback;

class AwsMqttClient{
public:  
    
    AwsMqttClient();
    AwsMqttClient(String mqtt_host, int mqtt_port, String thingName,
                  String amazonCa, String deviceCa, String rootCa, 
                  String private_key);
    ~AwsMqttClient();

    bool debug = false; 
    bool setupAwsJitp(NTPClient ntpClient);
    bool setupAws(NTPClient ntpClient);

    //connect to aws iot
    void connect();

    bool isConnected();

    void publish(String topic, String data, SubscriptionCallback cb);

    int subscribe(String topic, SubscriptionCallback cb);

    void unsubscribe(String topic);

    void loop(unsigned long millisecs);
        
    void setMqttHost(String host);
    void setMqttPort(int port);
    void setThingName(String name);
    void setAmazonCert(String cert);
    void setDeviceCert(String cert);
    void setRootCA(String cert);
    void setPrivateKey(String cert);
    String getThing();
    

private:                
    // load config from config.json
    String mqttHost;
    int mqttPort;
    String thing;
    String amazonCert;
    String deviceCert;
    String rootCA;
    String privateKey;
    
    struct{
        String topic;
        SubscriptionCallback cb;
    } subscriptionCallbacks[AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS];

    BearSSL::X509List amazon_crt;
    BearSSL::X509List client_crt;
    BearSSL::PrivateKey private_key;

    WiFiClientSecure wifiClientSecure;
    PubSubClient pubSubClient;

    //loads cert file from SPIFFS
    uint8_t* loadFile(File &file);
    
    //load certificates form SPIFS
    uint8_t* loadCertificate(String cert,size_t &size);

    bool loadAmazonCertificate();
    bool loadCACertificate(String cert);
    bool loadPrivateKey();

    String getMqttConnectionError(int8_t MQTTErr);

    void callback(char* topic, byte* payload, unsigned int length);

    void addCallback(String topic, SubscriptionCallback cb);
    void removeCallback(String topic);

    SubscriptionCallback getCallback(String topic);
    void handleCallback(const char* topic, const char* msg);

    unsigned long elapsed_time(unsigned long start_time_ms);
    void local_yield();
};

#endif
   
   
