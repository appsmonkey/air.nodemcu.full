// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

// Add inputs (sensors) header files
#include "DS_18B20.h"

// Accesss to all data
CityOS ctos;

DS_18B20 ds(D1);
