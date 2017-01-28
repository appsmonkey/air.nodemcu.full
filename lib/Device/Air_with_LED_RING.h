// Load CityOS framework first
#include "CityOS.h"

// We are using NodeMCU v2 board
#include "NodeMCU.h"

#include "WS_2812_Ring.h"

// Accesss to all data
CityOS ctos;

WS_2812_Ring ring(D2, 12);
