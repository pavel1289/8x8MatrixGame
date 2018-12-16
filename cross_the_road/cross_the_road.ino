#include "LedControl.h"
#include <LiquidCrystal.h>
#define V0_PIN 9
#define X_IN A0
#define Y_IN A1
#define BUTTON_IN 8
LedControl ledControl = LedControl(12, 11, 10, 1);
LiquidCrystal liquidCrystalDisplay(2, 3, 4, 5, 6, 7);
bool buildings[8][8];
bool vehicles[8][8];
bool buildingsLine[8];
bool vehiclesLine[8];
bool gameMode, playerLed, fogLed, carLed, carDirection;
int positionLine, positionColumn;                                       //variables for the position of the player
int carOffset, carSpeed;                                                //variables for the movement of the vehicles/obstacles
int positionX, positionY, buttonState;                                  //variables for JoyStick
unsigned long lastTimePlayer, lastTimeCars, lastTimeMoved, lastTimeFog;

void printBuildings(bool buildings[8][8])
{
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
    ledControl.setLed(0, j, i, buildings[i][j]);
}

void printVehicles(bool vehicles[8][8])
{
  for(int i = 0; i < 8; i++)
    ledControl.setLed(0, (i + carOffset) % 8, 5, vehicles[5][i]);
}

void printFog(bool on)
{
  for(int i = 0; i < 8; i++)
    ledControl.setLed(0, i, 2, on);
}

void generateFirstLayoutBuildings(bool buildings[8][8], bool buildingsLine[8])
{
  ///incep de pe linia 7, de cate randuri sa fac strada? de cate randuri sa fac blocurile?poate 2 la blocuri? 1 la strada?
  int tunnel, safePoint;
  for (int i = 0; i < 8; i++)
  {
    if (i != 3)
    {
      buildings[7][i] = 1;
      buildings[6][i] = 1;
    }
  }
  buildingsLine[7] = 1;
  buildingsLine[6] = 1;
  buildingsLine[5] = 0;
  buildingsLine[4] = 1;
  buildingsLine[3] = 1;
  buildingsLine[2] = 0;
  buildingsLine[1] = 1;
  buildingsLine[0] = 1;
  for (int i = 0; i < 2; i++)
  {
    tunnel = rand() % 8;
    safePoint = rand() % 8;
    while (tunnel == safePoint || tunnel + 1 == safePoint || tunnel - 1 == safePoint)
      safePoint = rand() % 8;
    for (int j = 0; j < 8; j++)
    {
      if (j != tunnel)
        if (j != safePoint)
          if (i == 0)
          {
            buildings[4][j] = 1;
            buildings[3][j] = 1;
          }
          else
          {
            buildings[1][j] = 1;
            buildings[0][j] = 1;
          }
        else
          if (i == 0)
          {
            buildings[4][j] = 0;
            buildings[3][j] = 1;
          }
          else
          {
            buildings[1][j] = 0;
            buildings[0][j] = 1;
          }
      else
        if (i == 0)
        {
          buildings[4][j] = 0;
          buildings[3][j] = 0;
        }
        else
        {
          buildings[1][j] = 0;
          buildings[0][j] = 0;
        }
    }
    if (carDirection == 0)
      if (tunnel < 4)
        carDirection = -1;
      else
        carDirection = 1;
  }
}

void initialize(bool buildings[8][8], bool vehicles[8][8])//, bool buildingsLine[8], bool vehiclesLine[8])
{
  for(int i = 0; i < 8; i++)
  {
    /*buildingsLine[i] = 0;
    vehiclesLine[i] = 0;*/
    for(int j = 0; j < 8; j++)
    {
      buildings[i][j] = 0;
      vehicles[i][j] = 0;
    }
  }
}

void generateBuildings(bool buildings[8][8])//, bool buildingsLine[8])
{
  int tunnel;
  int safePoint;

  tunnel = rand() % 8;
  safePoint = rand() % 8;
  while (tunnel == safePoint || tunnel + 1 == safePoint || tunnel - 1 == safePoint)
    safePoint = rand() % 8;
  for(int i = 0; i < 8; i++)
  {
    if (i != tunnel)
      if (i != safePoint)
      {
        buildings[1][i] = 1;
        buildings[0][i] = 1;
      }
      else
      {
        buildings[1][i] = 0;
        buildings[0][i] = 1;
      }
    else
    {
      buildings[1][i] = 0;
      buildings[0][i] = 0;
    }
  }
  if (tunnel < positionColumn)
    carDirection = -1;
  else
    carDirection = 1;
}

