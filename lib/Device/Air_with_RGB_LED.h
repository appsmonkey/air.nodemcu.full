// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

// Add inputs (sensors) header files
#include "DHT_22.h"
#include "PMS_1003.h"

// Add output
#include "RGB_LED.h"

// Accesss to all data
CityOS ctos;

// Initialize PINS on all inputs and outputs
DHT_22 dht(D1);
PMS_1003 pm(D7, D8);
RGB_LED led(D2, D5, D6);
