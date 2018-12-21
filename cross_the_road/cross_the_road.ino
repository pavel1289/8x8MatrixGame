#include "LedControl.h"
#include <LiquidCrystal.h>
#define V0_PIN 9
#define X_IN A0
#define Y_IN A1
#define MATRIX_SIZE 8
#define TIME_PER_LEVEL 60000                                            //1 minute per level, cars move faster
LedControl ledControl = LedControl(12, 11, 10, 1);
LiquidCrystal liquidCrystalDisplay(2, 3, 4, 5, 6, 7);
bool buildings[MATRIX_SIZE][MATRIX_SIZE];
bool vehicles[MATRIX_SIZE][MATRIX_SIZE];
//bool buildingsLine[8];                                                //these 2 arrays were intended to be of use if the scroll were to be done 
//bool vehiclesLine[8];                                                 //row by row
bool gameMode, playerLed, fogLed, carLed, textBlink;
bool writtenMenuState, highscoreMoved;
bool scoreLevelChanged;
int carDirection;
int positionLine, positionColumn;                                       //variables for the position of the player
int carSpeed;                                                           //variables for the movement of the vehicles/obstacles
int positionX, positionY;                                               //variables for JoyStick
int menuState, currentCursorColumn, currentCursorLine;                  //0 - Start/Highscore, 1 - Highscore, 2 - Currently playing
int cursorScoreLine;
int score;
unsigned long lastTimePlayer, lastTimeCars, lastTimeMoved, lastTimeFog, lastTimePlayerMoved, lastMenuMoved, lastLcdBlink, timePlayed;
struct Highscore
{
  int score;
  char playerName[9];
};
Highscore highscore[11];

void printBuildings(bool buildings[MATRIX_SIZE][MATRIX_SIZE])
{
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
    ledControl.setLed(0, j, i, buildings[i][j]);
}

void printVehicles(bool vehicles[MATRIX_SIZE][MATRIX_SIZE])
{
  for(int i = 0; i < 8; i++)
    ledControl.setLed(0, i, 5, vehicles[5][i]);
}

bool checkCollision(bool vehicles[MATRIX_SIZE][MATRIX_SIZE])
{
  if (positionLine == 5)
    for(int i = 0; i < 8; i++)
      if (vehicles[5][positionColumn] == 1)
        return 1;
  return 0;
}

void printFog(bool on)
{
  for(int i = 0; i < MATRIX_SIZE; i++)
    ledControl.setLed(0, i, 2, on);
}

void generateFirstLayoutBuildings(bool buildings[MATRIX_SIZE][MATRIX_SIZE])//, bool buildingsLine[8])
{
  int tunnel, safePoint;
  carDirection = 0;
  for (int i = 0; i < 8; i++)
  {
    if (i != 3)
    {
      buildings[7][i] = 1;
      buildings[6][i] = 1;
    }
    else
    {
      buildings[7][i] = 0;
      buildings[6][i] = 0;
    }
  }
  /*buildingsLine[7] = 1;
  buildingsLine[6] = 1;
  buildingsLine[5] = 0;
  buildingsLine[4] = 1;
  buildingsLine[3] = 1;
  buildingsLine[2] = 0;
  buildingsLine[1] = 1;
  buildingsLine[0] = 1;*/
  for (int i = 0; i < 2; i++)
  {
    tunnel = random(0, 8);
    safePoint = random(0, 8);
    while (tunnel == safePoint || tunnel + 1 == safePoint || tunnel - 1 == safePoint)
      safePoint = random(0, 8);
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

void initialize(bool buildings[MATRIX_SIZE][MATRIX_SIZE], bool vehicles[MATRIX_SIZE][MATRIX_SIZE])//, bool buildingsLine[8], bool vehiclesLine[8])
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

void generateBuildings(bool buildings[MATRIX_SIZE][MATRIX_SIZE])//, bool buildingsLine[8])
{
  int tunnel;
  int safePoint;

  tunnel = random(0, 8);
  safePoint = random(0, 8);
  while (tunnel == safePoint || tunnel + 1 == safePoint || tunnel - 1 == safePoint)
    safePoint = random(0, 8);
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
  for(int i = 0; i < 8; i++)
    if (buildings[3][i] == 0)
    {
      tunnel = i;
      break;
    }
  if (tunnel < positionColumn)
    carDirection = -1;
  else
    carDirection = 1;
}

void generateVehicles(bool vehicles[MATRIX_SIZE][MATRIX_SIZE])
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
  numberOfVehicles = random(0, 2);
  if (numberOfVehicles == 0)
  {
    vehicleLength[0] = random(1, 7);
    for(int i = 0; i < vehicleLength[0]; i++)
      vehicles[5][i] = 1;
  }
  else
  {
    vehicleLength[0] = random(1, 3);
    vehicleLength[1] = random(1, 4);
    for(int i = 0; i < 2; i++)
      for(int j = 0; j < vehicleLength[i]; j++)
        vehicles[5][i * 4 + j] = 1;
  }
}

void translateThreeRowsDown(bool buildings[MATRIX_SIZE][MATRIX_SIZE], bool vehicles[MATRIX_SIZE][MATRIX_SIZE])//, bool buildingsLine[8], bool vehiclesLine[8])
{
  for(int i = 7; i > 2; i--)
  {
    for(int j = 0; j < 8; j++)
    {
      buildings[i][j] = buildings[i - 3][j];
      //vehicles[i][j] = vehicles[i + 3][j];
    }
    /*buildingsLine[i] = buildingsLine[i + 3];
    vehiclesLine[i] = vehiclesLine[i + 3];*/
  }
  generateBuildings(buildings);
  generateVehicles(vehicles);
  printBuildings(buildings);
  printVehicles(vehicles);
}

void carOffset(bool vehicles[MATRIX_SIZE][MATRIX_SIZE])
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
  printVehicles(vehicles);
}

