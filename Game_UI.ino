#include <stdbool.h>
#include <string.h>
#include <stdio.h>


unsigned long starttime;
unsigned long endtime;

unsigned long count = 2500;
int levelTime = 250;
int actionnum;
bool dorand = true;
int actiontime = 0;
int score;
int delTime = 500;

static enum GamePages
{
  Welcome       = 0,
  Level         = 1,
  Instructions  = 2,
  Correct       = 3,
  GameResult    = 4,
  Win           = 5,
} gameUiPage = Welcome;

const uint32_t SwitchCount = 2;
const uint32_t ButtonCount = 2; 
const uint32_t Switches[SwitchCount] = { PA_7, PA_6 };
const uint32_t Buttons[ButtonCount] = { PD_2, PE_0 };
const uint32_t Potentiometer = PE_3;
const size_t MaximumLevels = 2;

struct ButtonState
{
  bool state;
  bool isRising;
};

typedef enum Action
{
  Bop     = 0,
  Shake   = 1,
  Twist   = 2,
  Switch  = 3,
  ActionCount = 4,
} Action;


struct GameState
{
  int gameLevel;
  enum Action playerActions[MaximumLevels];
} activeGame;

struct PotState
{
  float orientation;
  bool isTurning;
};

struct SwitchState
{
  bool state;
  bool isSwitching;
};

static struct InputState
{
  struct SwitchState  switches[2];
  struct ButtonState  buttons[2];
  struct PotState     potentiometer;
} gameInputState;

void GameUIInit()
{
  OrbitOledInit();
  OrbitOledClear();
  OrbitOledClearBuffer();
  OrbitOledSetFillPattern(OrbitOledGetStdPattern(iptnSolid));
  OrbitOledSetDrawMode(modOledSet);

  gameInputState = { 0 };
  activeGame = { 0 };

  for (int i = 0; i < SwitchCount; ++i )
    pinMode(Switches[i], INPUT);
  for (int i = 0; i < ButtonCount; ++i )
    pinMode(Buttons[i], INPUT);
  gameInputState.potentiometer.isTurning = false;
}

static void handlePageWelcome()
{
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("Kody's BOP IT!");

  OrbitOledMoveTo(0, 15);
  OrbitOledDrawString("Press BTN1.");

  if (gameInputState.buttons[0].isRising)
  {
    srand(millis());
    OrbitOledClearBuffer();
    OrbitOledClear();
    gameUiPage = Level;
  }
}

static void handleLevelPage()
{
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("Level ");
  OrbitOledDrawChar('1' + activeGame.gameLevel);

  OrbitOledMoveTo(0,10);
  OrbitOledDrawString("Press BTN1.");

  OrbitOledMoveTo(0,20);
  OrbitOledDrawChar('0'+(count - activeGame.gameLevel * levelTime)/1000);
  OrbitOledDrawChar('.');
  OrbitOledDrawChar('0'+(count - activeGame.gameLevel * levelTime)%1000/100);
  OrbitOledDrawChar('0'+(count - activeGame.gameLevel * levelTime)%100/10);
  OrbitOledDrawString(" s");

  if (gameInputState.buttons[0].isRising) {
    OrbitOledClearBuffer();
    OrbitOledClear();
    gameUiPage = Instructions;
  }
}

static void handleInstructions() {
  OrbitOledMoveTo(0, 0);

  if (dorand) {
    actionnum = (enum Action)(rand() % ActionCount);
    dorand = false;
    starttime = millis();
  }

  else if (actionnum == 0) {
    OrbitOledDrawString ("Bop It");

    if (gameInputState.buttons[0].isRising) {
      endtime = millis();

      if ((endtime - starttime) <= (count - activeGame.gameLevel * levelTime)) {
        OrbitOledClearBuffer();
        OrbitOledClear();
        dorand = true;
        actiontime++;
        gameUiPage = Correct;
      }
    }

    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||ShakeIsShaking()||gameInputState.switches[0].isSwitching||gameInputState.switches[1].isSwitching||gameInputState.potentiometer.isTurning) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }
  else if (actionnum == 1) {
    OrbitOledDrawString ("Shake It");

      if(ShakeIsShaking()){
          endtime = millis();

          if ((endtime - starttime) <= (count - activeGame.gameLevel * levelTime)) {
          OrbitOledClearBuffer();
          OrbitOledClear();
          OrbitOledMoveTo(0,0);
          dorand=true;
          actiontime++;
          gameUiPage = Correct;
          }
      }

    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||gameInputState.switches[0].isSwitching||gameInputState.switches[1].isSwitching||gameInputState.potentiometer.isTurning||gameInputState.buttons[0].isRising) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }

  }
  else if (actionnum == 2){
    OrbitOledDrawString ("Twist It");

    if (gameInputState.potentiometer.isTurning) {
      endtime = millis();

      if ((endtime - starttime) <= (count - activeGame.gameLevel * levelTime)) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      dorand = true;
      actiontime++;
      gameUiPage = Correct;
      }
    }
   
    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||gameInputState.switches[0].isSwitching||gameInputState.switches[1].isSwitching||gameInputState.buttons[0].isRising||ShakeIsShaking()) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }
  else {
    OrbitOledDrawString ("Switch It");

    if (gameInputState.switches[0].isSwitching || gameInputState.switches[1].isSwitching) {
      endtime = millis();

      if ((endtime - starttime) <= (count - activeGame.gameLevel *levelTime)) {
        OrbitOledClearBuffer();
        OrbitOledClear();
        dorand = true;
        actiontime++;
        gameUiPage = Correct;
      }
    }

    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||gameInputState.buttons[0].isRising||ShakeIsShaking()||gameInputState.potentiometer.isTurning) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }
}

