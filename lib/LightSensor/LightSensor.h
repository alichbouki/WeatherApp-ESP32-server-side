#ifndef LIGHTSENSOR
#define LIGHTSENSOR

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

#define LIGHT_INT_HIGH 1000 // High light level for interrupt
#define LIGHT_INT_LOW 10 

void interruptRoutine();
void initLightSensor(int inter, SparkFun_APDS9960 apds);
uint16_t getLight(SparkFun_APDS9960 sensor);

#endif