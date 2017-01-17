
This directory is for the project specific (private) libraries.
PlatformIO will compile them to static libraries and link to executable file.

Place the source code of each library in separate directory, like
"lib/Air/[here are source files]".

For example, see how we organized `Air` library:

|--lib
|  |--Air
|  |  |--docs
|  |  |--examples
|  |  |--src
|  |     |- Air.cpp
|  |     |- Air.h
|  |- readme.txt --> THIS FILE
|- platformio.ini
|- CityOSAir.ino

Then in `CityOSAir.ino` you use:

#include <Air.h>

// rest H/C/CPP code

Setup will find your libraries automatically, configure preprocessor's
include paths and build them.

More information about PlatformIO Library Dependency Finder
- http://docs.platformio.org/page/librarymanager/ldf.html
