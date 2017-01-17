#include <Air.h>

// Constructor config
Air::Air()
{
    pin.dht = D1;

    pin.red   = D2;
    pin.green = D5;
    pin.blue  = D6;

    pin.pmRX = D7;
    pin.pmTX = D8;

    range.pm2_5 = 0;
    range.pm10  = 0;
    range.worst = 0;

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
    addSensor("temperature_air");
    addSensor("humidity_air");
    addSensor("temperature_air_feels-like");
    // addSensor("humidity_soil_1");

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

    delay(5000); // WAIT FOR SERIAL

    if (debug.wifi || debug.webserver)
        printWifiStatus();


    CityOS::setup();
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

        readPM();
        setWorstRange();
        setLedColors();


        if (api.active)
            sendData();

        // memory leaks check
        if (debug.memory)
            printHeapSize();
    }

    yield();

    if (webserver.active)
        serveHTML();


    // yield() calls on the background functions to allow them to
    // keep WiFi connected, manage the TCP/IP stack, etc
    yield();
    CityOS::setup();
} /* cos_loop */

char Air::checkValue(unsigned char * thebuf, char leng)
{
    char receiveflag = 0;
    int receiveSum   = 0;

    for (int i = 0; i < (leng - 2); i++) {
        receiveSum = receiveSum + thebuf[i];
    }
    receiveSum = receiveSum + 0x42;

    // check the serial data
    if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) {
        receiveSum  = 0;
        receiveflag = 1;
    }
    return receiveflag;
}

int Air::read16Bits(unsigned char * thebuf, int offset)
{
    unsigned int PMVal;

    PMVal = ((thebuf[offset] << 8) + thebuf[offset + 1]);
    return PMVal;
}

void Air::readPM()
{
    SoftwareSerial * _swSer;

    _swSer = new SoftwareSerial(pin.pmRX, pin.pmTX);
    _swSer->begin(9600); // PM Serial
    // set the Timeout to 1500ms
    // longer than the data transmission periodic time of the sensor
    // which is 1000ms
    _swSer->setTimeout(1500);

    // start to read when detect 0x42 (PM)
    if (!_swSer->find(0x42))
        return;

    unsigned int LENG = 31;
    unsigned char buf[LENG]; // 0x42 + 31 bytes equal to 32 bytes

    _swSer->readBytes(buf, LENG);
    delete _swSer;

    if (buf[0] == 0x4d) {
        if (checkValue(buf, LENG)) {
            // count PM1.0 value of the air detector module
            sensor.pm1 = read16Bits(buf, 3);
            if (debug.readings) Serial << "READINGS PM1: " << sensor.pm1 << endl;
            setValue(4, sensor.pm1);

            // count PM2.5 value of the air detector module
            sensor.pm2_5 = read16Bits(buf, 5);
            setPM2_5Range();
            if (debug.readings) Serial << "READINGS PM2.5: " << sensor.pm2_5 << endl;
            setValue(5, sensor.pm2_5);

            // count PM10 value of the air detector module
            sensor.pm10 = read16Bits(buf, 7);
            setPM10Range();
            setValue(6, sensor.pm10);

            if (debug.readings) Serial << "READINGS PM10: " << sensor.pm10 << endl;
        }
    }
} // Air::readPM

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

// 0-5
int Air::setPM2_5Range()
{
    // USA
    int ranges[5] = { 13, 36, 56, 151, 251 };

    // Asia
    // int ranges[5] = { 30, 60, 90, 120, 250 };

    for (int i = 0; i < sizeof(ranges); i++) {
        if (sensor.pm2_5 < ranges[i]) {
            range.pm2_5 = i;
            return i;
        }
    }
    range.pm2_5 = sizeof(ranges);
    return range.pm2_5;
}

int Air::setPM10Range()
{
    // USA
    int ranges[5] = { 55, 155, 255, 355, 425 };

    // Asia
    // int ranges[5] = { 50, 100, 250, 350, 430 };

    for (int i = 0; i < sizeof(ranges); i++) {
        if (sensor.pm10 <

          ranges[i])
        {
            range.pm10 = i;
            return i;
        }
    }

    range.pm10 = sizeof(ranges);
    return range.pm10;
}

int Air::setWorstRange()
{
    range.worst = range.pm2_5 > range.pm10 ? range.pm2_5 : range.pm10;

    if (debug.led) {
        Serial
            << "LED: PM2.5 value: "
            << sensor.pm2_5 << " (range: " << range.pm2_5 << ")" << endl
            << "LED: PM10 value: "
            << sensor.pm10 << " (range: " << range.pm10 << ")" << endl
            << "LED: worst range: "
            << range.worst << endl;
    }

    return range.worst;
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

    if (range.worst >= sizeof(colors))
        range.worst = sizeof(colors) - 1;

    int red   = map(colors[range.worst][0], 0, 255, 0, 1023);
    int green = map(colors[range.worst][1], 0, 255, 0, 1023);
    int blue  = map(colors[range.worst][2], 0, 255, 0, 1023);

    if (debug.led) {
        Serial
            << "LED INFO: Printing for range: " << range.worst << endl
            << "red: " << red << "[" << colors[range.worst][0] << "]"
            << " green: " << green << "[" << colors[range.worst][1] << "]"
            << " blue: " << blue << "[" << colors[range.worst][2] << "]" << endl;
    }

    analogWrite(pin.red, red);
    analogWrite(pin.green, green);
    analogWrite(pin.blue, blue);

    last_range = range.worst;
} // Air::setLedColors
