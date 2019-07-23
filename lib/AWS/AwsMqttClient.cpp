/**
 * AwsMqttClient.cpp
 * 
 * AwsMqttClient, AWS IOT MQTT client
 * 
 * @author Creator Zarko Runjevac
 * @version 1.0.0
 */

#include "AwsMqttClient.h"

/**
 * Constructor initializes subscription handlers
 */
AwsMqttClient::AwsMqttClient()    
{    
    for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
      subscriptionCallbacks[i].topic = String();
      subscriptionCallbacks[i].cb = nullptr;
    }
}
/**
 * Constructor initializes subscription handlers and inits AWS object
 * 
 * @param mqtt_host address of mqtt host E.g. xxxxxxxxx.iot.us-east-1.amazonaws.com
 * @param mqtt_port port of mqtt server E.g 8883
 * @param thingName AWS IOT thing name
 * @param amazonCa name of the amazon root certificate 
 * @param deviceCa name device certificate (generated for example using AWS IOT JITP )
 * @param rootCa name of the root certificate for JITP service
 * @param private_key name of the private key
 */
AwsMqttClient::AwsMqttClient(String mqtt_host, int mqtt_port, String thingName,
              String amazonCa, String deviceCa, String rootCa, 
              String private_key)
{   
    mqttHost = mqtt_host;
    mqttPort = mqtt_port;
    thing = thingName;
    amazonCert = amazonCa;
    deviceCert = deviceCa;
    rootCA = rootCa;
    privateKey = private_key;

    for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
      subscriptionCallbacks[i].topic =  String();
      subscriptionCallbacks[i].cb = nullptr;
    }
}

AwsMqttClient::~AwsMqttClient(){
    
}
/**
 * Set up AWS IOT JITP connection https://aws.amazon.com/blogs/iot/setting-up-just-in-time-provisioning-with-aws-iot-core/
 * @param ntpClient current date/time to perform the TLS handshake.
 * 
 * @returns true if setup successfull
 */
bool AwsMqttClient::setupAwsJitp(NTPClient ntpClient){

    
    wifiClientSecure.setBufferSizes(1024, 1024);

    //check heap before
    if (debug) {
        Serial << "AwsMqttClient::setup  " << ESP.getFreeHeap() << endl; 
    }    

    //load amazon certificate  
    if (!loadAmazonCertificate()){
        return false;
    }

    //load device certificate
    if (!loadCACertificate(deviceCert))
    {
        return false;
    }
    //load root certificate for jitp service 
    if (!loadCACertificate(rootCA))
    {
        return false;
    }

    //load private key
    if (!loadPrivateKey())
    {
        return false;
    }   

    
    wifiClientSecure.setX509Time(ntpClient.getEpochTime());

    wifiClientSecure.setTrustAnchors(&amazon_crt);
    wifiClientSecure.setClientRSACert(&client_crt, &private_key);

    pubSubClient.setClient(wifiClientSecure);
    pubSubClient.setServer(mqttHost.c_str(), mqttPort);

    //check heap after
    if (debug) {
        Serial << F("AwsMqttClient::setup  Ram ") << ESP.getFreeHeap() << endl; 
    }
    return true;
}
/**
 * Set up AWS IOT connection 
 * Used when thing certificates are manually generated
 * @param ntpClient current date/time to perform the TLS handshake.
 * 
 * @returns true if setup successfull
 */
bool AwsMqttClient::setupAws(NTPClient ntpClient){

    
    wifiClientSecure.setBufferSizes(1024, 1024);

    //check heap before
    if (debug) {
        Serial << F("AwsMqttClient::setup  Ram ") << ESP.getFreeHeap() << endl; 
    }    

    //load amazon certificate  
    if (!loadAmazonCertificate()){
        return false;
    }

    //load device certificate
    if (!loadCACertificate(deviceCert))
    {
        return false;
    }
 

    //load private key
    if (!loadPrivateKey())
    {
        return false;
    }   

    
    wifiClientSecure.setX509Time(ntpClient.getEpochTime());

    wifiClientSecure.setTrustAnchors(&amazon_crt);
    wifiClientSecure.setClientRSACert(&client_crt, &private_key);

    pubSubClient.setClient(wifiClientSecure);
    pubSubClient.setServer(mqttHost.c_str(), mqttPort);
    pubSubClient.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });

    //check heap after
    if (debug) {
        Serial << F("AwsMqttClient::setup  Ram ") << ESP.getFreeHeap() << endl; 
    }

    return true;
}
/**
 * Reads Amazon root certificate from SPIFFS and appens it to BearSSL::X509List amazon_crt
 * @returns true if certificate is loaded, otherwise return false
 */
bool AwsMqttClient::loadAmazonCertificate(){
    size_t&& size=0;
    uint8_t *buffer;
    ///load amazon certificate  
    buffer=loadCertificate(amazonCert, size);
    
    if (!amazon_crt.append(buffer, size)) {
        free(buffer);
        if(debug)
            Serial << F("Failed to load Amazon root certificate.") << endl;
        return false;
        free(buffer);
    }
    free(buffer);
    return true;
}
/**
 * Reads  certificate from SPIFFS and appens it to BearSSL::X509List client_crt
 * @returns true if certificate is loaded, otherwise return false
 */
