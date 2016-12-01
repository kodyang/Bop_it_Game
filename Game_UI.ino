#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <OrbitBoosterPackDefs.h>

//variable declaration
unsigned long starttime;
unsigned long endtime;

static unsigned long count = 2500;
static int levelTime = 250;
int actionnum;
bool dorand = true;
bool dolose = true;
bool dowin = true;
bool newHighScore = false;
int actiontime = 0;
int score;
int points;

//delay times
static int delTime = 300;
static int delTone = 20;
static int delToneLose = 60;
static int resetTime = 1500;

//tone frequencies
static unsigned int freqtri1 = 880;
static unsigned int freqtri2 = 1109;
static unsigned int freqtri3 = 1319; 
static unsigned int freqlost = 262;
static unsigned int freqwin = 600;

//indentation
static unsigned int rowSize=10;
static unsigned int indL1=40;
static unsigned int indL2=15;
static unsigned int indL3=15;

char userScore [2] = "";
char highScore [2] = "";

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

//Welcoming Page
static void handlePageWelcome()
{
  OrbitOledMoveTo(indL1, 0);
  OrbitOledDrawString("BOP IT!");

  OrbitOledMoveTo(indL2, rowSize);
  OrbitOledDrawString("BTN1 to start.");

  OrbitOledMoveTo(indL3, rowSize * 2);
  OrbitOledDrawString("BTN2 to reset.");

  dolose = true;
  newHighScore = false;
  
  // if user presses button to advance to the next page
  if (gameInputState.buttons[0].isRising)
  {
    srand(millis());
    OrbitOledClearBuffer();
    OrbitOledClear();
    gameUiPage = Level;
  }

  //resets high score
  if (gameInputState.buttons[1].isRising)
  {
    write_byte (EEPROMADDR, 0, 0);    
    OrbitOledClearBuffer();
    OrbitOledClear();
    delay(resetTime);
  }
}

//Levels Page
static void handleLevelPage()
{
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("Level ");
  OrbitOledDrawChar('1' + activeGame.gameLevel);

  OrbitOledMoveTo(0,10);
  OrbitOledDrawString("Press BTN1.");

  //displays how long user has to complete a certain action in certain levels
  OrbitOledMoveTo(0,20);
  OrbitOledDrawChar('0'+(count - activeGame.gameLevel * levelTime)/1000);
  OrbitOledDrawChar('.');
  OrbitOledDrawChar('0'+(count - activeGame.gameLevel * levelTime)%1000/100);
  OrbitOledDrawChar('0'+(count - activeGame.gameLevel * levelTime)%100/10);
  OrbitOledDrawString(" s");

  //pressed button one to begin the level
  if (gameInputState.buttons[0].isRising) {
    OrbitOledClearBuffer();
    OrbitOledClear();
    gameUiPage = Instructions;
  }
}

//Instructions
static void handleInstructions() {
  OrbitOledMoveTo(0, 0);

  // gives player a random action
  if (dorand) {
    actionnum = (enum Action)(rand() % ActionCount);
    dorand = false;
    starttime = millis();
  }

  //Bop It! - press a button
  else if (actionnum == 0) {
    OrbitOledDrawString ("Bop It");

    // if either button is pressed 
    if (gameInputState.buttons[0].isRising || gameInputState.buttons[1].isRising) {
      endtime = millis();

      // making sure user completes the task with the given time frame
      if ((endtime - starttime) <= (count - activeGame.gameLevel * levelTime)) {
        OrbitOledClearBuffer();
        OrbitOledClear();
        dorand = true;
        actiontime++;
        gameUiPage = Correct;
      }
    }

    //if the user does not complete the task within the time frame or does a wrong action they lose
    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||ShakeIsShaking()||gameInputState.switches[0].isSwitching||gameInputState.switches[1].isSwitching||gameInputState.potentiometer.isTurning) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }

  //Shake It!
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

    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||gameInputState.switches[0].isSwitching||gameInputState.switches[1].isSwitching||gameInputState.potentiometer.isTurning||gameInputState.buttons[0].isRising||gameInputState.buttons[1].isRising) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }
  
  //Twist It!
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
   
    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||gameInputState.switches[0].isSwitching||gameInputState.switches[1].isSwitching||gameInputState.buttons[0].isRising|| gameInputState.buttons[1].isRising||ShakeIsShaking()) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }

  // Switch It!
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

    else if ((millis() - starttime > (count - activeGame.gameLevel * levelTime))||gameInputState.buttons[0].isRising||gameInputState.buttons[1].isRising||ShakeIsShaking()||gameInputState.potentiometer.isTurning) {
      OrbitOledClearBuffer();
      OrbitOledClear();
      gameUiPage = GameResult;
    }
  }
}

//Correct Page - when user completes the correct action within the correct timeframe
static void handleCorrect(){

  // green light will blink to give congratulations!!
  digitalWrite(GREEN_LED,HIGH);

  //makes a nice triad sound to give congratulations!!
  tone(37, freqtri1);
  delay(delTone);
  noTone(37);
  
  delay(delTone);
  
  tone(37, freqtri2);
  delay(delTone);
  noTone(37);

  delay(delTone);
  
  tone(37, freqtri3);
  delay(delTone);
  noTone(37);
  
  delay(delTime);
  digitalWrite(GREEN_LED,LOW);
  
  //takes user to the next action
  gameUiPage = Instructions;

  // if they have completed five actions they will move onto the next level
  if (actiontime == 5) {
    activeGame.gameLevel++;
    actiontime = 0;
    OrbitOledClear();
    gameUiPage = Level;
  }

  // if they complete 9 levels they have won the game!
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
  
}

//Losing Page
static void handlePageGameResult()
{ 
  digitalWrite(RED_LED, HIGH);
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("You Lost :(");
  OrbitOledMoveTo(0, 10);
  OrbitOledDrawString("Score: ");

  points = activeGame.gameLevel*5 + actiontime;

  // prints their score 

  sprintf(userScore, "%d", points);
  OrbitOledDrawString(userScore);

  OrbitOledMoveTo(0,20);
  int highest = read_byte (EEPROMADDR, 0);
 
  if (highest >= points && !newHighScore) {
    sprintf(highScore, "%d", highest);
    OrbitOledDrawString("High Score: ");
    OrbitOledDrawString (highScore);
  }
  else {
    OrbitOledDrawString("New High Score!");
    write_byte (EEPROMADDR, 0, points);
    newHighScore = true;
  }
  
  //plays a tone indicating you are wrong
  if(dolose){
    tone(37, freqlost);
    delay(delToneLose);
    noTone(37);
  
    delay(delToneLose);
    
    tone(37, freqlost);
    delay(delToneLose);
    noTone(37);
    dolose=false;
  }
  
  // Press button to restart the game
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

//Winning Page
static void handleWin()
{
  OrbitOledMoveTo(0, 0);
  OrbitOledDrawString("You Win! :)");
  OrbitOledMoveTo(0,15);
  OrbitOledDrawString("New High Score!!");

  if (dowin){
    tone (37, freqwin);
    delay(delTone);
    noTone(37);

    delay (delTone);

    tone(37,freqwin);
    delay(delTone);
    noTone(37);
  }

  if (gameInputState.buttons[0].isRising)
  {
    OrbitOledClearBuffer();
    OrbitOledClear();
    OrbitOledMoveTo(0,0);
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