void generateVehicles(bool vehicles[8][8])
{
  int numberOfVehicles;
  int vehicleLength[2];
  vehicleLength[0] = 0;
  vehicleLength[1] = 0;
  for(int i = 0; i < 8; i++)
  {
    vehicles[5][i] = 0;
    vehicles[2][i] = 1;
  }
  numberOfVehicles = rand() % 2;
  if (numberOfVehicles == 0)
  {
    vehicleLength[0] = rand() % 6 + 1;
    for(int i = 0; i < vehicleLength[0]; i++)
      vehicles[5][i] = 1;
  }
  else
  {
    vehicleLength[0] = rand() % 3 + 1;
    vehicleLength[1] = rand() % 3 + 1;
    for(int i = 0; i < 2; i++)
      for(int j = 0; j < vehicleLength[i]; j++)
        vehicles[5][i * 4 + j] = 1;
  }
}

void translateThreeRowsDown(bool buildings[8][8], bool vehicles[8][8])//, bool buildingsLine[8], bool vehiclesLine[8])
{
  for(int i = 0; i < 5; i++)
  {
    for(int j = 0; j < 8; j++)
    {
      buildings[i][j] = buildings[i + 3][j];
      //vehicles[i][j] = vehicles[i + 3][j];
    }
    /*buildingsLine[i] = buildingsLine[i + 3];
    vehiclesLine[i] = vehiclesLine[i + 3];*/
  }
  generateBuildings(buildings);
  generateVehicles(vehicles);
}

void setup() {
  // put your setup code here, to run once:
  ledControl.shutdown(0, false);
  ledControl.setIntensity(0, 2);
  ledControl.clearDisplay(0);
  liquidCrystalDisplay.begin(16, 2);
  liquidCrystalDisplay.clear();
  liquidCrystalDisplay.setCursor(0, 0);
  liquidCrystalDisplay.print("Welcome!");
  liquidCrystalDisplay.setCursor(0, 1);
  liquidCrystalDisplay.print("Press to start");
  carSpeed = 10;
  lastTimePlayer = millis();
  lastTimeCars = lastTimePlayer;
  lastTimeMoved = lastTimePlayer;
  lastTimeFog = lastTimePlayer;
  positionLine = 7;
  positionColumn = 3;
  gameMode = 0;
  playerLed = 0;
  fogLed = 0;
  carLed = 0;
  pinMode(V0_PIN, OUTPUT);
  pinMode(X_IN, INPUT);
  pinMode(Y_IN, INPUT);
  pinMode(BUTTON_IN, INPUT);
  analogWrite(V0_PIN, 90);
  generateFirstLayoutBuildings(buildings, buildingsLine);
  generateVehicles(vehicles);
  printBuildings(buildings);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // if (start == clicked) gameMode = 1;
  gameMode = 1;
  if (gameMode == 1)
  {
    unsigned long currentTime = millis();
    if (playerLed == 0 && currentTime - lastTimePlayer > 100)
    {
      playerLed = 1;
      ledControl.setLed(0, positionColumn, positionLine, playerLed);
      lastTimePlayer = currentTime;
    }
    else if (currentTime - lastTimePlayer > 100)
    {
      playerLed = 0;
      ledControl.setLed(0, positionColumn, positionLine, playerLed);
      lastTimePlayer = currentTime;
    }
    if (fogLed == 0 && currentTime - lastTimeFog > 1000)
    {
      fogLed = 1;
      printFog(fogLed);
      lastTimeFog = currentTime;
    }
    else if (currentTime - lastTimeFog > 1000)
    {
      fogLed = 0;
      printFog(fogLed);
      lastTimeFog = currentTime;
    }
    if (carLed == 0 && currentTime - lastTimeCars > 200)
    {
      carLed = 1;
      printVehicles(vehicles);
    }
    else if (currentTime - lastTimeCars > 200)
    {
      carLed = 0;
      for(int i = 0; i < 8; i++)
        ledControl.setLed(0, i, 5, false);
    }
    if (currentTime - lastTimeMoved > carSpeed * 150)
    {
      carOffset = (carOffset + carDirection) % 8;
      lastTimeMoved = currentTime;
    }
    /*if (positionLine < 4)
    {
      translateThreeRowsDown(buildings, vehicles);
      positionLine = positionLine + 3;
    }*/
  }
}
