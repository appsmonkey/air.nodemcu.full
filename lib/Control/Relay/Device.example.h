// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

// Add inputs (sensors) header files
#include "Relay.h"

// Accesss to all data
CityOS ctos;

// Initialize PINS on all inputs and outputs
Relay light(D1, true);
