#ifndef MotionSensor_H
#define MotionSensor_H

#include "CityOS.h"
//#include "Wire.h"

class MotionSensor : public CityOS{
    public:
      MotionSensor(int pin);
      ~MotionSensor();

      bool motion_detected();

      int read();

      //void interval();
      void loop();

      bool calibrated();

      void reportAllData();

      void reportWhenNewMotion();

    private:
      
      int pin;

      int value;
      int pirState;

      int calibrateTime;
      
};

#endif