static void handleCorrect(){
  digitalWrite(GREEN_LED,HIGH);
  delay(delTime);
  digitalWrite(GREEN_LED,LOW);
  gameUiPage = Instructions;
  if (actiontime == 5) {
    activeGame.gameLevel++;
    actiontime = 0;
    OrbitOledClear();
    gameUiPage = Level;
  }
  
  if (activeGame.gameLevel == 9) {
    gameUiPage = Win;
    OrbitOledClear();
  }
  
}

static void uiInputTick()
{
  for (int i = 0; i < SwitchCount; ++i ){
    bool previousState = gameInputState.switches[i].state;
    gameInputState.switches[i].state = digitalRead(Switches[i]);
    if(previousState != gameInputState.switches[i].state){
      gameInputState.switches[i].isSwitching=true;  
    }else{
      gameInputState.switches[i].isSwitching=false;
    }
  }
    
  for (int i = 0; i < ButtonCount; ++i )
  {
    // Only look for Rising Edge Signals.
    bool previousState = gameInputState.buttons[i].state;
    gameInputState.buttons[i].state = digitalRead(Buttons[i]);
    gameInputState.buttons[i].isRising = (!previousState && gameInputState.buttons[i].state);
  }
  float previousOrientation = gameInputState.potentiometer.orientation;
  gameInputState.potentiometer.orientation = analogRead(Potentiometer);
  if(fabs(previousOrientation - gameInputState.potentiometer.orientation)>40.0){
    gameInputState.potentiometer.isTurning = true;
  }else{
    gameInputState.potentiometer.isTurning = false;
  }
  
//  gameInputState.potentiometer = analogRead(Potentiometer);
}


static void handlePageGameResult()
{
  digitalWrite(RED_LED, HIGH);
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("You Lost :(");
  OrbitOledMoveTo(0, 15);
  OrbitOledDrawString("Score: ");

  int points = activeGame.gameLevel*5 + actiontime, digits=1;

  if(points){
  while(points>=pow(10,digits))
    digits++;
  digits--;
  while(points){
    OrbitOledDrawChar( points/pow(10,digits) + '0');
    points-=(int)pow(10,digits)*(int)(points/pow(10,digits));
    if(digits>0 && points ==0){
      OrbitOledDrawChar('0');
    }
    if(digits>=0)
      digits--;
  }
  }
  else{
    OrbitOledDrawChar('0');
  }
  //OrbitOledDrawChar((activeGame.gameLevel*5 + actiontime) + '0');
  /*
  OrbitOledDrawChar((char)(((int)("0" + (char)(activeGame.gameLevel * 5 + actiontime) - "a"))/10));
  OrbitOledDrawChar((char)(((int)("0" + (char)(activeGame.gameLevel * 5 + actiontime) - "a"))%10));
  */
  if (gameInputState.buttons[0].isRising)
  {
    OrbitOledClearBuffer();
    OrbitOledClear();
    dorand = true;
    actiontime = 0;
    activeGame.gameLevel = 0;
    gameUiPage = Welcome;
    digitalWrite(RED_LED,LOW);
  }
}


static void handleWin()
{
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("You Win! :)");

  if (gameInputState.buttons[0].isRising)
  {
    OrbitOledClearBuffer();
    OrbitOledClear();
    dorand = true;
    actiontime = 0;
    activeGame.gameLevel = 0;
    gameUiPage = Welcome;
  }
}

void GameUITick()
{
  uiInputTick();
  switch (gameUiPage)
  {
    case Welcome:
      handlePageWelcome();
      break;

    case Level:
      handleLevelPage();
      break;

    case Instructions:
      handleInstructions();
      break;

    case Correct:
      handleCorrect();
      break;
      
    case GameResult:
      handlePageGameResult();
      break;

    case Win:
      handleWin();
      break;

  }
  OrbitOledUpdate();
}

