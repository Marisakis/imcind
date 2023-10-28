//Code by Sebastiaan Colijn, Oct 2023


#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "menu.h"
#include "lcd.h"

#define BlockCharacter 0b11111111 //useful for animations

TWIST twist; //Qwiic Twist via Arduino library on default adress 0x3F

SemaphoreHandle_t i2c_handle;
void TaskMenuLoop(void * pvParameters);
void TaskMenuButton(void * pvParameters);
void TaskBounce(void * pvParameters);
char * realTimeBounce;

void cycleLedColour()
{
  int old = twist.getGreen();
  twist.setGreen(twist.getBlue());
  twist.setBlue(twist.getRed());
  twist.setRed(old);
}

//tasks that control user input for the menu
void setupTasks()
{
  xTaskCreate(
    TaskMenuLoop
    ,  "MenuLoop"   //task name
    ,  128  // stack size
    ,  NULL
    ,  1  // priority
    ,  NULL );

  xTaskCreate(
    TaskMenuButton
    ,  "MenuButton"   //task name
    ,  128  // stack size
    ,  NULL
    ,  1  // priority
    ,  NULL );
}

//task that animates a block bouncing on the screen as well as screen updates
void setupBounceTask()
{
  realTimeBounce = (char * ) malloc (sizeof(char) * 20);
  if (realTimeBounce == NULL)
    return;
  for (int i = 0; i < 20; i++)
  {
    realTimeBounce[i] = ' ';
  }
  realTimeBounce[0] = BlockCharacter;
  realTimeBounce[20] = '\0';
  setLCDLine(3, realTimeBounce);

  xTaskCreate(
    TaskBounce
    ,  "Bounce"   //task name
    ,  128  // stack size
    ,  NULL
    ,  2  // priority
    ,  NULL );
}

void setup() {
  Serial.begin(115200);

  int menu_err = initMenu();
  if (menu_err != 0)
    Serial.println(F("Menu initiation failed")); //F forces PROGMEM or into flash memory which frees memory but is read much slower. Good for single use statements
  else
    Serial.println(F("Menu initiation succesful"));

  int twist_status = twist.begin();
  if (twist_status == true)
  {
    Serial.println(F("Twist button initiation succesful"));
    twist.setColor(0, 0, 50); //soft blue
    twist.connectColor(0, 0, 0); //disable automated brightness change
  }
  else
  {
    Serial.println(F("Twist button initiation failed"));
  }

  addMenuOptionFunc("Menu option 1", cycleLedColour); //allows a demonstration of function binding
  addMenuOptionFunc("Menu option 2", NULL);
  const char * menu = "Menu option 3";
  addMenuOption(menu);
  addMenuOption("Menu option 4 whichiswaylongerthantheLCDcandisplayononerow");
  addMenuOption("Menu option 5");
  addMenuOption("Menu option 6");
  //  addMenuOption("Menu option 7");
  //  addMenuOption("Menu option 8");
  //  addMenuOption("Menu option 9");
  //  addMenuOption("Menu option 10");
  //  addMenuOption("Menu option 11"); //filling up that malloc space, but 11 strings is too much for the Arduino Uno to handle along with the FreeRtos tasks, only 2kB of memory

  LCDinit();
  setLCDLine(1, getCurrentMenuText());
  setLCDLine(3, realTimeBounce);
  UpdateLCDScreen();

  i2c_handle = xSemaphoreCreateMutex(); //create priority inheritance mutex, use on all i2c transactions (in tasks, not setup) to prevent data mixing
  setupTasks();
  setupBounceTask();

  Serial.println(F("Setup done, starting tasks"));
}

void loop() {
  // No loop, because FreeRTOS
}

void TaskMenuLoop(void * pvParameters)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;//timer: 20 times per second
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    if (i2c_handle != NULL)
    {
      if (xSemaphoreTake( i2c_handle, 10 / portTICK_PERIOD_MS))        // wait max 10 ms if not available

      {
        int diff = twist.getDiff(); //difference in steps, can be positive (clockwise) or negative (counterclockwise)
        if (diff > 0)
        {
          for (int i = 0; i < diff; i++) //for loop allows very fast scrolling in large menu structures, rather than one step per poll
          {
            goNextMenu();
          }
        }
        else if (diff < 0)
        {
          for (int i = 0; i > diff; i--)
          {
            goPreviousMenu();
          }
        }
        if (diff != 0)
        {
          setLCDLine(1, getCurrentMenuText());
          UpdateLCDScreen();//call here to increase responsivesness
        }
        xSemaphoreGive(i2c_handle);
      }
    }
  }
}

void TaskMenuButton(void * pvParameters)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;//timer: 20 times per second
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    if (i2c_handle != NULL)
    {
      if (xSemaphoreTake( i2c_handle, 10 / portTICK_PERIOD_MS))        // wait max 10 ms if not available

      {
        if (twist.isPressed())
        {
          executeCurrentMenu();
        }
        xSemaphoreGive(i2c_handle);
        vTaskDelay(200 / portTICK_PERIOD_MS ); //allow only 5 presses per second but requires no button release
      }
    }
  }
}


void TaskBounce(void * pvParameters)
{
  int bounceLocation = 0;
  int direction = 1;
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 200 / portTICK_PERIOD_MS;//timer: 5 times per seconds
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    realTimeBounce[bounceLocation] = ' ';
    if (direction == 1)
    {
      bounceLocation ++;
      if (bounceLocation >= 19)
        direction = 0;
    }
    else
    {
      bounceLocation --;
      if (bounceLocation <= 0)
        direction = 1;
    }

    realTimeBounce[bounceLocation] = BlockCharacter;

    if (i2c_handle != NULL)
    {
      if (xSemaphoreTake( i2c_handle, 10 / portTICK_PERIOD_MS))        // wait max 10 ms if not available
      {
        UpdateLCDScreen();// this is the looped update. It really should be in it's own task, but UNO memory is sparse.
        xSemaphoreGive(i2c_handle);
      }
    }
  }
}
//109 lines
