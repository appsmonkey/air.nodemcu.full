#include "AwsMqttClient.h"


AwsMqttClient::AwsMqttClient()    
{    
    for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
      subscriptionCallbacks[i].topic = String();
      subscriptionCallbacks[i].cb = nullptr;
    }
}

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

bool AwsMqttClient::setupAwsJitp(NTPClient ntpClient){

    
    wifiClientSecure.setBufferSizes(1024, 1024);

    //check heap before
    if (debug) {
        Serial << "AwsMqttClient::setup  " << ESP.getFreeHeap() << endl; 
    }    

    ///load amazon certificate  
    if (!loadAmazonCertificate()){
        return false;
    }

    //load device certificate
    if (!loadCACertificate(deviceCert))
    {
        return false;
    }
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
    // pubSubClient.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });

    //check heap after
    if (debug) {
        Serial << F("AwsMqttClient::setup  Ram ") << ESP.getFreeHeap() << endl; 
    }
    return true;
}

bool AwsMqttClient::setupAws(NTPClient ntpClient){

    
    wifiClientSecure.setBufferSizes(1024, 1024);

    //check heap before
    if (debug) {
        Serial << F("AwsMqttClient::setup  Ram ") << ESP.getFreeHeap() << endl; 
    }    

    ///load amazon certificate  
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

void AwsMqttClient::publish(String topic, String data, SubscriptionCallback cb){  
    subscribe(topic, cb);
    if(isConnected()){
        if (!pubSubClient.publish((char*)topic.c_str(),(char*) data.c_str(), false))
        {
            Serial << F("Failed to publish. ") << getMqttConnectionError(pubSubClient.state()) << endl;
        }

    }
    if (debug) {
        Serial << F("AwsMqttClient::publish Ram ") << ESP.getFreeHeap() << endl; 
    } 
    
}

void AwsMqttClient::local_yield()
{
    yield();

    pubSubClient.loop();   
    
}
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

bool AwsMqttClient::isConnected(){
    
    return pubSubClient.connected();
}

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

int AwsMqttClient::subscribe(String topic, SubscriptionCallback cb){ 
  
    if (topic == NULL || cb == NULL) {
        return -1;
    }  
    addCallback(topic, cb);
    if (pubSubClient.subscribe((const char*)topic.c_str())){
        if (debug)
        {
           Serial << getMqttConnectionError(pubSubClient.state()) << endl; 
        }       
                  
    }
    return 1;
}
void AwsMqttClient::callback(char* topic, byte* payload, unsigned int length) {  
    //call callback mathod in CityOS to handle incoming message   
    payload[length] = '\0';
    handleCallback(topic, (char*) payload);
}
void AwsMqttClient::unsubscribe(String topic)
{
    removeCallback(topic);
    pubSubClient.unsubscribe((char*)topic.c_str());  
}

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
