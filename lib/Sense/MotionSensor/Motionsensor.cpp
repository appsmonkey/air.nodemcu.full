#include <MotionSensor.h>

MotionSensor::MotionSensor(int pin){
  
    this->pin=pin;

    value=LOW; 
    pirState=LOW;

    //sample calibrate time
    calibrateTime = 30000;

    sense("motion");

    //addToInterval(this);

    addToLoop(this);

}

int MotionSensor::read(){
  
  value= digitalRead(pin);
  

  return value;
}

MotionSensor::~MotionSensor(){}

bool MotionSensor::motion_detected(){

  this->read();
  

  if(value == HIGH){    
   return true;
  } 

  return false;
}

// void MotionSensor::interval(){
//   this->read();

//   if(value == HIGH){
//     Serial.println("Motion detected!");
//   }else{
//       Serial.println("Not detected");
//   }

//   setSense("motion", value);
// }

void MotionSensor::loop(){

  static int lastUpdate = 0; 
  

  if ((millis() - lastUpdate) < 1000) {
      yield();
      return;
  }

  lastUpdate = millis();

  //if sensor is calibrated read 
  if(calibrated()){
    read();
  }

  reportAllData();
  //reportWhenNewMotion();

}

bool MotionSensor::calibrated(){

  return millis()-calibrateTime>0;

}

void MotionSensor::reportAllData(){
  setSense("motion", value);
  Serial.println(value);
  Serial.println(" ");
}

void MotionSensor::reportWhenNewMotion(){

  if(value==HIGH){

    if(pirState==LOW){

        setSense("motion", value);
        Serial.println(value);
        Serial.println(" ");
        pirState=HIGH;

    }
  }else{

        if(pirState==HIGH){
          pirState=LOW;
        }

  }

}