// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

// Add inputs (sensors) header files
#include "DS_3231.h"

// Accesss to all data
CityOS ctos;

// Initialize PINS on all inputs and outputs
DS_3231 ds(D1, D2); // SCL, SDA
