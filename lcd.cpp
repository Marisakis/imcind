//Code by Sebastiaan Colijn, Oct 2023

#include "lcd.h"
void updateLCDLine(int row, const char * textToPrint);

int maxLCDLineSize = 20;
LiquidCrystal_I2C lcd(0x27, maxLCDLineSize, 4); // set the LCD address to 0x27 for a 20 chars and 4 line display
const char  * * LcdText;

//Attempts to start the LCD display 
//Will show a warning on the LCD if memory allocation for new text failed
void LCDinit() {
  lcd.init();  //returns void, we cannot check for errors
  lcd.backlight();  //activate the backlight
  lcd.setCursor(0, 0);
  lcd.print("No text found");
  LcdText = (const char  * * ) malloc( 4 * sizeof(char *));
  //LcdText = NULL; //REMOVE BEFORE USE
  if (LcdText == NULL)
  {
    Serial.println(F("LCD array initialization failed"));
    return;
  }

  Serial.println(F("LCD array initialization succesful"));
  LcdText[0] = "    Hello,menus!    "; // padded string: 20 characters, always writes entire top row
  LcdText[1] = "";
  LcdText[2] = "";                    //free space
  LcdText[3] = "";
  lcd.clear(); //blocking function for 2 milliseconds
}

//Will add the provided text to the LCD display memory
void setLCDLine(int row, const char * textToPrint)
{
  if(LcdText == NULL)
    return;
  if (row >= 1 && row <= 4 && textToPrint != NULL)
  {
    LcdText[row] = textToPrint;
  }
}

//Private method for printing on a row of the LCD screen
void updateLCDLine(int row, const char * textToPrint)
{
  lcd.setCursor(0, row);
  char croppedText[maxLCDLineSize + 1];
  memcpy(croppedText, textToPrint, maxLCDLineSize);
  croppedText[maxLCDLineSize] = '\0'; //force append null terminator
  lcd.print(croppedText); // print text without overflow
}

//Clears the LCD screen and updates it with assigned text
void UpdateLCDScreen()
{
  if(LcdText == NULL)
    return;
  lcd.clear(); //blocking function for 2 milliseconds
  for (int i = 0; i < 4; i++)
  {
    updateLCDLine(i, LcdText[i]);
  }
}

//37 lines
