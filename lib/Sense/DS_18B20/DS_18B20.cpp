// Connect:
//
// NodeMCU 3v3 to the Vin of DS18B20 - Red
// NodeMCU Dx to the data of DS18B20 - Yellow (a 4.7K resistor is necessary)
// NodeMCU GND to the GND of DS18B20 - Gray or Black
// a 4.7kOhm resistor between Vin and data of DS18B20.

#include <DS_18B20.h>

DS_18B20::DS_18B20(int pin = D1)
{
    _ds = new OneWire(pin);

    info = false;

    temperature = 0;

    sense("soil temperature");

    addToInterval(this);
}

DS_18B20::~DS_18B20()
{
    delete _ds;
}

void DS_18B20::interval()
{
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius, fahrenheit;


    if (!_ds->search(addr)) {
        if (debug.errors) Serial
                << "No more addresses." << endl;
        _ds->reset_search();
        delay(250);
        return;
    }

    if (info) {
        Serial << "ROM =";
        for (i = 0; i < 8; i++) {
            Serial << ' ';
            Serial.print(addr[i], HEX);
        }
        Serial << endl;
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial << "CRC is not valid!" << endl;
        return;
    }

    // the first ROM byte indicates which chip
    switch (addr[0]) {
        case 0x10:
            if (info) Serial
                    << "  Chip = DS18S20 or old DS1820" << endl;
            type_s = 1;
            break;
        case 0x28:
            if (info) Serial
                    << "  Chip = DS18B20" << endl;
            type_s = 0;
            break;
        case 0x22:
            if (info) Serial
                    << "  Chip = DS1822" << endl;
            type_s = 0;
            break;
        default:
            if (debug.errors) {
                Serial
                    << "Device ON PIN: " << pin
                    << "is not a DS18x2x device." << endl
                    << "supported variations : "
                    << "DS1820, DS18S20, DS18B20 and DS1822."
                    << endl;
            }

            return;
    }


    _ds->reset();
    _ds->select(addr);
    _ds->write(0x44, 1); // start conversion, with parasite power on at the end

    delay(1000); // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    present = _ds->reset();
    _ds->select(addr);
    _ds->write(0xBE); // Read Scratchpad

    if (info) {
        Serial << "  Data = ";
        Serial.print(present, HEX);
        Serial << " ";
    }
    for (i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = _ds->read();
        if (info) Serial.print(data[i], HEX);
        if (info) Serial << " ";
    }
    if (info) {
        Serial << " CRC= ";
        Serial.print(OneWire::crc8(data, 8), HEX);
        Serial << endl;
    }
    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;       // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3;  // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1;  // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius     = (float) raw / 16.0;
    temperature = celsius;
    // fahrenheit = celsius * 1.8 + 32.0;

    setSense("soil temperature", temperature);
} // DS_18B20::interval
