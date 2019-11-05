#include <CityOS.h>

std::vector<String> CityOS::senses;
std::vector<String> CityOS::controls;

std::map<String, double> CityOS::senseValues;
std::map<String, double> CityOS::controlValues;

std::vector<CityOS *> CityOS::loops;
std::vector<CityOS *> CityOS::intervals;

CityOS::CityOS()
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

    webserver.active = false;
    webserver.port   = 80;

    wifi.ssid = WIFI_SSID;
    wifi.pass = WIFI_PASSWORD;

    api.token = CTOS_TOKEN;

    _server = new WiFiServer(webserver.port);
}

void CityOS::setup()
{
    // Turn off AP mode for ESP8266
    WiFi.softAPdisconnect(true);
    // Start Serials
    Serial.begin(115200); // serial terminal

    // Connect to WiFi network
    if (debug.wifi) Serial
            << "WIFI: Connecting to ssid:" << wifi.ssid << " wireless net." << endl;


    WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());

    int wifi_retry_count = 1;
    int retry_on         = 1000;
    int retry_for        = 10;
    while (wifi_retry_count < 10 && WiFi.status() != WL_CONNECTED) {
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

    delay(500); // WAIT FOR SERIAL

    if (debug.wifi || debug.webserver)
        printWifiStatus();

    api.deviceID = getMacHEX();

    Serial << "ID: " << api.deviceID << endl;

    // Send Schema to server
    if (api.active)
        sendSchema();
} // CityOS::setup

CityOS::~CityOS()
{
    delete _server;
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

    if (webserver.active)
        serveHTML();
} // CityOS::loop

void CityOS::interval()
{
    for (auto const& i:intervals) {
        i->interval();
        yield();
    }

    if (debug.senses)
        printSenses();

    if (api.active)
        sendSenses();

    // memory leaks check
    if (debug.memory)
        printHeapSize();
}

void CityOS::sendSchema()
{
    String json = "{\n";

    if (senses.size() > 0) {
        json += "  \"sense\" : {\n";

        if (debug.senses)
            Serial << "SENSE | Data points set: " << endl;
        int count = 1;

        for (auto const& sense : senses) {
            if (debug.senses)
                Serial << "SENSE | " << count << ". " << sense << endl;

            if (count > 1)
                json += ",\n";

            json += "    \"";
            json += count;
            json += "\" : \"" + sense + "\"";
            count++;
        }

        json += "  }";
    }
    if ((senses.size() > 0) && (controls.size() > 0))
        json += ",\n";

    if (controls.size() > 0) {
        json += "  \"control\" : {\n";

        if (debug.controls)
            Serial << "CONTROL | Data points set: " << endl;
        int count = 1;

        for (auto const& control : controls) {
            if (debug.controls)
                Serial << "CONTROL | " << count << ". " << control << endl;

            if (count > 1)
                json += ",\n";

            json += "    \"";
            json += count;
            json += "\" : \"" + control + "\"";
            count++;
        }

        json += "\n  }\n";
    }

    json += "}\n";

    if (debug.schema) Serial
            << "JSON| Schema: " << endl << json.c_str() << endl;

    rest("POST", "/device/" + api.deviceID + "/schema", json);
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

                for (auto const& sv : senseValues)
                    client << "<p>" << sv.first << ": " << sv.second << endl;

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

void CityOS::sendSenses()
{
    String json = "";

    json += "{";

    int count = 1;
    for (auto const& sense : senses) {
        if (count > 1)
            json += ", ";

        json += "\"";
        json += count;
        json += "\" : ";

        double value = senseValues[sense];
        // Optimize traffic -- striping zeros on doubles castable to integer
        int si = ceil(value);
        if (value == si)
            json += si;
        else
            json += value;

        count++;
    }

    json += "}";

    if (debug.json) Serial << json.c_str() << endl;

    rest("POST", "/device/" + api.deviceID + "/readings", json);
}

void CityOS::rest(String method, String url, String json)
{
    String hostPort = api.host + ":" + api.port;

    if (_client.connected()) {
        if (debug.api) Serial
                << "API | Connection to: " << hostPort << "  Reused" << endl;
    }   else if (!_client.connect(api.host.c_str(), api.port)) {
        if (debug.errors) Serial
                << "API | Connection to: " << hostPort << "  Failed" << endl;
        return;
    }

    if (debug.api) Serial
            << "API | connection to: " << hostPort << "  successful" << endl;

    if (debug.api) {
        Serial
            << "API | " << method << " to http://" << hostPort << url << endl;
    }

    if (debug.api) Serial
            << "API | Sending json:" << endl << json.c_str() << endl;

    _client << method << " " << url << " HTTP/1.1" << endl;
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

void CityOS::printSenses()
{
    Serial << endl << "SENSES | Data points: " << endl;
    Serial << " - -  - -  - -  - -  " << endl;
    int count = 1;
    for (auto const& sense : senses) {
        Serial << count << ". " << sense << ": ";

        double value = senseValues[sense];
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
    Serial << endl << "CONTROLS | Data points: " << endl;
    Serial << " - -  - -  - -  - -  " << endl;
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
    std::map<String, double>::iterator it;

    // if value on this case was never set return 0
    double oldValue = 0;

    // check for exiting data
    it = senseValues.find(sense);
    if (it != senseValues.end())
        oldValue = senseValues[sense];

    senseValues[sense] = value;
    // Serial << "oldValue: " << oldValue << endl;
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
