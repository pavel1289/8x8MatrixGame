#include "LedControl.h"
#include <LiquidCrystal.h>
#define V0_PIN 9
#define X_IN A0
#define Y_IN A1
LedControl ledControl = LedControl(12, 11, 10, 1);
LiquidCrystal liquidCrystalDisplay(2, 3, 4, 5, 6, 7);
bool buildings[8][8];
bool vehicles[8][8];
bool buildingsLine[8];
bool vehiclesLine[8];
bool gameMode, playerLed, fogLed, carLed, carDirection;
int positionLine, positionColumn;                                       //variables for the position of the player
int carSpeed;                                                           //variables for the movement of the vehicles/obstacles
int positionX, positionY;                                               //variables for JoyStick
int menuState, currentCursorColumn, currentCursorLine;                  //0 - Start/Highscore, 1 - Highscore, 2 - Currently playing
int cursorScoreLine;
int score;
unsigned long lastTimePlayer, lastTimeCars, lastTimeMoved, lastTimeFog, lastTimePlayerMoved, lastLcdBlink, lastMenuMoved;
struct Highscore
{
  int score;
  char playerName[9];
};
Highscore highscore[11];

void printBuildings(bool buildings[8][8])
{
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
    ledControl.setLed(0, j, i, buildings[i][j]);
}

void printVehicles(bool vehicles[8][8])
{
  for(int i = 0; i < 8; i++)
    ledControl.setLed(0, i, 5, vehicles[5][i]);
}

bool checkCollision(bool vehicles[8][8])
{
  if (positionLine == 5)
    for(int i = 0; i < 8; i++)
      if (vehicles[5][positionColumn] == 1)
        return 1;
  return 0;
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
  printBuildings(buildings);
}

void carOffset(bool vehicles[8][8])
{
  bool tmp[8];
  if (carDirection == 1)
  {
    for(int i = 0; i < 8; i++)
    {
      tmp[i] = 0;
      if (i == 0)
        tmp[i] = vehicles[5][7];
      else
        tmp[i] = vehicles[5][i - 1];
    }
    
  }
  else
  {
    for(int i = 0; i < 8; i++)
    {
      tmp[i] = 0;
      if (i == 7)
        tmp[i] = vehicles[5][0];
      else
        tmp[i] = vehicles[5][i + 1];
    }
  }
  for(int i = 0; i < 8; i++)
    vehicles[5][i] = tmp[i];
}

void getName()
{
  int currentCursor = 0;
  liquidCrystalDisplay.setCursor(0, 1);
  liquidCrystalDisplay.print("Score");
  liquidCrystalDisplay.setCursor(6, 1);
  liquidCrystalDisplay.print(score);
  liquidCrystalDisplay.setCursor(0, 0);
  liquidCrystalDisplay.print("A");
  highscore[10].playerName[0] = 'A';
  while(1)
  {
    positionX = analogRead(X_IN);
    positionY = analogRead(Y_IN);
    if (positionX < 480)
    {
      if (currentCursor == 0)
      {
        liquidCrystalDisplay.setCursor(0, 0);
        return ;
      }
      else
      {
        currentCursor--;
        liquidCrystalDisplay.setCursor(currentCursor, 0);
      }
    }
    else if (positionX > 550)
    {
      if (currentCursor < 7)
      {
        currentCursor++;
        liquidCrystalDisplay.setCursor(currentCursor, 0);
      }
    }
    else if (positionY < 480)
    {
      highscore[10].playerName[currentCursor] = 'A' + (highscore[10].playerName[currentCursor] - 'A' - 1 + 'Z' - 'A' + 1) % ('Z' - 'A' + 1);
      liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
    }
    else if (positionY > 550)
    {
      highscore[10].playerName[currentCursor] = 'A' + (highscore[10].playerName[currentCursor] - 'A' + 1) % ('Z' - 'A' + 1);
      liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
    }
  }
}

void sortHighscore()
{
  Highscore tmp;
  tmp = highscore[10];
  int i = 9;
  while (i >= 0)
  {
    if (tmp.score >= highscore[i].score)
      highscore[i + 1] = highscore[i];
    else
      break;
    i--;
  }
  highscore[i + 1] = tmp;
}

void initialize()
{
  for(int i = 0; i < 11; i++)
  {
    highscore[i].score = 0;
    highscore[i].playerName[0] = 'N';
    highscore[i].playerName[1] = 'O';
    highscore[i].playerName[2] = 'N';
    highscore[i].playerName[3] = 'E';
    highscore[i].playerName[8] = '\0';
  }
}

void setup() {
  initialize();
  ledControl.shutdown(0, false);
  ledControl.setIntensity(0, 2);
  ledControl.clearDisplay(0);
  liquidCrystalDisplay.begin(16, 2);
  liquidCrystalDisplay.clear();
  liquidCrystalDisplay.setCursor(0, 0);
  liquidCrystalDisplay.print("Start");
  liquidCrystalDisplay.setCursor(0, 1);
  liquidCrystalDisplay.print("Highscore");
  liquidCrystalDisplay.setCursor(0, 0);
  currentCursorLine = 0;
  carSpeed = 10;
  lastTimePlayer = millis();
  lastTimeCars = lastTimePlayer;
  lastTimeMoved = lastTimePlayer;
  lastTimeFog = lastTimePlayer;
  lastLcdBlink = lastTimePlayer;
  positionLine = 7;
  positionColumn = 3;
  gameMode = 1;
  playerLed = 0;
  fogLed = 0;
  carLed = 0;
  menuState = 0;
  score = 0;
  pinMode(V0_PIN, OUTPUT);
  pinMode(X_IN, INPUT);
  pinMode(Y_IN, INPUT);
  analogWrite(V0_PIN, 90);
  generateFirstLayoutBuildings(buildings, buildingsLine);
  generateVehicles(vehicles);
  printBuildings(buildings);
  Serial.begin(9600);
}

