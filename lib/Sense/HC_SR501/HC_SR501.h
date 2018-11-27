#ifndef HC_SR501_H
#define HC_SR501_H

#include "CityOS.h"

class HC_SR501 : public CityOS{
    public:
      HC_SR501(int pin, unsigned long calibrateTime=60, unsigned long updateInterval=1);      

      int read();
      
      void loop();

      void interval();   

      bool debug = false;    

    private:
      
      int pin;

      int value;
      int pirState;

      unsigned long calibrateTime; 
      unsigned long updateInterval;

      bool isCalibrated();   
      
};
#endif