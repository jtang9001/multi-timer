#define MODE_BTN 1
#define BTN_1 0
#define BTN_2 2

#include <TinyWireM.h>
#include <Tiny4kOLED.h>
// #include <ADCTouch.h>
#include <CapacitiveSensor.h>


enum states {
  MODE_SEL,
  RUBIKS_ENTRY,
  RUBIKS_READY,
  RUBIKS_RUN,
  RUBIKS_STOP,
  CHESS_SEL_MAIN_TIME,
  CHESS_SEL_INCR_TIME,
  CHESS_READY,
  CHESS_CLOCK1_RUN,
  CHESS_CLOCK2_RUN,
  CHESS_END
};

double dispTime1 = 0.0;
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long lastClockTick;
unsigned long timeElapsed;
unsigned long currentTime;

byte chessMainTime = 5;
byte chessIncrTime = 10;

bool btn1Pressed;
bool btn2Pressed;

CapacitiveSensor cs_0 = CapacitiveSensor(1, 0);
CapacitiveSensor cs_2 = CapacitiveSensor(1, 2);

enum states state = MODE_SEL;


void setup() {

  oled.begin(128, 32, sizeof(tiny4koled_init_128x32r), tiny4koled_init_128x32r);
  oled.setFont(FONT8X16);
  oled.clear();
  oled.on();
  oled.switchRenderFrame();

  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);

  initState(state);
}

void loop() {
  maintainState();
  updateState();
}

void maintainState() {
  switch (state)
  {
  // case MODE_SEL:
  //   oled.setCursor(64,2);
  //   oled.print(F("      "));
  //   oled.setCursor(64,2);
  //   oled.print(ADCTouch.read(A5, 100));
  //   oled.switchFrame();
  //   break;

  case RUBIKS_RUN:
    timeElapsed = millis() - timer1;
    dispTime1 = timeElapsed / 1000.0;
    oled.setCursor(0,0);
    oled.print(dispTime1);
    oled.switchFrame();
    break;

  case CHESS_SEL_MAIN_TIME:
    btn1Pressed = !digitalRead(BTN_1);
    btn2Pressed = !digitalRead(BTN_2);
    if (btn1Pressed) {
      chessMainTime++;
      printChessSetting(true);
    }
    else if (btn2Pressed) {
      chessMainTime--;
      printChessSetting(true);
    }
    break;
    
  case CHESS_SEL_INCR_TIME:
    btn1Pressed = !digitalRead(BTN_1);
    btn2Pressed = !digitalRead(BTN_2);
    if (btn1Pressed) {
      chessIncrTime++;
      printChessSetting(false);
    }
    else if (btn2Pressed) {
      chessIncrTime--;
      printChessSetting(false);
    }
    break;

  case CHESS_CLOCK1_RUN:
    currentTime = millis();
    timeElapsed = currentTime - lastClockTick;
    if (timeElapsed >= timer1) {
      timer1 = 0;
      state = CHESS_END;
    }
    else {
      timer1 = timer1 - timeElapsed;
      lastClockTick = currentTime;
    }
    printChessTime(1);
    break;
  
  case CHESS_CLOCK2_RUN:
    currentTime = millis();
    timeElapsed = currentTime - lastClockTick;
    if (timeElapsed >= timer2) {
      timer2 = 0;
      state = CHESS_END;
    }
    else {
      timer2 = timer2 - timeElapsed;
      lastClockTick = currentTime;
    }
    printChessTime(2);
    break;

  default:
    break;
  }
}

