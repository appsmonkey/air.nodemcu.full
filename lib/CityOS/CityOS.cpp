#include <CityOS.h>

std::vector<String> CityOS::senses;//vector with sense values e.g. ["s1","s2"....]
std::vector<String> CityOS::controls;

std::map<String, SenseValue> CityOS::senseValues;//map with sensevalues e.g. {"s1":(oldValue, newValue), "s2":(oldValue, newValue)...}
std::map<String, double> CityOS::controlValues;

std::vector<CityOS *> CityOS::loops;
std::vector<CityOS *> CityOS::intervals;
std::vector<CityOS *> CityOS::setups;


CityOS::CityOS():
ntpClient(wifiUDP, "pool.ntp.org")
{
    // Print errors to Serial
    debug.errors = true;
    
    sensing.active   = true;

    timeStamp = 0;
    heartbeatSendTime = 0;
   
}

void CityOS::setup()
{
    // Start Serials
    Serial.begin(115200); // serial terminal
    delay(500);

    control("wifi");// add wifi to control to be able to signal in ring if wifi is not connected 

    Config config;

    if(config.loadConfig())
    {
        sensing.interval = config.awsConfig.senseInterval; //  interval to send senses (default is 60 seconds)

        sensing.heartbeat = config.awsConfig.heartbeat;// heartbeat interval to send all senses (default is 3600 seconds)

        _awsMqttClient = new AwsMqttClient(config.awsConfig.mqttHost, config.awsConfig.mqttPort.toInt(), config.awsConfig.thing,
                                           config.awsConfig.amazonCa, config.awsConfig.deviceCa, config.awsConfig.rootCa,
                                           config.awsConfig.privateKey);
                                           
        device.thing = config.awsConfig.thing;
        device.location = config.awsConfig.location;    

    } else{
        _awsMqttClient = nullptr;

        if(debug.config)
                    Serial << F("Failed to load config.") << endl;
    }    
    
    for (auto const& i:setups) {
        i->setup();
        yield();
    }
    //load config values for senses
    for (auto const& sense : senses) {
        if (sense){
            senseValues[sense].min = config.sensesConfig[sense].min;
            senseValues[sense].max  = config.sensesConfig[sense].max;
            senseValues[sense].shadowKey = config.sensesConfig[sense].shadowKey;
            senseValues[sense].threshold  = config.sensesConfig[sense].threshold;         
        }
    }

    connectToWiFi(true);

    //if connected to wifi, setup time and aws connection
    if (WiFi.status() == WL_CONNECTED){
        //set up time      
        ntpConnect();
        //set up aws
        if (_awsMqttClient !=nullptr)
        {
            _awsMqttClient -> setupAwsJitp(ntpClient);
            _awsMqttClient -> connect();
        }               
    }    
    delay(500);
    if (debug.wifi )
        printWifiStatus();
        
    Serial << "ID: " << getMacHEX() << endl;
    Serial << "THING: " << device.thing << endl;

    printSchema();
    
    
} // CityOS::setup

CityOS::~CityOS()
{
    delete _awsMqttClient;
}

void CityOS::loop()
{
    static unsigned long OledTimer = millis();
    static bool first = true;

    if (first){
        setup();
    }
    else{        
        wifiAndAwsReconnect();//if connection to wifi or aws is lost, try to reconnect
    }    
    
    
    if (sensing.active &&
      (first || millis() - OledTimer > (unsigned long)(sensing.interval * 1000)))
    {
        // update right away - no wait on first time
        first     = false;

        OledTimer = millis();

        interval();
        
        if (sensing.active)
        {
            publishData();
        }        
        
    }

    for (auto const& l:loops) {
        l->loop();
        yield();
    }

    // mqtt loop to called regularly to allow the client to process incoming messages and maintain its connection to the server.
    if (_awsMqttClient != nullptr && _awsMqttClient ->isConnected())
    {
        _awsMqttClient -> loop(5000);  
    }  
    if (debug.memory)
    {
        printHeapSize(); 
    }
    
    
} // CityOS::loop

void CityOS::interval()
{
    for (auto const& i:intervals) {
        i->interval();
        yield();
    }

    if (debug.senses)
        printSenses();   

    // memory leaks check
    if (debug.memory)
        printHeapSize();
}

String CityOS::getAllData(){
    DynamicJsonDocument jsonBuffer(512);
    //add senses
    for (auto const& sense : senses) {
        if (sense){
            if (senseValues[sense].isValid())
            {
                double value = senseValues[sense].value;            
                String key = senseValues[sense].shadowKey;   
                // String key = config[sense];   

                int si       = ceil(value);
                
                if (value == si) 
                {
                    jsonBuffer[key] = si;                
                }
                else 
                {                
                    jsonBuffer[key] = value;
                }
                senseValues[sense].oldValue = senseValues[sense].value;
            }          

        }
    }

    String result;
    result.reserve(512);
    serializeJson(jsonBuffer, result);

    return result;
}