bool AwsMqttClient::loadCACertificate(String cert){
    size_t&& size=0;
    uint8_t *buffer;

    //load device certificate
    buffer=loadCertificate(cert, size);
       
    if (!client_crt.append(buffer, size)) {
        free(buffer);
        if(debug)
            Serial << F("Failed to load device certificate.") << endl;
        return false;
    }
    
    free(buffer);
    return true;
}
/**
 * Reads private key from SPIFFS and appens it to BearSSL::PrivateKey private_key
 * @returns true if private key is loaded, otherwise return false
 */
bool AwsMqttClient::loadPrivateKey(){
    size_t&& size=0;
    uint8_t *buffer;

    buffer = loadCertificate(privateKey, size);
    
    if (!private_key.parse(buffer, size)) {
        free(buffer);
        if(debug)
            Serial << F("Failed to load private key certificate.") << endl;
        return false;
    }

    free(buffer);
    return true;
}
/**
 * Reads a file from SPIFFS
 * @param file to read
 * @returns uint8_t buffer if files exists. If file doesn't exist returns nullptr
 */
uint8_t* AwsMqttClient::loadFile(File &file){
    size_t size=file.size();
    uint8_t *buf = (uint8_t*)malloc(size);

    if (!buf) {
      return nullptr;
    }
    if (size != file.read(buf, size)) {
      free(buf);
      return nullptr;
    }  
    file.close();
    return buf;
}
/**
 * Load an Certificate form SPIFFS 
 * @param cert certificate name E.g "/amazonrootca.der"
 * @param size[out] file size
 * 
 * @returns uint8_t buffer if files exists. If file doesn't exist returns nullptr
 */
uint8_t* AwsMqttClient::loadCertificate(String cert, size_t &size){   
    if(SPIFFS.begin()){
        if (debug)
            Serial << F("SPIFFS mounted file system") << endl;
        File file = SPIFFS.open(cert, "r");
        
        if (!file) {
            if (debug)
                Serial << cert <<F(" missing!") << endl;
            return nullptr;    
        }  
        
        size=file.size();
        uint8_t *buffer=loadFile(file);
        return buffer;          

    }else{
        if(debug)
            Serial << F("Failed to mount file system.") << endl;
        return nullptr;
    }            
}
/**
 * Connects Esp8266 with AWS IOT
 */
void AwsMqttClient::connect(){
    if (debug){
        Serial << F("Connecting to AWS...") << endl;
    }     
    while(!isConnected()){  
        yield();
        pubSubClient.connect(thing.c_str());        
        if(!isConnected()){
            if (debug)
            {
                Serial << F("Failed to connect. ") << getMqttConnectionError(pubSubClient.state()) 
                    <<F(". Try again in 2 seconds.") << endl;
                
                Serial << F(" AwsMqttClient::connect Ram ") << ESP.getFreeHeap() << endl;     
             delay(2000);    
            }            
        }else{
            if (debug)
            {
                Serial << F("Connected!") << endl ;
            } 
            pubSubClient.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });            
        }        
    }        
    //check heap after
    if (debug) {
        Serial << F("AwsMqttClient::connect Ram ") << ESP.getFreeHeap() << endl; 
    }
  
}
/**
 * Publish data to AWS IOT Core
 * @param topic topic where data is published
 * @param data data to send
 * @param cb callback to call when response arrives
 */
void AwsMqttClient::publish(String topic, String data, SubscriptionCallback cb){  
    subscribe(topic, cb);
    
    if (debug)
    {
        Serial << F("AwsMqttClient::publish start")<<endl;
    }
    
    if (!pubSubClient.publish((char*)topic.c_str(),(char*) data.c_str(), false))
    {
        Serial << F("Failed to publish. ") << getMqttConnectionError(pubSubClient.state()) << endl;
    }

    if (debug) {
        Serial << F("AwsMqttClient::publish Ram ") << ESP.getFreeHeap() << endl; 
    } 
    
}
/**
 * Yield for PubSubClient Loop https://www.sigmdel.ca/michel/program/esp8266/arduino/watchdogs_en.html
 */
void AwsMqttClient::local_yield()
{
    yield();

    pubSubClient.loop();   
    
}
/**
 * Loops for incoming messages from AWS IOT
 * @param millisecs interval to check
 */
void AwsMqttClient::loop(unsigned long millisecs)
{
  unsigned long start = millis();
  local_yield();
  if (millisecs > 0)
  {
    while (elapsed_time(start) < millisecs)
    {
      local_yield();
    }
  }
}

unsigned long AwsMqttClient::elapsed_time(unsigned long start_time_ms)
{
  return millis() - start_time_ms;
}
/**
 * Checks connection between AWS IOT and ESP8266
 * @returns true if connected, otherwise false
 */
