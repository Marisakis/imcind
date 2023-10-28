//Code by Sebastiaan Colijn, Oct 2023

#include "menu.h"

#define MENU_INCREMENT_SIZE 5

//typedef to allow the following struct to refer to itself
typedef struct menuoption menuoption;

//this struct is intended to be part of a chain of them. 
struct menuoption {
  const char* menu_text;
  menuoption * next_menu;
  menuoption * previous_menu;
  void (* menu_func_ptr)(void );
};

menuoption * current_menu = NULL;
menuoption * menus = NULL;
int current_menu_array_size = 0;
int current_number_of_menus = 0;

//creates room on heap, returns -1 on insufficient memory
int initMenu()
{
  menus = malloc( MENU_INCREMENT_SIZE* (sizeof(menuoption)));
  if (menus == NULL)
  {
    return -1;
  }
  current_menu_array_size = MENU_INCREMENT_SIZE;
  return 0;
}

//checks if current arrayy size is sufficient, otherwise expands it, returns -1 on insufficient memory, returns -2 on no initialization done. Not exposed to users.
int _checkSize()
{
  if (current_menu_array_size == 0 || menus == NULL)
  {
    return -2;
  }
  //array is full, expand before adding struct
  if (current_number_of_menus == current_menu_array_size)
  {
    menus = realloc(menus, (MENU_INCREMENT_SIZE + current_menu_array_size) * (sizeof(menuoption)));
    if (menus == NULL)
    {
      return -1;
    }
    current_menu_array_size = current_menu_array_size + MENU_INCREMENT_SIZE;
  }
  return 0;
}

//Add a menu option to the list
//Args: a non-null char pointer and a function pointer. Passing a null function pointer is effectively the same as addMenuOption(char *)
//Returns -1 on null char pointer argument. Returns -2 if initMenu has not been executed first
int addMenuOptionFunc( const char * menu_option_text, void (* menu_func_ptr)(void))
{
    if(menu_option_text == NULL)
      return -1;
    int err = addMenuOption(menu_option_text);
    if(err == 0)
      menus[current_number_of_menus-1].menu_func_ptr = menu_func_ptr;
    return err;
}



//Add a menu option to the end of the existing list, without a function attached
//Args: a non-null char pointer
//Returns -1 on null argument. Returns -2 if initMenu has not been executed first
int addMenuOption(const char * menu_option_text)
{
  if(menu_option_text == NULL)
    return -1;

  if (_checkSize() < 0)
    return -2;
  
  //create first struct
  if (current_number_of_menus == 0)
  {
    //printf("String = %s,  Address = %u\n", menu_option_text, menu_option_text);

    menus[0].menu_text = menu_option_text;
    menus[0].next_menu = &menus[0];
    menus[0].previous_menu = &menus[0];
    menus[0].menu_func_ptr = NULL;// init pointer to prevent undefined behaviour
    current_number_of_menus ++;
    current_menu = &menus[0];
  }
  //current_number_of_menus != 0)
  else
  {
    //add new menu option, link references. This creates a single-layered menu only.
    menus[current_number_of_menus].menu_text = menu_option_text;
    menus[current_number_of_menus].previous_menu = &menus[current_number_of_menus - 1]; //reference last old member
    //menus[current_number_of_menus].next_menu = menus[current_number_of_menus - 1].next_menu; //copy value (which is always menus[0] in the current design, since we are not inserting, but anyway..)
    menus[current_number_of_menus].next_menu = &menus[0]; //always create loop forward to start
    menus[current_number_of_menus - 1].next_menu = &menus[current_number_of_menus]; //overwrite reference with new member
    menus[0].previous_menu = &menus[current_number_of_menus]; //always create link between first and last member so the menu can loop back
    menus[current_number_of_menus].menu_func_ptr = NULL;// init function pointer to prevent undefined behaviour

    current_number_of_menus ++;
  }
  return 0;
}


//Gets the char pointer for the currently selected menu item
//May return null: only use after initializing and succesfully adding an item!
const char * getCurrentMenuText()
{
  if(current_menu!= NULL)
    return current_menu->menu_text;
  else
    return NULL;
}

//Cycles the menu to the next option
void goNextMenu()
{
  if(current_menu != NULL)
    current_menu = current_menu->next_menu;
}

//Cycles the menu to the preivous option
void goPreviousMenu()
{
  if(current_menu != NULL)
    current_menu = current_menu->previous_menu;
}

//Executes the function attached to the currently selected menu option, if one exists
void executeCurrentMenu()
{
  if(current_menu != NULL)
  {
    if (current_menu->menu_func_ptr != NULL)
    {
      (current_menu->menu_func_ptr)();
    }
  }
}

//Frees any allocated memory and returns all variables to their default state
void freeMenuMemory()
{
  free(menus); // will ignore if menus is already NULL
  menus = NULL;
  current_menu = NULL;
  current_menu_array_size = 0;
  current_number_of_menus = 0;
}

//75 lines
