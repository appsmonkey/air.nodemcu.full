#ifndef NEO_PATTERNS_H
#define NEO_PATTERNS_H

#include <Adafruit_NeoPixel.h>

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns: public Adafruit_NeoPixel
{
public:

    // Member Variables:
    pattern ActivePattern; // which pattern is running
    direction Direction;   // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2; // What colors are in use
    uint16_t TotalSteps;     // total number of steps in the pattern
    uint16_t Index;          // current step within the pattern

    void (* OnComplete)(); // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin);

    void Complete();
    // Update the pattern
    void Update();

    // Increment the Index and reset at the end
    void Increment();

    // Follwing two are same for all options bellow
    // interval : number of milliseconds between updates, which determines the speed of the pattern.
    // dir : [optional] FORWARD/REVERSE

    void RainbowCycle(uint8_t interval, direction dir = FORWARD);
    void RainbowCycleUpdate();

    // color : color to 'wipe' across the strip.
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD);
    void ColorWipeUpdate();

    // color1 and color2 : foreground and background colors of the pattern.
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD);
    void TheaterChaseUpdate();

    // color : color of the scanning pixel.
    void Scanner(uint32_t color1, uint8_t interval);
    void ScannerUpdate();

    // color1 : starting color
    // color2 : ending color
    // steps : how many steps it should take to get from color1 to color2.
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD);
    void FadeUpdate();

    // Red(), Blue() and Green() functions.
    // These are the inverse of the Neopixel Color() function.
    // They allow us to extract the Red, Blue and Green components of a pixel color.
    uint8_t Red(uint32_t color);
    uint8_t Green(uint32_t color);
    uint8_t Blue(uint32_t color);

    // Return color, dimmed by 75% (e.g. used by scanner)
    uint32_t DimColor(uint32_t color);

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos);

    // Reverse direction of the pattern
    void Reverse();

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color);
};


#endif /* ifndef NEO_PATTERNS_H */