void loop() {
  // if (start == clicked) gameMode = 1;
  unsigned long currentTime = millis();
  if (gameMode == 1)
  {
    if (playerLed == 0 && currentTime - lastTimePlayer > 200)
    {
      playerLed = 1;
      ledControl.setLed(0, positionColumn, positionLine, playerLed);
      lastTimePlayer = currentTime;
    }
    else if (currentTime - lastTimePlayer > 200)
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
    if (checkCollision(vehicles) == 1)
    {
      gameMode = 0;
      for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
          ledControl.setLed(0, j, i, false);
    }
    if (currentTime - lastTimeMoved > carSpeed * 150)
    {
      carOffset(vehicles);
      lastTimeMoved = currentTime;
    }
    if (currentTime - lastTimePlayerMoved > 200)
    {
      lastTimePlayerMoved = currentTime;
      positionX = analogRead(X_IN);
      positionY = analogRead(Y_IN);
      ledControl.setLed(0, positionColumn, positionLine, false);
      if (positionX < 480)
      {
        //move left
        if (positionLine == 5)
        {
          positionColumn--;
          if (positionColumn < 0)
            positionColumn = 0;
        }
      }
      else if (positionX > 550)
      {
        //move right
        if (positionLine == 5)
        {
          positionColumn++;
          if (positionColumn > 7)
            positionColumn = 7;
        }
      }
      if (positionY < 480)
      {
        //move ahead
        if (positionLine == 6)
        {
          if (vehicles[5][positionColumn] == 0)
            positionLine--;
        }
        else
        {
          if (buildings[positionLine - 1][positionColumn] == 0)
            positionLine--;
        }
      }
      else if (positionY > 550)
      {
        //move backwards
        if (positionLine != 7)
        {
          if (positionLine == 4)
          {
            if (vehicles[5][positionColumn] == 0)
              positionLine++;
          }
          else if (buildings[positionLine + 1][positionColumn] == 0)
              positionLine++;
        }
      }
      ledControl.setLed(0, positionColumn, positionLine, 1);
    }
    if (positionLine < 4)
    {
      translateThreeRowsDown(buildings, vehicles);
      positionLine = positionLine + 3;
      score++;
    }
  }
  else
  {
    if (score != 0)
    {
      highscore[10].score = score;
      getName();
      sortHighscore();
      score = 0;
      menuState = 1;
      cursorScoreLine = 0;
    }
    if (menuState == 0)
    {
      liquidCrystalDisplay.setCursor(0, 0);
      liquidCrystalDisplay.print("Start");
      liquidCrystalDisplay.setCursor(0, 1);
      liquidCrystalDisplay.print("Highscore");
      liquidCrystalDisplay.setCursor(0, 0);
      if (currentTime - lastMenuMoved > 500)
      {
        lastMenuMoved = currentTime;
        positionX = analogRead(X_IN);
        positionY = analogRead(Y_IN);
        if (positionX > 550)
        {
          if (currentCursorLine == 0)
          {
            gameMode = 1;
            menuState = 2;
          }
          else
          {
            menuState = 1;
            cursorScoreLine = 0;
          }
        }
        else if (positionY < 480)
        {
          currentCursorLine = 0;
        }
        else if (positionY > 550)
        {
          currentCursorLine = 1;
        }
      }
    }
    if (menuState == 1)
    {
      for(int j = 0; j < 2; j++)
      {
        liquidCrystalDisplay.setCursor(0, j);
        liquidCrystalDisplay.print(cursorScoreLine + 1);
        if (cursorScoreLine == 9)
        {
          liquidCrystalDisplay.setCursor(2, j);
          liquidCrystalDisplay.print(".");
          liquidCrystalDisplay.setCursor(3, j);
        }
        else
        {
          liquidCrystalDisplay.setCursor(1, j);
          liquidCrystalDisplay.print(".");
          liquidCrystalDisplay.setCursor(2, j);
        }
        liquidCrystalDisplay.print(highscore[cursorScoreLine].playerName);
        int tmpScore = highscore[cursorScoreLine].score;
        for(int i = 15; i >= 12 && tmpScore != 0; i--)
        {
          liquidCrystalDisplay.setCursor(i, j);
          liquidCrystalDisplay.print(tmpScore % 10);
          tmpScore /= 10;
        }
        cursorScoreLine++;
      }
      cursorScoreLine--;
      if (currentTime - lastMenuMoved > 500)
      {
        lastMenuMoved = currentTime;
        positionX = analogRead(X_IN);
        positionY = analogRead(Y_IN);
        if (positionX < 480)
        {
          menuState = 0;
          cursorScoreLine = 0;
        }
        else if (positionY < 480)
        {
          if (cursorScoreLine > 0)
            cursorScoreLine--;
        }
        else if (positionY > 550)
        {
          if (cursorScoreLine < 9)
            cursorScoreLine++;
        }
      }
    }
    if (menuState == 2)
    {
      liquidCrystalDisplay.setCursor(0, 0);
      liquidCrystalDisplay.print("Currently");
      liquidCrystalDisplay.setCursor(0, 1);
      liquidCrystalDisplay.print("playing...");
      generateFirstLayoutBuildings(buildings, buildingsLine);
      generateVehicles(vehicles);
      printBuildings(buildings);
    }
  }
}
