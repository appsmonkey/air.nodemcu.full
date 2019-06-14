#include <Adafruit_NeoPixel.h>
#include "NeoPatterns.h"

// Constructor - calls base-class constructor to initialize strip
NeoPatterns::NeoPatterns(uint16_t pixels, uint8_t pin)
    : Adafruit_NeoPixel(pixels, pin)
{ }

void NeoPatterns::Update()
{
    if ((millis() - lastUpdate) > Interval) { // time to update
        lastUpdate = millis();
        switch (ActivePattern) {
            case RAINBOW_CYCLE:
                RainbowCycleUpdate();
                break;
            case THEATER_CHASE:
                TheaterChaseUpdate();
                break;
            case COLOR_WIPE:
                ColorWipeUpdate();
                break;
            case SCANNER:
                ScannerUpdate();
                break;
            case FADE:
                FadeUpdate();
                break;
            default:
                break;
        }
    }
}

// Increment the Index and reset at the end
void NeoPatterns::Increment()
{
    if (Direction == FORWARD) {
        Index++;
        if (Index >= TotalSteps) {
            Index = 0;
            if (OnComplete != NULL) {
                OnComplete(); // call the comlpetion callback
            }
        }
    } else { // Direction == REVERSE
        --Index;
        if (Index <= 0) {
            Index = TotalSteps - 1;
            if (OnComplete != NULL) {
                OnComplete(); // call the comlpetion callback
            }
        }
    }
}

// Initialize for a RainbowCycle
void NeoPatterns::RainbowCycle(uint8_t interval, direction dir /* = FORWARD */)
{
    ActivePattern = RAINBOW_CYCLE;
    Interval      = interval;
    TotalSteps    = 255;
    Index         = 0;
    Direction     = dir;
}

// Update the Rainbow Cycle Pattern
void NeoPatterns::RainbowCycleUpdate()
{
    for (int i = 0; i < numPixels(); i++) {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
    }
    show();
    Increment();
}

// Initialize for a ColorWipe
void NeoPatterns::ColorWipe(uint32_t color, uint8_t interval, direction dir /* = FORWARD */)
{
    ActivePattern = COLOR_WIPE;
    Interval      = interval;
    TotalSteps    = numPixels();
    Color1        = color;
    Index         = 0;
    Direction     = dir;
}

// Update the Color Wipe Pattern
void NeoPatterns::ColorWipeUpdate()
{
    setPixelColor(Index, Color1);
    show();
    Increment();
}

// Initialize for a Theater Chase
void NeoPatterns::TheaterChase(
    uint32_t  color1,
    uint32_t  color2,
    uint8_t   interval,
    direction dir /* = FORWARD */)
{
    ActivePattern = THEATER_CHASE;
    Interval      = interval;
    TotalSteps    = numPixels();
    Color1        = color1;
    Color2        = color2;
    Index         = 0;
    Direction     = dir;
}

// Update the Theater Chase Pattern
void NeoPatterns::TheaterChaseUpdate()
{
    for (int i = 0; i < numPixels(); i++) {
        if ((i + Index) % 3 == 0) {
            setPixelColor(i, Color1);
        } else {
            setPixelColor(i, Color2);
        }
    }
    show();
    Increment();
}

// Initialize for a SCANNNER
void NeoPatterns::Scanner(uint32_t color1, uint8_t interval)
{
    ActivePattern = SCANNER;
    Interval      = interval;
    TotalSteps    = (numPixels() - 1) * 2;
    Color1        = color1;
    Index         = 0;
}

// Update the Scanner Pattern
void NeoPatterns::ScannerUpdate()
{
    for (int i = 0; i < numPixels(); i++) {
        if (i == Index) { // first half of the scan
            Serial.print(i);
            setPixelColor(i, Color1);
        } else if (i == TotalSteps - Index) { // The return trip.
            Serial.print(i);
            setPixelColor(i, Color1);
        } else { // fade to black
            setPixelColor(i, DimColor(getPixelColor(i)));
        }
    }
    show();
    Increment();
}

// Initialize for a Fade
void NeoPatterns::Fade(
    uint32_t  color1,
    uint32_t  color2,
    uint16_t  steps,
    uint8_t   interval,
    direction dir /* = FORWARD */)
{
    ActivePattern = FADE;
    Interval      = interval;
    TotalSteps    = steps;
    Color1        = color1;
    Color2        = color2;
    Index         = 0;
    Direction     = dir;
}

// Update the Fade Pattern
void NeoPatterns::FadeUpdate()
{
    uint8_t red   = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue  = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(Color(red, green, blue));
    show();
    Increment();
}

// Returns the Red component of a 32-bit color
uint8_t NeoPatterns::Red(uint32_t color)
{
    return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t NeoPatterns::Green(uint32_t color)
{
    return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t NeoPatterns::Blue(uint32_t color)
{
    return color & 0xFF;
}

// Return color, dimmed by 75%
uint32_t NeoPatterns::DimColor(uint32_t color)
{
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);

    return dimColor;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t NeoPatterns::Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

// Reverse direction of the pattern
void NeoPatterns::Reverse()
{
    if (Direction == FORWARD) {
        Direction = REVERSE;
        Index     = TotalSteps - 1;
    } else {
        Direction = FORWARD;
        Index     = 0;
    }
}

// Set all pixels to a color (synchronously)
void NeoPatterns::ColorSet(uint32_t color)
{
    for (int i = 0; i < numPixels(); i++) {
        setPixelColor(i, color);
    }
    show();
}