bool AwsMqttClient::isConnected(){
    
    return pubSubClient.connected();
}
/**
 * Returns error according to error code
 * @param MQTTErr error code
 */
String AwsMqttClient::getMqttConnectionError(int8_t MQTTErr){
    switch (MQTTErr)
    {
    case MQTT_CONNECTION_TIMEOUT:
        return F("Connection timeout.");
        break;
    case MQTT_CONNECTION_LOST:
        return F("Connection lost.");
        break;
    case MQTT_CONNECT_FAILED:
        return F("Connect failed.");
        break;
    case MQTT_DISCONNECTED:
        return F("Disconnected.");
        break;
    case MQTT_CONNECTED:
        return "Connected.";
        break;
    case MQTT_CONNECT_BAD_PROTOCOL:
        return F("Connect bad protocol.");
        break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
        return F("Connect bad Client-ID.");
        break;
    case MQTT_CONNECT_UNAVAILABLE:
        return F("Connect unavailable.");
        break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
        return F("Connect bad credentials.");
        break;  
    case MQTT_CONNECT_UNAUTHORIZED:
        return F("Connect unauthorized.");
        break;       
    default:
        return F("Unknown error.");
        break;
    }
}
/**
 * Makes a subscription to a topic and adds hanler to respond to 
 * 
 * @param topic topic to subscribe
 * @param cb callback to respond to topic
 */
bool AwsMqttClient::subscribe(String topic, SubscriptionCallback cb){ 
  
    if (topic == NULL || cb == NULL) {
        return false;
    }  
    addCallback(topic, cb);
    if (pubSubClient.subscribe((const char*)topic.c_str())){
        if (debug)
        {
           Serial << getMqttConnectionError(pubSubClient.state()) << endl; 
        }       
                  
    }
    return true;
}
/**
 * Callback method for PubSubClient. Topic and message are delegated to handleCallback
 * @param topic topic to subscribe
 * @param payload message received
 * @param length length of received message
 */
void AwsMqttClient::callback(char* topic, byte* payload, unsigned int length) {        
    payload[length] = '\0';
    //call callback method in CityOS object to handle incoming message
    handleCallback(topic, (char*) payload);
}
/**
 * Unsubscribes from topic
 * @param topic topic to unsubscribe
 */
void AwsMqttClient::unsubscribe(String topic)
{
    removeCallback(topic);
    pubSubClient.unsubscribe((char*)topic.c_str());  
}
/**
 * Adds callback method to subscriptionCallbacks list
 * @param topic topic for callback method
 * @param cb callback method
 */
void AwsMqttClient::addCallback(String topic, SubscriptionCallback cb){
 
    if (topic == NULL || cb == NULL) {
        return;
    }

    for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
      if (subscriptionCallbacks[i].topic != emptyString && subscriptionCallbacks[i].topic == topic) {
        break;            
      }          
      if (subscriptionCallbacks[i].topic == emptyString) {            
            subscriptionCallbacks[i].topic=topic;
            subscriptionCallbacks[i].cb = cb;
            break;
      }
    }
}
/**
 * Removes callback method to subscriptionCallbacks list
 * @param topic topic to remove from subscriptionCallbacks list
 */
void AwsMqttClient::removeCallback(String topic){
    if (topic == NULL) {
      return;
    }

    for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
        if (subscriptionCallbacks[i].topic == topic) {
            subscriptionCallbacks[i].topic=String();
            subscriptionCallbacks[i].cb = nullptr;
          break;
        }
    }
}

/**
 * Returns callback method from subscriptionCallbacks list for topic
 * @param topic topic to search in subscriptionCallbacks list
 */
SubscriptionCallback AwsMqttClient::getCallback(String topic){

    if (topic == NULL) {
        return NULL;
    }

    for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
        if (subscriptionCallbacks[i].topic == topic) {
          return subscriptionCallbacks[i].cb;
        }
    }
    return NULL;
}
/**
 * Finds a topic in subscriptionCallbacks list and calls associated method
 */
void AwsMqttClient::handleCallback(const char* topic, const char* msg){

    String str(topic);
    SubscriptionCallback cb = getCallback(str);
    if (topic != NULL && cb != NULL) {  
      cb(topic, msg);
    }
}

String AwsMqttClient::getThing(){    
    return thing; 
}

void AwsMqttClient::setMqttHost(String host){
    mqttHost=host;
}
void AwsMqttClient::setMqttPort(int port){
    mqttPort=port;
}
void AwsMqttClient::setThingName(String name){
    thing=name;
}
void AwsMqttClient::setAmazonCert(String cert){
    amazonCert=cert;
}
void AwsMqttClient::setDeviceCert(String cert){
    deviceCert=cert;
}
void AwsMqttClient::setRootCA(String cert){
    rootCA=cert;
}
void AwsMqttClient::setPrivateKey(String cert){
    privateKey=cert;
}
