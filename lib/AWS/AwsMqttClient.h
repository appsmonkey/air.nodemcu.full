/**
 * AwsMqttClient.h
 * 
 * AwsMqttClient, AWS IOT MQTT client
 * 
 * @author Creator Zarko Runjevac
 * @version 1.0.0
 */

#ifndef AWSMQTTCLIENT_H
#define AWSMQTTCLIENT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <NTPClient.h>
#include <functional>
#include "Utils.h"

//capacity of DynamicJsonDocument generated on 
//https://arduinojson.org/v5/assistant/ for 
//aws-config.json
// static const size_t JSON_CAPACITY= JSON_OBJECT_SIZE(7) + 220;

#define emptyString String()
#define AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 5           // number of callbeck handlers fro incoming aws mqtt messages

typedef std::function<void (const char*, const char*)> SubscriptionCallback;            //callback method signature

class AwsMqttClient{
public:  
    
    AwsMqttClient();
    AwsMqttClient(String mqtt_host, int mqtt_port, String thingName,
                  String amazonCa, String deviceCa, String rootCa, 
                  String private_key);
    ~AwsMqttClient();

    bool debug = false; 
    //setup connection to AWS using just in time provisioning 
    bool setupAwsJitp(NTPClient ntpClient);
    //setup connection to AWS using certificates manually generated when creating things in AWS IOT
    bool setupAws(NTPClient ntpClient);

    //connect to AWS IOT
    void connect();
    //check if client is connected to AWS IOT
    bool isConnected();
    //publish data to AWS IOT
    void publish(String topic, String data, SubscriptionCallback cb);
    //subscribe callback to topic
    bool subscribe(String topic, SubscriptionCallback cb);
    //unsusbscribe from topic
    void unsubscribe(String topic);
    //loops for incoming messages from AWS IOT
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
    
    //load certificates from SPIFS
    uint8_t* loadCertificate(String cert,size_t &size);
    //loads Amazon root certificate E.g. amazonrootca.der
    bool loadAmazonCertificate();
    //loads CA certificates E.g. for jitp rootCA.der(registered certificate for provisioning service)
    bool loadCACertificate(String cert);
    //loads private key
    bool loadPrivateKey();
    //get mqqt error
    String getMqttConnectionError(int8_t MQTTErr);
    // callback supplied to PubSub Mqtt client
    void callback(char* topic, byte* payload, unsigned int length);
    //add callback which is triggered when message from AWS IOT arrives
    void addCallback(String topic, SubscriptionCallback cb);
    //remove callback    
    void removeCallback(String topic);

    SubscriptionCallback getCallback(String topic);
    //handles callback delegate
    void handleCallback(const char* topic, const char* msg);

    unsigned long elapsed_time(unsigned long start_time_ms);
    //yield implementation for PubSubClient https://www.sigmdel.ca/michel/program/esp8266/arduino/watchdogs_en.html
    void local_yield();
};

#endif
   
   
