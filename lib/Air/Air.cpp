#include <Air.h>

// Constructor config
Air::Air()
{
    pin.dht = D1;

    pin.red   = D2;
    pin.green = D5;
    pin.blue  = D6;

    pinMode(pin.red, OUTPUT);
    pinMode(pin.green, OUTPUT);
    pinMode(pin.blue, OUTPUT);

    _dht = new DHT(pin.dht, DHT22);
}

Air::~Air()
{
    delete _dht;
}

void Air::setup()
{
    // Start Serials
    Serial.begin(115200); // serial terminal

    // RGB LED
    pinMode(pin.red, OUTPUT);
    pinMode(pin.green, OUTPUT);
    pinMode(pin.blue, OUTPUT);

    delay(10);
    _dht->begin();

    // Connect to WiFi network
    if (debug.wifi) Serial
            << "WIFI: Connecting to ssid:" << wifi.ssid << " wireless net." << endl;


    WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());

    int wifi_retry_count = 1;
    int retry_on         = 500;
    while (WiFi.status() != WL_CONNECTED) {
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

        api.deviceID = getMacINT();
    }

    if (debug.wifi && (wifi_retry_count > 1)) Serial
            << "connected." << endl;

    // Starting the web server
    if (debug.webserver) Serial
            << "WEBSERVER: Started on port: " << webserver.port << endl;
    _server->begin();

    delay(1000); // WAIT FOR SERIAL

    if (debug.wifi || debug.webserver)
        printWifiStatus();

    for (auto const& s:sensors) {
        s->setup();
    }
} /* cos_setup */

void Air::loop()
{
    static unsigned long OledTimer = millis();
    static bool first = true;

    if (readings.active &&
      (first || millis() - OledTimer > (readings.interval * 1000)))
    {
        // update right away - no wait on first time
        first     = false;
        OledTimer = millis();

        readDHT();

        setLedColors();


        if (api.active)
            sendData();

        // memory leaks check
        if (debug.memory)
            printHeapSize();

        for (auto const& s:sensors) {
            s->loop();
            // yield() calls on the background functions to allow them to
            // keep WiFi connected, manage the TCP/IP stack, etc
            yield();
        }
    }

    // if (webserver.active)
    //  serveHTML();
} /* cos_loop */

void Air::readDHT()
{
    // Read temperature as Celsius (the default)
    sensor.temperature = _dht->readTemperature();
    if (debug.readings) Serial << "READINGS Temperature: " << sensor.temperature << endl;
    setValue(1, sensor.temperature);

    // Sensor readings may be up to 2 seconds 'old' (its a slow sensor)
    sensor.humidity = _dht->readHumidity();
    if (debug.readings) Serial << "READINGS Humidity: " << sensor.humidity << endl;
    setValue(2, sensor.humidity);

    // Check if any reads failed and exit early (to try again).
    if (isnan(sensor.humidity) || isnan(sensor.temperature)) {
        if (debug.errors) Serial
                << "ERROR: Failed to read from DHT sensor (humidity & temperature set to 0)!" << endl;
    } else {
        // Computes temperature values in Celsius and Humidity
        sensor.feelsLike = _dht->computeHeatIndex(sensor.temperature, sensor.humidity, false);
        if (debug.readings) Serial << "READINGS Feels Like: " << sensor.feelsLike << endl;
        setValue(3, sensor.feelsLike);
    }
}

void Air::setLedColors()
{
    static int last_range = -1;

    // no need to update if same as last
    // if (range.worst == last_range)
    // return;

    int colors[6][3] = {
        {    1, 200, 255 }, // blue
        {  175, 255,   1 }, // green
        {  245,  90,   1 }, // yellow
        {  255,  40,   1 }, // orange
        {  255,   1, 170 }, // purple
        {  255,   1,   1 }  // red
    };

    if (values[7] >= sizeof(colors))
        values[7] = sizeof(colors) - 1;

    int current_range = values[7];
    int red   = map(colors[current_range][0], 0, 255, 0, 1023);
    int green = map(colors[current_range][1], 0, 255, 0, 1023);
    int blue  = map(colors[current_range][2], 0, 255, 0, 1023);

    if (debug.led) {
        Serial
            << "LED INFO: Printing for range: " << current_range << endl
            << "red: " << red << "[" << colors[current_range][0] << "]"
            << " green: " << green << "[" << colors[current_range][1] << "]"
            << " blue: " << blue << "[" << colors[current_range][2] << "]" << endl;
    }

    analogWrite(pin.red, red);
    analogWrite(pin.green, green);
    analogWrite(pin.blue, blue);

    last_range = current_range;
} // Air::setLedColors
