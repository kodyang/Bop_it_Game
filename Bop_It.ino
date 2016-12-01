#include <FillPat.h>
#include <LaunchPad.h>
#include <OrbitBoosterPackDefs.h>
#include <OrbitOled.h>
#include <OrbitOledChar.h>
#include <OrbitOledGrph.h>
#include <stdlib.h>

void WireInit();
void GameUIInit();
void ShakeInit();

void ShakeTick();
void GameUITick();

extern const uint32_t Potentiometer;

void setup() 
{
  WireInit();
  Serial.begin(9600);

  delay(100);

  ShakeInit();
  GameUIInit();

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  
}

void loop() 
{
  ShakeTick();
  GameUITick();
}
