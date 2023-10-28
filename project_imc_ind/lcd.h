//Code by Sebastiaan Colijn, Oct 2023
//This functionality relies on the LiquidCrystal_I2C lcd class, a c++ concept, and therefore has an accompanying cpp file

#include <Wire.h>
#include<Arduino.h>
#ifndef LCDDISPLAY
#define LCDDISPLAY

#include <LiquidCrystal_I2C.h>

void LCDinit();
void setLCDLine(int row, const char * textToPrint);
void UpdateLCDScreen();

#endif

//9 lines
