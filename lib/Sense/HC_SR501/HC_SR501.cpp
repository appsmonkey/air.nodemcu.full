#include "HC_SR501.h"

HC_SR501::HC_SR501(int pin, unsigned long calibrateTime, unsigned long updateInterval){
  
    this->pin=pin;

    value=LOW; 
    pirState=LOW;

    //sample calibrate time
    this->calibrateTime = calibrateTime*1000;
    this->updateInterval = updateInterval*1000;

    sense(config["MOTION"]);
    //intrval is used to send data to server
    addToInterval(this);
    //loop is used to poll motion sensor for a change
    addToLoop(this);    

}

int HC_SR501::read(){
  
  value= digitalRead(pin);  

  return value;
}

void HC_SR501::loop(){

  static int lastUpdate = 0; 
//if sensor is not calibrated return
  if(!isCalibrated()){
      yield();
      return;
  }
  //if update interval is not passed return
  if( ((millis() - lastUpdate) < updateInterval)){
      yield();
      return;
  }

  lastUpdate = millis();

  read();

  if (debug) {
        Serial
          << "Motion value detected is " << value << endl;
  }

  if(value==HIGH){
    pirState=HIGH;
  }

}

void HC_SR501::interval(){
  setSense(config["MOTION"], pirState);
  pirState=LOW;
}

bool HC_SR501::isCalibrated(){
  return ((long)(millis()-calibrateTime))>0;
}