void updateState() {
  bool modeBtnPressed = !digitalRead(MODE_BTN);
  btn1Pressed = !digitalRead(BTN_1);
  btn2Pressed = !digitalRead(BTN_2);
  enum states oldState = state;

  switch (state)
  {
  case MODE_SEL:
    if (btn1Pressed) {
      state = RUBIKS_ENTRY;
    }
    else if (btn2Pressed) {
      state = CHESS_SEL_MAIN_TIME;
    }
    break;
  
  case RUBIKS_ENTRY:
    if (btn1Pressed && btn2Pressed) {
      state = RUBIKS_READY;
    }
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }    
    break;

  case RUBIKS_READY:
    if (!btn1Pressed && !btn2Pressed) {
      state = RUBIKS_RUN;
    }
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;

  case RUBIKS_RUN:
    if (btn1Pressed && btn2Pressed) {
      state = RUBIKS_STOP;
    }
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;

  case RUBIKS_STOP:
    if (!btn1Pressed && !btn2Pressed) {
      state = RUBIKS_ENTRY;
    }
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;

  case CHESS_SEL_MAIN_TIME:
    if (modeBtnPressed) {
      state = CHESS_SEL_INCR_TIME;
      delay(100);
    }   
    break;
  
  case CHESS_SEL_INCR_TIME:
    if (modeBtnPressed) {
      state = CHESS_READY;
      delay(100);
    }   
    break;
  
  case CHESS_READY:
    if (btn1Pressed) {
      state = CHESS_CLOCK2_RUN;
    }  
    else if (btn2Pressed) {
      state = CHESS_CLOCK1_RUN;
    }  
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;

  case CHESS_CLOCK1_RUN:
    if (btn1Pressed) {
      timer1 = timer1 + chessIncrTime * 1000;
      state = CHESS_CLOCK2_RUN;
    }     
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;

  case CHESS_CLOCK2_RUN:
    if (btn2Pressed) {
      timer2 = timer2 + chessIncrTime * 1000;
      state = CHESS_CLOCK1_RUN;
    }   
    else if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;

  case CHESS_END:
    if (modeBtnPressed) {
      state = MODE_SEL;
    }   
    break;
  
  default:
    break;
  }

  if (state != oldState) {
    initState(state);
  }
}

void initState(states state) {
  switch (state)
  {
  case MODE_SEL:
    oled.clear();
    oled.switchFrame();
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("1 for Rubiks"));
    oled.setCursor(0,2);
    oled.print(F("2 for chess"));
    oled.switchFrame();
    break;

  case RUBIKS_ENTRY:
    oled.setCursor(0,2);
    oled.print(F("Hands on btns"));
    oled.switchFrame();
    break;

  case RUBIKS_READY:
    dispTime1 = 0;
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("0.00"));
    oled.switchFrame();
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("0.00"));
    oled.switchFrame();
    break;

  case RUBIKS_RUN:
    timer1 = millis();
    break;

  case RUBIKS_STOP:
    oled.setCursor(0,0);
    oled.print(dispTime1);
    oled.switchFrame();
    break;

  case CHESS_SEL_MAIN_TIME:
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("Main Time (mins)"));
    oled.setCursor(0,2);
    oled.print(chessMainTime);
    oled.switchFrame();
    
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("Main Time (mins)"));
    oled.setCursor(0,2);
    oled.print(chessMainTime);
    oled.switchFrame();
    break;
  
  case CHESS_SEL_INCR_TIME:
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("Incr Time (sec)"));
    oled.setCursor(0,2);
    oled.print(chessIncrTime);
    oled.switchFrame();

    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("Incr Time (sec)"));
    oled.setCursor(0,2);
    oled.print(chessIncrTime);
    oled.switchFrame();
    break;
  
  case CHESS_READY:
    oled.clear();
    oled.switchFrame();
    oled.clear();
    oled.switchFrame();
    timer1 = chessMainTime * 60000;
    timer2 = chessMainTime * 60000;
    printChessTime(1);
    printChessTime(1);
    printChessTime(2);
    printChessTime(2);
    break;

  case CHESS_CLOCK1_RUN:
    printChessTime(2);
    printChessTime(2);
    lastClockTick = millis();
    break;

  case CHESS_CLOCK2_RUN:
    printChessTime(1);
    printChessTime(1);
    lastClockTick = millis();
    break;
  
  default:
    break;
  }
}

void printChessTime(byte timeIdx) {
  oled.setCursor( (timeIdx == 1) ? 0 : 64 , 0 );
  oled.print(F("      "));
  oled.setCursor( (timeIdx == 1) ? 0 : 64 , 0 );

  unsigned long timeToPrint = (timeIdx == 1) ? timer1 : timer2;

  oled.print(timeToPrint/60000);
  oled.print(F(":"));
  if ((timeToPrint % 60000)/1000 < 10) {
    oled.print(F("0"));
  }
  oled.print((timeToPrint % 60000)/1000);

  oled.switchFrame();
}

void printChessSetting(bool printMainTime) {
  oled.setCursor(0,2);
  oled.print(F("    "));
  oled.setCursor(0,2);
  oled.print(printMainTime ? chessMainTime : chessIncrTime);
  oled.switchFrame();
  delay(100);
}