String CityOS::getChangedData(){
    
    DynamicJsonDocument jsonBuffer(512);
    //add senses
    for (auto const& sense : senses) {
        if (sense){
                if (senseValues[sense].isValid() && senseValues[sense].isOverThreshold()){
                // String key = config[sense];
                String key = senseValues[sense].shadowKey;
                
                jsonBuffer[key] = senseValues[sense].value;    
                senseValues[sense].oldValue = senseValues[sense].value;            
            }
        }        
    }  
    
    if (!jsonBuffer.isNull())
    {
        String result;

        result.reserve(512);
        serializeJson(jsonBuffer, result);

        return result;
    }else
    {
        return "";
    }
    
}

String CityOS::getData(){
    //first time or in hourly interval
    if (timeStamp == 0 || timeStamp >= heartbeatSendTime )
    {
        timeStamp = ntpClient.getEpochTime();//change timestamp to new value
        heartbeatSendTime = timeStamp + sensing.heartbeat; // set up time for new hourly data sending

        return getAllData();
    }
    else
    {
        timeStamp = ntpClient.getEpochTime();

        return getChangedData();
    }
    
}

void CityOS::publishData(){

    if (WiFi.status() == WL_CONNECTED && _awsMqttClient != nullptr && _awsMqttClient -> isConnected())
    {  
        String data = getData();
        // if data was changed send it
        if(data != ""){
            _awsMqttClient -> publish(getTopic(), data, [&](const char* topic, const char* msg) {
                    handleMessages(topic, msg);                
            });
        } else{
            Serial << "Data was not changed over threshold. Nothing to send.";
        }              
    } 
    else
    {
        if (debug.errors || debug.wifi)
        {
            Serial << "Could not publish data!" << endl;
        }        
    }


}

void CityOS::handleMessages(const char* topic, const char* msg){

    Serial << F("Topic: ") << topic <<endl;
    Serial << F("Msg: ") << msg << endl;
}


void CityOS::printSenses()
{
    Serial << endl << F("SENSES | Data points: ") << endl;
    Serial << F(" - -  - -  - -  - -  ") << endl;
    int count = 1;
    for (auto const& sense : senses) {        

        Serial << count << ". " << sense<< " (" << senseValues[sense].shadowKey << ") : " ; 

        double value = senseValues[sense].value;
        int si       = ceil(value);
        if (value == si)
            Serial << si;
        else
            Serial << value;

        Serial << endl;
        count++;
    }
    Serial << " - -  - -  - -  - -  " << endl;
}
void CityOS::printSchema(){
    if (debug.senses)
    {
        int count = 1;
        if (senses.size() > 0)
        {
            Serial << "SENSE | Data points set: " << endl;
            Serial << F(" - -  - -  - -  - -  ") << endl;
            for (auto const& sense : senses) {
                Serial << "SENSE | " << count << ". " << sense << endl;
                count++;
            }
        } 
        Serial << F(" - -  - -  - -  - -  ") << endl;  
    }
}

void CityOS::printControls()
{
    Serial << endl << F("CONTROLS | Data points: ") << endl;
    Serial << F(" - -  - -  - -  - -  ") << endl;
    int count = 1;
    for (auto const& control : controls) {
        Serial << count << ". " << control << ": ";

        double value = controlValues[control];
        int si       = ceil(value);
        if (value == si)
            Serial << si;
        else
            Serial << value;

        Serial << endl;
        count++;
    }
    Serial << F(" - -  - -  - -  - -  ") << endl;
}

// Mem leaks check
void CityOS::printHeapSize()
{
    static int last_ram = 0;
    int ram = ESP.getFreeHeap();

    Serial << "RAM: " << ram << " [ change: " << (ram - last_ram) << " ]" << endl;
    last_ram = ram;
}

void CityOS::ntpConnect(){
    ntpClient.begin();
    while (!ntpClient.update())
    {        
        yield();
        ntpClient.forceUpdate();        
    }    
}

