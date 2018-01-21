// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

// Add inputs (sensors) header files
#include "BH_1750.h"

// Accesss to all data
CityOS ctos;

BH_1750 bh(D3, D4); // SDA, SLC
