# BME/BMP280

# Introduction
The BME280 measures pressure, humidity, and temperature, runs I2C or SPI, takes measurements at less than 1mA and idles less than 5uA (microamps). It can calculate relative altitude changes, or absolute altitude if the locally reported barometric pressure is known.

## Ranges

Temp: -40C to 85C
Humidity: 0 - 100% RH, =-3% from 20-80%
Pressure: 30,000Pa to 110,000Pa, relative accuracy of 12Pa, absolute accuracy of 100Pa
Altitude: 0 to 30,000 ft (9.2 km), relative accuracy of 3.3 ft (1 m) at sea level, 6.6 (2 m) at 30,000 ft.


## Pins
GND:	Ground
3.3v:	Power Supply - should be regulated between 1.8V and 3.6V.
SDA:	I2C data - serial data (bi-directional)
SCL:	I2C serial clock

## RED NOTE:
The BME280 is a 3.3V device! Supplying voltages greater than ~3.6V can permanently damage the IC.

## Variations
https://www.sparkfun.com/products/13676?_ga=1.223526427.1297898021.1482240603

## Tutorials
https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout/wiring-and-test
https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide
https://learn.sparkfun.com/tutorials/i2c
