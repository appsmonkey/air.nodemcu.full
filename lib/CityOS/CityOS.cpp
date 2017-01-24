#include <CityOS.h>


std::vector<String> CityOS::inputs;
std::vector<String> CityOS::outputs;

std::map<String, float> CityOS::inputValues;
std::map<String, float> CityOS::outputValues;

std::vector<CityOS *> CityOS::senses;
std::vector<CityOS *> CityOS::controls;

CityOS::CityOS()
{
    debug.errors = true;

    api.active   = true;
    api.host     = "ctos.io";
    api.port     = 80;
    api.deviceID = 0;
    api.timeout  = 10;

    readings.active   = true;
    readings.interval = 60;
    // make api.interval 30+ sec for production - do not go much lower here
    // each client.connect() eats 184 bytes at a time
    // and returns it in few minutes as they timeout

    webserver.active = false;
    webserver.port   = 80;

    wifi.ssid = WIFI_SSID;
    wifi.pass = WIFI_PASSWORD;

    api.token = CTOS_TOKEN;

    _server = new WiFiServer(webserver.port);
}

CityOS::~CityOS()
{
    delete _server;
}

void CityOS::setup()
{
    // Start Serials
    Serial.begin(115200); // serial terminal

    // Connect to WiFi network
    if (debug.wifi) Serial
            << "WIFI: Connecting to ssid:" << wifi.ssid << " wireless net." << endl;


    WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());

    int wifi_retry_count = 1;
    int retry_on         = 500;
    while (wifi_retry_count < 5 && WiFi.status() != WL_CONNECTED) {
        yield();
        delay(retry_on);
        if (debug.wifi)
            Serial << ".";

        // Make sure you notify if WIFI is not connecting
        // notify every 20 attempts
        if (debug.errors && (wifi_retry_count % 20 == 0 )) {
            Serial
                << "wifi ssid: " << wifi.ssid << " still NOT connected." << endl
                << "Retring for: "
                << (wifi_retry_count * retry_on) / 1000 << " seconds." << endl;
        }
        wifi_retry_count++;
    }

    if (debug.wifi) {
        if (WiFi.status() == WL_CONNECTED)
            Serial << "connected." << endl;
        else
            Serial << "giving up on connection for now." << endl;
    }
    // Starting the web server
    if (debug.webserver) Serial
            << "WEBSERVER: Started on port: " << webserver.port << endl;
    _server->begin();

    delay(1000); // WAIT FOR SERIAL

    if (debug.wifi || debug.webserver)
        printWifiStatus();

    api.deviceID = getMacINT();

    for (auto const& s:senses) {
        s->setup();
        yield();
    }

    for (auto const& c:controls) {
        c->setup();
        yield();
    }

    // Send Schema to server
    if (api.active)
        sendSchema();
} // CityOS::setup

void CityOS::loop()
{
    static unsigned long OledTimer = millis();
    static bool first = true;

    if (readings.active &&
      (first || millis() - OledTimer > (readings.interval * 1000)))
    {
        // update right away - no wait on first time
        first     = false;
        OledTimer = millis();

        for (auto const& s:senses) {
            s->loop();
            yield();
        }

        for (auto const& c:controls) {
            c->loop();
            yield();
        }

        if (debug.inputs)
            printInputValues();

        if (debug.outputs)
            printOutputValues();

        if (api.active)
            sendData();

        // memory leaks check
        if (debug.memory)
            printHeapSize();
    }

    if (webserver.active)
        serveHTML();
} // CityOS::loop

void CityOS::sendSchema()
{
    String json = "";

    json += "{\n";

    int count = 1;
    for (auto const& input : inputs) {
        if (count > 1)
            json += ",\n";

        json += "\"";
        json += count;
        json += "\" : ";

        json += "\"";
        json += input;
        json += "\"";
        count++;
    }

    json += "\n}\n";

    if (debug.json) Serial
            << "JSON| Schema: " << json.c_str() << endl;

    if (debug.inputs) {
        Serial << "INPUTS | Data points set: " << endl;
        count = 1;
        for (auto const& input : inputs) {
            Serial << "INPUTS | " << count << ". " << input << endl;
            count++;
        }
    }
} // CityOS::sendSchema

void CityOS::serveHTML()
{
    // Listenning for new clients
    WiFiClient client = _server->available();

    if (!client)
        return;

    if (debug.webserver) Serial
            << "WEBSERVER: New client connection from: " << client.remoteIP() << ":" << client.remotePort() << endl;

    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
        if (client.available()) {
            const char * nl = "\n";

            char c = client.read();

            if (c == '\n' && blank_line) {
                client << "HTTP/1.1 200 OK" << nl;
                client << "Content-Type: text/html" << nl;
                client << "Connection: close" << nl << nl;

                client << HTMLHead();

                client << "<h1>Sensors</h1>" << nl;

                for (auto const& r : inputValues)
                    client << "<p>" << r.first << ": " << r.second << endl;

                client << HTMLFoot();
                break;
            }
            if (c == '\n') {
                // when starts reading a new line
                blank_line = true;
            } else if (c != '\r') {
                // when finds a character on the current line
                blank_line = false;
            }
        }
    }
    // closing the client connection
    delay(1);
    client.stop();
    if (debug.webserver) Serial
            << "WEBSERVER: Client disconnected." << endl;
} // CityOS::serveHTML

