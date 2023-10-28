//Code by Sebastiaan Colijn, Oct 2023

#ifndef LCDMENU
#define LCDMENU

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

int initMenu();
int addMenuOption(const char * menu_option_text) ;
int addMenuOptionFunc(const char * menu_option_text, void (* menu_func_ptr)(void));
const char * getCurrentMenuText();
void goNextMenu();
void goPreviousMenu();
void executeCurrentMenu();
void freeMenuMemory();

#ifdef __cplusplus
}
#endif

#endif

//20 lines
