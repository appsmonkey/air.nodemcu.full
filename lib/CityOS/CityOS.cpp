#include <CityOS.h>

int CityOS::inputCount;
int CityOS::outputCount;

std::map<int, String> CityOS::inputs;
std::map<int, String> CityOS::outputs;
std::map<int, float> CityOS::values;
std::vector<CityOS*> CityOS::sensors;


CityOS::CityOS()
{
    debug.errors = true;
    // debug.readings = true;
    // debug.led      = true;
    // debug.wifi = true;
    // debug.api       = true;
    // debug.json = true;
    // debug.webserver = true;
    // debug.memory = true;

    api.active   = true;
    api.host     = "ctos.io";
    api.port     = 80;
    api.deviceID = 0;
    api.timeout  = 10;

    readings.active   = true;
    readings.interval = 30;
    // make api.interval 30+ sec - do not go much lower here
    // each client.connect() eats 184 bytes at a time
    // and returns it in few minutes as they timeout

    webserver.active = true;
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
    sendSchema();
}

void CityOS::loop()
{ }

void CityOS::sendSchema()
{
    String json = "";

    json += "{";

    bool first = true;

    for (auto const& s : inputs) {
        if (!first)
            json += ", ";

        json += "\"";
        json += s.first;
        json += "\" : ";

        json += "\"";
        json += s.second;
        json += "\"";

        first = false;
    }

    json += "}";

    if (debug.json) Serial << json.c_str() << endl;
}

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

                for (auto const& r : values)
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

    bool first = true;

    for (auto const& r : values) {
        if (!first)
            json += ", ";

        json += "\"";
        json += r.first;
        json += "\" : ";

        // Optimize traffic -- striping zeros on doubles castable to integer
        int si = ceilf(r.second);
        if (r.second == si)
            json += si;
        else
            json += r.second;

        first = false;
    }

    json += "}";

    if (debug.json) Serial << json.c_str() << endl;

    // try to reuse connection to save memory

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

    // DO not disconnect - will try to reuse connection to save some memory
    // _client.stop();

    if (debug.api) {
        Serial
            << "API: connection to: " << hostPort << "closed." << endl;
    }
} // CityOS::sendData

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
    inputCount++;
    inputs[inputCount] = type;
    return outputCount;
}

// Used during setup()
int CityOS::output(String type)
{
    outputCount++;
    outputs[inputCount] = type;
    return outputCount;
}

float CityOS::setValue(int position, float newValue)
{
    std::map<int, float>::iterator it;

    // if value on this case was never set return 0
    float oldValue = 0;

    // check for exiting data
    it = values.find(position);
    if (it != values.end())
        oldValue = values[position];

    values[position] = newValue;
    return oldValue;
}

int CityOS::sensor(CityOS * sensor)
{
    sensors.push_back(sensor);
}