void getName()
{
  int currentCursor = 0, atLeast3Letters = 0;
  unsigned long currentTime, lastTime;
  lastTime = millis();
  liquidCrystalDisplay.clear();
  liquidCrystalDisplay.setCursor(0, 1);
  liquidCrystalDisplay.print("Score");
  liquidCrystalDisplay.setCursor(6, 1);
  liquidCrystalDisplay.print(highscore[10].score);
  liquidCrystalDisplay.setCursor(0, 0);
  for(int i = 0; i < 8; i++)
    highscore[10].playerName[i] = '\0';
  highscore[10].playerName[0] = 'A';
  liquidCrystalDisplay.print(highscore[10].playerName[0]);
  while(1)
  {
    currentTime = millis();
    if (textBlink == 0 && currentTime - lastLcdBlink > 500)
    {
      lastLcdBlink = currentTime;
      textBlink = 1;
      liquidCrystalDisplay.setCursor(currentCursor, 0);
      liquidCrystalDisplay.print(" ");
    }
    else if (currentTime - lastLcdBlink > 500)
    {
      lastLcdBlink = currentTime;
      textBlink = 0;
      liquidCrystalDisplay.setCursor(currentCursor, 0);
      liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
    }
    if (currentTime - lastTime > 300)
    {
      lastTime = currentTime;
      positionX = analogRead(X_IN);
      positionY = analogRead(Y_IN);
      if (positionX < 480)
      {
        if (currentCursor == 0)
        {
          if (atLeast3Letters == 1)
          {
            liquidCrystalDisplay.setCursor(0, 0);
            return ;
          }
        }
        else
        {
          liquidCrystalDisplay.setCursor(currentCursor, 0);
          liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
          currentCursor--;
          liquidCrystalDisplay.setCursor(currentCursor, 0);
          liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
        }
      }
      else if (positionX > 550)
      {
        if (atLeast3Letters == 0 && currentCursor == 1)
          atLeast3Letters = 1;
        if (currentCursor < 7)
        {
          liquidCrystalDisplay.setCursor(currentCursor, 0);
          liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
          currentCursor++;
          if (highscore[10].playerName[currentCursor] == '\0')
            highscore[10].playerName[currentCursor] = 'A';
          liquidCrystalDisplay.setCursor(currentCursor, 0);
          liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
        }
      }
      else if (positionY < 480)
      {
        highscore[10].playerName[currentCursor] = 'A' + (highscore[10].playerName[currentCursor] - 'A' - 1 + 'Z' - 'A' + 1) % ('Z' - 'A' + 1);
        liquidCrystalDisplay.setCursor(currentCursor, 0);
        liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
      }
      else if (positionY > 550)
      {
        highscore[10].playerName[currentCursor] = 'A' + (highscore[10].playerName[currentCursor] - 'A' + 1) % ('Z' - 'A' + 1);
        liquidCrystalDisplay.setCursor(currentCursor, 0);
        liquidCrystalDisplay.print(highscore[10].playerName[currentCursor]);
      }
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
  highscore[10].score = 0;
  strcpy(highscore[10].playerName, "NONE");
}

void initializeHighscore()
{
  for(int i = 0; i < 11; i++)
  {
    highscore[i].score = 0;
    for(int j = 0; j < 8; j++)
      highscore[i].playerName[j] = '\0';
    strcpy(highscore[i].playerName, "NONE");
    highscore[i].playerName[8] = '\0';
  }
}

void initializeLedControl()
{
  ledControl.shutdown(0, false);
  ledControl.setIntensity(0, 2);
  ledControl.clearDisplay(0);
}

void initializeLCD()
{
  liquidCrystalDisplay.begin(16, 2);
  liquidCrystalDisplay.clear();
  liquidCrystalDisplay.setCursor(0, 0);
  liquidCrystalDisplay.print("Start");
  liquidCrystalDisplay.setCursor(0, 1);
  liquidCrystalDisplay.print("Highscore");
  liquidCrystalDisplay.setCursor(0, 0);
}

void initializeTimers()
{
  lastTimePlayer = millis();
  lastTimeCars = lastTimePlayer;
  lastTimeMoved = lastTimePlayer;
  lastTimeFog = lastTimePlayer;
  lastMenuMoved = lastTimePlayer;
  lastLcdBlink = lastTimePlayer;
}

void generateFirstLayout()
{
  generateFirstLayoutBuildings(buildings);//, buildingsLine);
  generateVehicles(vehicles);
  printBuildings(buildings);
}

void initializeVariables()
{
  carSpeed = 10;
  positionLine = 7;
  positionColumn = 3;
  gameMode = 0;
  playerLed = 0;
  fogLed = 0;
  carLed = 0;
  textBlink = 0;
  menuState = 0;
  writtenMenuState = 0;
  highscoreMoved = 0;
  scoreLevelChanged = 0;
  score = 0;
  currentCursorLine = 0;
}

void playing(unsigned long currentTime)
{
    if (playerLed == 0 && currentTime - lastTimePlayer > 100)               //blinking the led for player
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
    /*if (fogLed == 0 && currentTime - lastTimeFog > 1000)                    //blinking the second road
    {
      fogLed = 1;
      printFog(fogLed);                                                       //this is commented because some players might not want to have a row
      lastTimeFog = currentTime;                                              //from the matrix blinking, given the fact that it represents the
    }                                                                         //second road that can't be "seen"
    else if (currentTime - lastTimeFog > 1000)
    {
      fogLed = 0;
      printFog(fogLed);
      lastTimeFog = currentTime;
    }*/
    /*if (carLed == 0 && currentTime - lastTimeCars > 200)                    //blinking the vehicles
    {
      carLed = 1;
      printVehicles(vehicles);
    }                                                                         //blinking the vehicles won't work mainly because the harder it gets
    else if (currentTime - lastTimeCars > 200)                                //the faster the cars move so it would interfere with their movement
    {
      carLed = 0;
      for(int i = 0; i < 8; i++)
        ledControl.setLed(0, i, 5, false);
    }*/
    if (checkCollision(vehicles) == 1)
    {
      gameMode = 0;
      for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
          ledControl.setLed(0, j, i, false);
    }
    else
    {
      if (currentTime - lastTimeMoved > carSpeed * 150)
      {
        carOffset(vehicles);
        lastTimeMoved = currentTime;
      }
      if (currentTime - lastTimePlayerMoved > 200)                                      // every 200 ms reading for a player movement
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
            if (vehicles[5][positionColumn - 1] == 0)
            {
              positionColumn--;
              if (positionColumn < 0)
                positionColumn = 0;
            }
          }
        }
        else if (positionX > 550)
        {
          //move right
          if (positionLine == 5)
          {
            if (vehicles[5][positionColumn + 1] == 0)
            {
              positionColumn++;
              if (positionColumn > 7)
                positionColumn = 7;
            }
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
        ledControl.setLed(0, positionColumn, positionLine, true);
      }
      if (positionLine < 4)                                                     //advance with the game once the first road is cleared
      {
        translateThreeRowsDown(buildings, vehicles);
        positionLine = 7;
        score = score + 5;
        scoreLevelChanged = 1;
      }
      if (currentTime - timePlayed > TIME_PER_LEVEL)
      {
        timePlayed = currentTime;
        if (carSpeed > 1)
        {
          carSpeed--;
          scoreLevelChanged = 1;
        }
      }
    }
    scoreLevelLCD();
}

void menu(unsigned long currentTime)
{
    if (score != 0)
    {
      highscore[10].score = score;
      getName();
      sortHighscore();
      delay(200);
      score = 0;
      menuState = 1;
      cursorScoreLine = 0;
    }
    if (menuState == 0)
    {
      if (writtenMenuState == 0)
      {
        initializeLCD();
        writtenMenuState = 1;
      }
      if (textBlink == 0 && currentTime - lastLcdBlink > 500)
      {
        lastLcdBlink = currentTime;
        textBlink = 1;
        liquidCrystalDisplay.setCursor(0, currentCursorLine);
        if (currentCursorLine == 0)
          liquidCrystalDisplay.print("Start");
        else
          liquidCrystalDisplay.print("Highscore");
      }
      else if (currentTime - lastLcdBlink > 500)
      {
        lastLcdBlink = currentTime;
        textBlink = 0;
        liquidCrystalDisplay.clear();
        liquidCrystalDisplay.setCursor(0, 1 - currentCursorLine);
        if (currentCursorLine == 0)
          liquidCrystalDisplay.print("Highscore");
        else
          liquidCrystalDisplay.print("Start");
      }
      if (currentTime - lastMenuMoved > 500)
      {
        lastMenuMoved = currentTime;
        positionX = analogRead(X_IN);
        positionY = analogRead(Y_IN);
        if (positionX > 550)
        {
          if (currentCursorLine == 0)
            menuState = 2;
          else
          {
            menuState = 1;
            cursorScoreLine = 0;
            writtenMenuState = 0;
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
      if (writtenMenuState == 0)
      {
        liquidCrystalDisplay.begin(16, 2);
        writtenMenuState = 1;
        liquidCrystalDisplay.clear();
      }
      if (cursorScoreLine == 9)
        cursorScoreLine--;
      if (highscoreMoved == 1)
      {
        liquidCrystalDisplay.clear();
        highscoreMoved = 0;
      }
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
        if (tmpScore == 0)
        {
          liquidCrystalDisplay.setCursor(15, j);
          liquidCrystalDisplay.print(tmpScore % 10);
        }
        for(int i = 15; i >= 12 && tmpScore != 0; i--)
        {
          liquidCrystalDisplay.setCursor(i, j);
          liquidCrystalDisplay.print(tmpScore % 10);
          tmpScore /= 10;
        }
        cursorScoreLine++;
      }
      cursorScoreLine -= 2;
      if (currentTime - lastMenuMoved > 300)
      {
        lastMenuMoved = currentTime;
        positionX = analogRead(X_IN);
        positionY = analogRead(Y_IN);
        if (positionX < 480)
        {
          menuState = 0;
          writtenMenuState = 0;
          cursorScoreLine = 0;
          highscoreMoved = 0;
        }
        else if (positionY < 480)
        {
          if (cursorScoreLine > 0)
          {
            cursorScoreLine--;
            highscoreMoved = 1;
          }
        }
        else if (positionY > 550)
        {
          if (cursorScoreLine < 9)
          {
            cursorScoreLine++;
            highscoreMoved = 1;
          }
        }
      }
    }
    if (menuState == 2)
    {
      liquidCrystalDisplay.begin(16, 2);
      liquidCrystalDisplay.clear();
      liquidCrystalDisplay.setCursor(0, 0);
      liquidCrystalDisplay.print("Level 1");
      liquidCrystalDisplay.setCursor(0, 1);
      liquidCrystalDisplay.print("Score 0");
      generateFirstLayout();
      printBuildings(buildings);
      initializeTimers();
      initializeVariables();
      gameMode = 1;
    }
}

void scoreLevelLCD()
{
  if (scoreLevelChanged == 1)
  {
    liquidCrystalDisplay.clear();
    liquidCrystalDisplay.setCursor(0, 0);
    liquidCrystalDisplay.print("Level");
    liquidCrystalDisplay.setCursor(6, 0);
    liquidCrystalDisplay.print(11 - carSpeed);
    liquidCrystalDisplay.setCursor(0, 1);
    liquidCrystalDisplay.print("Score");
    liquidCrystalDisplay.setCursor(6, 1);
    liquidCrystalDisplay.print(score);
    scoreLevelChanged = 0;
  }
}

void setup() {
  initializeHighscore();
  initializeLedControl();
  initializeLCD();
  initializeTimers();
  initializeVariables();
  pinMode(V0_PIN, OUTPUT);
  pinMode(X_IN, INPUT);
  pinMode(Y_IN, INPUT);
  analogWrite(V0_PIN, 90);
  randomSeed(analogRead(A5));
}

void loop() {
  unsigned long currentTime = millis();
  if (gameMode == 1)
  {
    playing(currentTime);
  }
  else
  {
    menu(currentTime);
  }
}
