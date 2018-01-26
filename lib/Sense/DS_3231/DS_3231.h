#ifndef DS_3231_H
#define DS_3231_H

#include "CityOS.h"
#include "Wire.h"
#include "WiFiUdp.h"
#include <TimeLib.h>

#define DS3231_I2C_ADDRESS 0x68

class DS_3231 : public CityOS {
public:

    struct _time {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int dayOfWeek;
        int unixtime;
    } t;

    bool info = true;

    DS_3231(int scl, int sda);

    void ntpUpdate();
    void populateUnixTime();

    void interval();
private:
    void sendNTPpacket(IPAddress& address);
    const int NTP_UPDATE_EVERY = 3600000; // 1 hr. = 3600000 ms
    // const int NTP_UPDATE_EVERY = 10000; // 10s

    unsigned int localPort; // local port to listen for UDP packets

    /* Don't hardwire the IP address or we won't get the benefits of the pool.
     *  Lookup the IP address for the host name instead */
    // IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
    IPAddress timeServerIP; // time.nist.gov NTP server address
    const char * ntpServerName;

    static const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

    const int DST_OFFSET = 0;

    byte packetBuffer[NTP_PACKET_SIZE]; // NTP_PACKET_SIZE buffer to hold incoming and outgoing packets

    // A UDP instance to let us send and receive packets over UDP
    WiFiUDP udp;
    const int STATE_SEND = 1;
    const int STATE_RECV = 2;
    const int STATE_WAIT = 3;

    int state;
    long recvStart;

    int intToBcd(int val);
    int bcdToInt(int val);

    void setClockTime();
    void readClockTime();
    void printClockTime();
};

#endif /* ifndef DS_3231_H */