void CityOS::sendData()
{
    String json = "";

    json += "{";

    int count = 1;
    for (auto const& input : inputs) {
        if (count > 1)
            json += ", ";

        json += "\"";
        json += count;
        json += "\" : ";

        float value = inputValues[input];
        // Optimize traffic -- striping zeros on doubles castable to integer
        int si = ceilf(value);
        if (value == si)
            json += si;
        else
            json += value;

        count++;
    }

    json += "}";

    if (debug.json) Serial << json.c_str() << endl;

    String hostPort = api.host + ":" + api.port;

    if (_client.connected()) {
        if (debug.api) Serial
                << "API connection to: " << hostPort << "  Reused" << endl;
    }   else if (!_client.connect(api.host.c_str(), api.port)) {
        if (debug.errors) Serial
                << "API connection to: " << hostPort << "  Failed" << endl;
        return;
    }

    if (debug.api) Serial
            << "API connection to: " << hostPort << "  successful" << endl;

    if (debug.api) Serial
            << "API: Sending json:" << endl << json.c_str() << endl;

    if (debug.api) {
        Serial
            << "API: http://" << hostPort << "/device/"
            << api.deviceID << "/readings" << endl;
    }

    _client << "POST /device/" << api.deviceID << "/readings HTTP/1.1" << endl;
    _client << "Host: " << api.host.c_str() << endl;
    _client << "Authorization: Bearer " << api.token.c_str() << endl;
    _client << "Content-Type: application/json" << endl;
    _client << "Content-Length: ";
    _client << json.length() << endl;
    _client << endl;

    _client << json.c_str() << endl;

    unsigned long timeout = millis();
    while (_client.available() == 0) {
        if (millis() - timeout > (api.timeout * 1000)) {
            if (debug.errors) {
                Serial
                    << "ERROR: Timeout API connection to: "
                    << hostPort << "/device/" << api.deviceID << "/readings"
                    << " waited for: " << api.timeout
                    << "seconds before giving up" << endl;
            }
            _client.stop();
            return;
        }
    }

    if (debug.api) Serial
            << "API: Response from: " << hostPort << endl << endl;

    // Read all the lines of the reply from server and print them to Serial
    while (_client.available()) {
        String line = _client.readStringUntil('\r');

        if (debug.api)
            Serial << line;
    }

    if (debug.api)
        Serial << endl;

    // DO not disconnect - reuse connection to save some memory
    // _client.stop();

    if (debug.api) {
        Serial
            << "API: connection to: " << hostPort << "closed." << endl;
    }
} // CityOS::sendData

void CityOS::printInputValues()
{
    Serial << endl << "INPUTS | Data points: " << endl;
    Serial << " - -  - -  - -  - -  " << endl;
    int count = 1;
    for (auto const& input : inputs) {
        Serial << count << ". " << input << ": ";

        float value = inputValues[input];
        int si      = ceilf(value);
        if (value == si)
            Serial << si;
        else
            Serial << value;

        Serial << endl;
        count++;
    }
    Serial << " - -  - -  - -  - -  " << endl;
}

void CityOS::printOutputValues()
{
    Serial << endl << "OUTPUTS | Data points: " << endl;
    Serial << " - -  - -  - -  - -  " << endl;
    int count = 1;
    for (auto const& output : outputs) {
        Serial << count << ". " << output << ": ";

        float value = outputValues[output];
        int si      = ceilf(value);
        if (value == si)
            Serial << si;
        else
            Serial << value;

        Serial << endl;
        count++;
    }
    Serial << " - -  - -  - -  - -  " << endl;
}

const char * CityOS::HTMLHead()
{
    std::string page;

    page  = "<!DOCTYPE HTML>";
    page += "<html>";
    page += "<head></head>";
    page += "<body>";

    return page.c_str();
}

const char * CityOS::HTMLFoot()
{
    std::string page;

    page = "</body></html>";

    return page.c_str();
}

// Mem leaks check
void CityOS::printHeapSize()
{
    static int last_ram = 0;
    int ram = ESP.getFreeHeap();

    Serial << "RAM: " << ram << " [ change: " << (ram - last_ram) << " ]" << endl;
    last_ram = ram;
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
        cMac += String(mac[i], HEX);
        if (i < 5)
            cMac += "-";
    }
    cMac.toUpperCase();
    return cMac;
}

// Debug Info function
int CityOS::getMacINT()
{
    static int macINT = 0;

    if (macINT)
        return macINT;

    byte mac[6];

    WiFi.macAddress(mac);

    String cMac = "";
    for (int i = 0; i < 6; ++i)
        cMac += String(mac[i]);

    macINT = atoi(cMac.c_str());

    return macINT;
}

// Used during setup()
int CityOS::input(String type)
{
    inputs.push_back(type);
    return inputs.size();
}

// Used during setup()
int CityOS::output(String type)
{
    outputs.push_back(type);
    return outputs.size();
}

int CityOS::setInputValue(String type, int newValue)
{
    return setInputValue(type, (float) newValue);
}

float CityOS::setInputValue(String type, float newValue)
{
    std::map<String, float>::iterator it;

    // if value on this case was never set return 0
    float oldValue = 0;

    // check for exiting data
    it = inputValues.find(type);
    if (it != inputValues.end())
        oldValue = inputValues[type];

    inputValues[type] = newValue;
    return oldValue;
}

int CityOS::setOutputValue(String type, int newValue)
{
    return setOutputValue(type, (float) newValue);
}

float CityOS::setOutputValue(String type, float newValue)
{
    std::map<String, float>::iterator it;

    // if value on this case was never set return 0
    float oldValue = 0;

    // check for exiting data
    it = outputValues.find(type);
    if (it != inputValues.end())
        oldValue = outputValues[type];

    outputValues[type] = newValue;
    return oldValue;
}

int CityOS::sense(CityOS * sensor)
{
    senses.push_back(sensor);
}

int CityOS::control(CityOS * control)
{
    controls.push_back(control);
}