void CityOS::connectToWiFi(bool useWiFiManager){
       
    WiFi.mode(WIFI_STA);    

    if (useWiFiManager)
    {
        WiFiManager wifiManager;
        // wifiManager.resetSettings();
        wifiManager.setMinimumSignalQuality(10);
        wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT); // sets timeout until configuration portal gets turned off
        // wifiManager.setConnectTimeout(ESP_CONNECT_TIMEOUT); // how long to try to connect for before continuing
        wifiManager.setCaptivePortalEnable(false);

        if (debug.wifi) wifiManager.setDebugOutput(true);

        wifiManager.startConfigPortal(device.thing.c_str());        
    } 
    else 
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            WiFi.begin();
            // WiFi.begin(WiFi.SSID(), WiFi.psk()); 
            if (debug.wifi){
                Serial << "SSID="<<WiFi.SSID()<<" psk="<<WiFi.psk();
            }
            int wifi_retry_count = 1;
            int retry_on         = 1000;
            int retry_for        = 10;
            while (wifi_retry_count < retry_for && WiFi.status() != WL_CONNECTED) {
                yield();
                delay(retry_on);
                if (debug.wifi)
                    Serial << ".";

                // Make sure you notify if WIFI is not connecting
                // notify every 20 attempts
                if (debug.errors && (wifi_retry_count % 20 == 0 )) {
                    Serial
                        << "wifi ssid: " << WiFi.SSID() << " still NOT connected." << endl
                        << "Retring for: "
                        << (wifi_retry_count * retry_on) / 1000 << " seconds." << endl;
                }
                wifi_retry_count++;
                
            }
        }
    }        
    if (WiFi.status() != WL_CONNECTED)
    {
        setControl("wifi",0);
    }
    else
    {
        setControl("wifi",1);
    }
    if (debug.wifi) {
        if (WiFi.status() == WL_CONNECTED)
            Serial << "connected." << endl;
        else
            Serial << "giving up on connection for now." << endl;
    }    
}

void CityOS::wifiAndAwsReconnect(){
     //reconect to wifi    
    if (WiFi.status() != WL_CONNECTED)
    {
        if (debug.errors || debug.wifi)
        {
           Serial
                << "wifi ssid: " << WiFi.SSID() << " NOT connected." << endl;
        }        
        connectToWiFi(false);
    }
    //aws reconnect
    if (_awsMqttClient != nullptr && !_awsMqttClient ->isConnected() && WiFi.status() == WL_CONNECTED)
    {
        _awsMqttClient -> connect();
    }   
    if (WiFi.status() != WL_CONNECTED || _awsMqttClient == nullptr || !_awsMqttClient ->isConnected() )
    {
        setControl("wifi",0);
    }
    else
    {
        setControl("wifi",1);
    }
    
}

// Debug Info Function
void CityOS::printWifiStatus()
{
    //    Serial << "MAC address: " << getMacAddress() << endl;
    Serial << "IP Address: " << WiFi.localIP() << endl;
    Serial << "Subnet Mask: " << WiFi.subnetMask() << endl;
    Serial << "Gateway: " << WiFi.gatewayIP() << endl;

    Serial << "Connected to SSID: " << WiFi.SSID() << endl;
    Serial << "Wifi signal strength (RSSI):" << WiFi.RSSI() << " dBm" << endl;    
}

// Debug Info function
String CityOS::getMacHEX()
{
    byte mac[6];

    WiFi.macAddress(mac);

    String cMac = "";
    for (int i = 0; i < 6; ++i) {
        // if (mac[i] < 16)
        //    cMac += "0";
        cMac += String(mac[i], HEX);
        if (i < 5)
            cMac += ":";
    }
    // cMac.toUpperCase();
    return cMac;
}

// Used during setup()
int CityOS::sense(String sense)
{
    senses.push_back(sense);
    return senses.size();
}

// Used during setup()
int CityOS::control(String control)
{
    controls.push_back(control);
    return controls.size();
}

double CityOS::setSense(String sense, int value)
{
    return setSense(sense, (double) value);
}

double CityOS::setSense(String sense, long value)
{
    return setSense(sense, (double) value);
}

double CityOS::setSense(String sense, double value)
{
    std::map<String, SenseValue>::iterator it;

    // if value on this case was never set return 0
    double oldValue = 0.00;    
    
    // check for exiting data
    it = senseValues.find(sense);
    if (it != senseValues.end())
        oldValue = senseValues[sense].value;
    
    // round value to 2 decimal places
    double v = (int)(value * 100 + .5);
    senseValues[sense].value = (double) v / 100;
    
    return oldValue;
}

int CityOS::setControl(String control, int value)
{
    return setControl(control, (double) value);
}

double CityOS::setControl(String control, double value)
{
    std::map<String, double>::iterator it;

    // if value on this case was never set return 0
    double oldValue = 0;

    // check for exiting data
    it = controlValues.find(control);
    if (it != controlValues.end())
        oldValue = controlValues[control];

    controlValues[control] = value;
    return oldValue;
}

void CityOS::addToLoop(CityOS * c)
{
    loops.push_back(c);
}

void CityOS::addToInterval(CityOS * i)
{
    intervals.push_back(i);
}

void CityOS::addToSetup(CityOS * i)
{
    setups.push_back(i);
}

String CityOS::getTopic(){
    return  "dt/air/" + device.location +"/" + device.thing; 
}
