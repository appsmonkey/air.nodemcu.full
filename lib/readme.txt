# Libs
This directory is for the libraries.
PlatformIO will compile them to static libraries and link to executable file.

Place the source code of each library in separate directory, like
"lib/Control/RGB_LED/[here are source files]".

There are 3 Main groups of libs:

## Boards
e.g. "lib/Board/NodeMCU/[here are source files]".
Libs specific to the boards like pin layout mappings, specific board libraries includes etc. 

## Control
e.g. "lib/Control/RGB_LED/[here are source files]".
These are all things that you will control: leds, servos, relays...

## Sense
e.g. "lib/Sense/DHT22/[here are source files]".
These are all things that you will use to sense (one per sensor or unit)

## Device
e.g. "lib/Device/Air.h".
These are different Device configurations 

For example, see how we organized `Air Device` libraries:

|--lib
|  |--Board
|  |  |--NodeMCU
|  |     |- NodeMCU.h
|  |--Control
|  |  |--RGB_LED
|  |     |- RGB_LED.h
|  |     |- RGB_LED.cpp
|  |--Sense
|  |  |--DHT_22
|  |     |- DHT_22.h
|  |     |- DHT_22.cpp
|- platformio.ini
|- Air.ino

Then in `Air.ino` you use:

#include <Device.h>

// rest H/C/CPP code

Setup will find your libraries automatically, configure preprocessor's
include paths and build them.

More information about PlatformIO Library Dependency Finder
- http://docs.platformio.org/page/librarymanager/ldf.html
