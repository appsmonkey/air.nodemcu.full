#include "DS_3231.h"

// DS3231Time = 1/1/2000
// DS3231Time + 946684800 = UnixTime
// UnixTime – 946684800 = DS3231Time

const uint8_t daysArray [] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

DS_3231::DS_3231(int scl, int sda)
{
    localPort     = 2390; // local port to listen for UDP packets
    ntpServerName = "time.nist.gov";
    // ntpServerName = "tik.cesnet.cz";

    udp.begin(localPort);

    state = STATE_SEND;

    Wire.begin(sda, scl);

    ntpUpdate();

    sense("time unixtime");

    addToInterval(this);
}

void DS_3231::interval()
{
    ntpUpdate();
    readClockTime();
    setSense("time unixtime", (long) now());
} // DS_3231::interval

void DS_3231::ntpUpdate()
{
    static int lastUpdate = 0;

    if (WiFi.status() != WL_CONNECTED) {
        if (debug.errors) Serial
                << "NTP UPDATE | No WIFI connection : can not update" << endl;
        return;
    }

    if (state == STATE_SEND) {
        if (info) Serial
                << "Sending UDP NTP packet to : " << ntpServerName << endl;
        // get a random server from the pool
        WiFi.hostByName(ntpServerName, timeServerIP);
        if (info) {
            Serial
                << "Connected to : " << timeServerIP
                << " NTP Server IP address from the DNS pool." << endl;
        }
        sendNTPpacket(timeServerIP); // send an NTP packet to a time server
        state     = STATE_RECV;
        recvStart = millis();
    } else if (state == STATE_RECV) {
        int cb       = udp.parsePacket();
        long elapsed = millis() - recvStart;
        if (!cb) {
            if (elapsed >= 1000) {
                if (info) {
                    Serial
                        << "No packet yet from : " << timeServerIP
                        << " NTP Server IP, will wait more." << endl;
                }
                state = STATE_WAIT;
            }
        } else {
            if (info) {
                Serial
                    << "We've received a packet from : "
                    << timeServerIP
                    << " NTP Server IP, read the packet into the buffer." << endl;
            }
            udp.read(packetBuffer, NTP_PACKET_SIZE);

            // the timestamp starts at byte 40 of the received packet and is four bytes,
            // or two words, long. First, esxtract the two words:

            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord  = word(packetBuffer[42], packetBuffer[43]);
            // combine the four bytes (two words) into a long integer
            // this is NTP time (seconds since Jan 1 1900):
            unsigned long secsSince1900 = highWord << 16 | lowWord;

            // now convert NTP time into everyday time:
            // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
            const unsigned long seventyYears = 2208988800UL;
            // subtract seventy years:
            unsigned long epoch = secsSince1900 - seventyYears + DST_OFFSET * 3600;

            if (info) {
                Serial
                    << "unix ntp time received: "
                    << epoch
                    << " setting it to the clock." << endl;
            }

            setTime(epoch);
            t.unixtime = epoch;

            setClockTime();

            state = STATE_WAIT;
        }
    } else if (state == STATE_WAIT) {
        // update every NTP_UPDATE_EVERY in milliseconds
        if ((millis() - lastUpdate) < NTP_UPDATE_EVERY) {
            yield();
            return;
        }
        // wait NTP_UPDATE_EVERY seconds before asking for the time again
        state      = STATE_SEND;
        lastUpdate = millis();
    }
} // DS_3231::ntpUpdate

// send an NTP request to the time server at the given address
void DS_3231::sendNTPpacket(IPAddress& address)
{
    if (WiFi.status() != WL_CONNECTED) {
        if (debug.errors) Serial
                << "NTP UPDATE | No WIFI connection : can not update" << endl;
        return;
    }

    if (info) Serial
            << "sending NTP packet..." << endl;
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); // NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

// Convert normal decimal numbers to binary coded decimal
int DS_3231::intToBcd(int val)
{
    return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
int DS_3231::bcdToInt(int val)
{
    return ( (val / 16 * 10) + (val % 16) );
}

void DS_3231::setClockTime()
{
    // sets time and date data to DS3231
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0);                         // set next input to start at the seconds register
    Wire.write(intToBcd(second()));        // set seconds
    Wire.write(intToBcd(minute()));        // set minutes
    Wire.write(intToBcd(second()));        // set hours
    Wire.write(intToBcd(weekday()));       // set day of week (1=Sunday, 7=Saturday)
    Wire.write(intToBcd(day()));           // set date (1 to 31)
    Wire.write(intToBcd(month()));         // set month
    Wire.write(intToBcd((year() - 2000))); // set year (0 to 99)
    Wire.endTransmission();
}

void DS_3231::readClockTime()
{
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0); // set DS3231 register pointer to 00h
    Wire.endTransmission();
    Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
    // request seven bytes of data from DS3231 starting from register 00h
    t.second    = bcdToInt(Wire.read() & 0x7f);
    t.minute    = bcdToInt(Wire.read());
    t.hour      = bcdToInt(Wire.read() & 0x3f);
    t.dayOfWeek = bcdToInt(Wire.read());
    t.day       = bcdToInt(Wire.read());
    t.month     = bcdToInt(Wire.read());
    t.year      = bcdToInt(Wire.read()) + 2000;

    setTime(t.hour, t.minute, t.second, t.day, t.month, t.year);
    t.unixtime = now();
    if (info) {
        Serial << "Time set to Unix time: " << t.unixtime << endl;
        Serial << hour() << ":" << minute() << ":" << second() << " "
               << day() << "/" << month() << "/" << year() << endl;
    }
}

void DS_3231::printClockTime()
{
    // retrieve data from DS3231
    readClockTime();

    // send it to the serial monitor
    Serial << t.hour << ":";

    if (t.minute < 10)
        Serial << "0";
    Serial << t.minute << ":";

    if (t.second < 10)
        Serial << "0";
    Serial << t.second;

    Serial << " " << t.day << "/" << t.month << "/" << t.year;
    Serial << " Day of week: ";

    Serial << dayStr(t.dayOfWeek);

    Serial << endl;
} // displayTime
