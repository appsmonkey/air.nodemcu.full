#include <CityOS.h>

std::vector<String> CityOS::senses;
std::vector<String> CityOS::controls;

std::map<String, _SENSEVALUE> CityOS::senseValues;
std::map<String, double> CityOS::controlValues;

std::vector<CityOS *> CityOS::loops;
std::vector<CityOS *> CityOS::intervals;

std::map<String, String> CityOS::config;

CityOS::CityOS():
ntpClient(wifiUDP, "pool.ntp.org")
{
    // Print errors to Serial
    debug.errors = true;

    api.active   = true;
    api.host     = "ctos.io";
    api.port     = 80;
    api.deviceID = "00:00:00:00:00:00";
    api.timeout  = 10;

    sensing.active   = true;
    sensing.interval = 60;

    // make api.interval 30+ sec for production - do not go much lower here
    // each client.connect() eats 184 bytes at a time
    // and returns it in few minutes as they timeout

    wifi.ssid = WIFI_SSID;
    wifi.pass = WIFI_PASSWORD;

    // api.token = CTOS_TOKEN;  

    if(loadConfig()){
        _awsMqttClient = new AwsMqttClient(config["mqtt_host"], config["mqtt_port"].toInt(), config["thing"],
                                           config["amazon_ca"], config["device_ca"], config["root_ca"],
                                           config["private_key"]);
        device.thing = config["thing"];
        device.location = config["location"]                ;                    
    } else{
        _awsMqttClient = nullptr;
        if(debug.config)
                    Serial << F("Failed to load config.") << endl;
    }
   
}

void CityOS::setup()
{
    // Start Serials
    Serial.begin(115200); // serial terminal
    
    // Connect to WiFi network
    if (debug.wifi) Serial
            << "WIFI: Connecting to ssid:" << wifi.ssid << " wireless net." << endl;    
    
    
    WiFiManager wifiManager;
    
    wifiManager.autoConnect(device.thing.c_str());
    
    if (debug.wifi) {
        if (WiFi.status() == WL_CONNECTED)
            Serial << "connected." << endl;
        else
            Serial << "giving up on connection for now." << endl;
    }    

    //if connected to wifi setup time, and aws
    if (WiFi.status() == WL_CONNECTED){
        //set up time      
        ntpConnect();
        if (_awsMqttClient !=nullptr)
        {
            _awsMqttClient -> setupAwsJitp(ntpClient);
            _awsMqttClient -> connect();
        }               
    }
       

    if (debug.wifi || debug.webserver)
        printWifiStatus();

    api.deviceID = getMacHEX();

    Serial << "ID: " << api.deviceID << endl;
    
} // CityOS::setup

CityOS::~CityOS()
{
    delete _awsMqttClient;
}

void CityOS::loop()
{
    static unsigned long OledTimer = millis();
    static bool first = true;

    if (first)
        setup();

    if (sensing.active &&
      (first || millis() - OledTimer > (sensing.interval * 1000)))
    {
        // update right away - no wait on first time
        first     = false;
        OledTimer = millis();
        interval();
    }

    for (auto const& l:loops) {
        l->loop();
        yield();
    }

    // mqtt loop to called regularly to allow the client to process incoming messages and maintain its connection to the server.
    _awsMqttClient -> loop(5000);        

} // CityOS::loop


void CityOS::interval()
{
    for (auto const& i:intervals) {
        i->interval();
        yield();
    }

    printSenses();      
    
    if (WiFi.status() == WL_CONNECTED && _awsMqttClient != nullptr && _awsMqttClient -> isConnected())
    {        
        _awsMqttClient -> publish(getTopic(), getSchema(), [&](const char* topic, const char* msg) {
                handleMessages(topic, msg);                
        });
        
    }  
    timeStamp=ntpClient.getEpochTime();
    // memory leaks check
    if (debug.memory)
        printHeapSize();
}

String CityOS::getAllData(){
    DynamicJsonDocument jsonBuffer(512);
    //add senses
    for (auto const& sense : senses) {
        if (sense){
            double value = senseValues[sense].value;
            int si       = ceil(value);
            if (value == si) 
            {
                jsonBuffer[sense] = si;                
            }
            else 
            {
                jsonBuffer[sense] = value;
            }
        }
    }

    String result;
    result.reserve(512);
    serializeJson(jsonBuffer, result);

    return result;
}

String CityOS::getChangedData(){
    double epsilon = 0.001;
    DynamicJsonDocument jsonBuffer(512);
    //add senses
    for (auto const& sense : senses) {
        if (sense){  
            if (std::abs(senseValues[sense].value-senseValues[sense].oldValue) > epsilon){
                jsonBuffer[sense] = senseValues[sense].value;
            }
        }
    }

    String result;
    result.reserve(512);
    serializeJson(jsonBuffer, result);

    return result;
}

String CityOS::getSchema(){
    //first time or interval is bigger then sensing interval
    unsigned long oldTimeStamp = timeStamp;
    if (timeStamp == 0 || (timeStamp - oldTimeStamp)>= (sensing.interval + 10)) {
        return getAllData();
    }else{
        return getChangedData();
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
        Serial << count << ". " << sense << ": ";

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
    Serial << " - -  - -  - -  - -  " << endl;
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

// Debug Info Function
void CityOS::printWifiStatus()
{
    //    Serial << "MAC address: " << getMacAddress() << endl;
    Serial << "IP Address: " << WiFi.localIP() << endl;
    Serial << "Subnet Mask: " << WiFi.subnetMask() << endl;
    Serial << "Gateway: " << WiFi.gatewayIP() << endl;

    Serial << "Connected to SSID: " << WiFi.SSID() << endl;
    Serial << "Wifi signal strength (RSSI):" << WiFi.RSSI() << " dBm" << endl;

    if (debug.webserver)
        Serial << "My Server: http://" << WiFi.localIP() << ":" << webserver.port << endl;
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
    std::map<String, _SENSEVALUE>::iterator it;

    // if value on this case was never set return 0
    double oldValue = 0.00;

    // check for exiting data
    it = senseValues.find(sense);
    if (it != senseValues.end())
        oldValue = senseValues[sense].value;
    
    senseValues[sense].oldValue = oldValue;
    senseValues[sense].value = value;
    
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

bool CityOS::loadConfig(){
    if(SPIFFS.begin()){
        if(debug.config)
            Serial << F("SPIFFS mounted file system --- Loading config") << endl;
        if(SPIFFS.exists(F("/config.json"))){
            File configFile=SPIFFS.open(F("/config.json"), "r");
            if(configFile){
                size_t size=configFile.size();

                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);

                DynamicJsonDocument json(JSON_CAPACITY);
                auto error=deserializeJson(json, buf.get());                
                if (!error) {
                    
                    JsonObject obj = json.as<JsonObject>();
                    for (JsonPair p : obj) {
                         String key(p.key().c_str());                        
                        config[key]=p.value().as<String>();
                    }             
                }else
                {
                    if(debug.config)
                        Serial << F("Failed to load json config.") << endl;
                    return false;  
                } 
            }
        } else{
            if(debug.config)
                Serial << F("Failed to load config.json.") << endl;
            return false;    
        }   
    }else{
        if(debug.config)
            Serial << F("Failed to mount file system.") << endl;
        return false;
    }
    return true;
}

String CityOS::getTopic(){
    return  "dt/air/" + device.location +"/" + device.thing; 
}
