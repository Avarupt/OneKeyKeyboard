/*
  Debounce

  Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
  press), the output pin is toggled from LOW to HIGH or HIGH to LOW.  There's
  a minimum delay between toggles to debounce the circuit (i.e. to ignore
  noise).

  created 21 November 2006
  by David A. Mellis
  modified 3 Jul 2009
  by Limor Fried
  modified 15 Jul 2010
  by Bryan Newbold; thanks adamfeuer!rocket99
*/

#include "ConstDefines.h"


// Variables:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
int lastDebounceTime = 0;   // the last time the output pin was toggled
int debounceDelay = 50;     // the debounce time; increase if the output flickers

#include "Keyboard.h"
#define KEY_PIN A0



int debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 150;            // max ms between clicks for a double click event
int holdTime = 500;        // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000;    // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = HIGH;   // value read from button
boolean buttonLast = HIGH;  // buffered value of the button's previous state
boolean DCwaiting = false;  // whether we're waiting for a double click (down)
boolean DConUp = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;    // whether it's OK to do a single click
long downTime = -1;         // time the button was pressed down
long upTime = -1;           // time the button was released
boolean ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;        // when held, whether to wait for the up event
boolean holdEventPast = false;    // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already



unsigned long timeLastTouched = 0;
boolean withheldEventPast = true;
boolean longWithheldEventPast = true;

#define MORSE 0
#define ASCII 1
#define WTF 2
#define GAME 3

int currentWTF = 0;

int mode = GAME;

//ASCII vars
unsigned long timeLastAscii=0;
char currentAscii = 'a';
//MORSE vars
String morseBuffer = "";
char prevChar = ' ';
//GAME vars
boolean isMoving = false;
#define FORWARD 0
#define RIGHT 1
#define BACKWARD 2
#define LEFT 3
int direction = FORWARD;


int red = 9; //150 ohm
int green = 11; //100 ohm
int blue = 10; //100 ohm
int redBrightness = 0;
int greenBrightness = 0;
int blueBrightness = 0;

byte wheelPos = 0;
long timeSinceLastUpdate = 0;
long rgbDelay = 5;


void gameMove(){
  if(isMoving){
        Keyboard.releaseAll();
        isMoving = false;
      }else{
        if(direction==FORWARD){
          Keyboard.press('w');
          isMoving = true;
        }
      }
}

void clicked(){
   if(mode == MORSE){
   morseBuffer+=".";
   Keyboard.print(".");
   }else if(mode == ASCII){

    

      if(millis()<1000+timeLastAscii){
        
        Keyboard.print("\b");
        if(currentAscii == 'z'){
          currentAscii = 'A';
        }else if(currentAscii == 'Z'){
          currentAscii = '0';
        }else if(currentAscii == '9'){
          currentAscii = ' ';
        }else if(currentAscii == '\b'){
          currentAscii = 'a';
        }else if(currentAscii == ' '){
          currentAscii = '\b';
        }else{        
          currentAscii++;
        }
      }else{
        currentAscii = 'a';
      }
      timeLastAscii=millis();
      Keyboard.print(currentAscii);

     
   }else if(mode == WTF){
    backspace(frames[currentWTF].length()+1);
    Keyboard.println(frames[currentWTF]);
    currentWTF++;
    if(currentWTF>=numFrames){
      currentWTF = 0;
    }
    
   }else if(mode==GAME){
      gameMove();
      
   }
}
void doubleClicked(){
  if(mode == MORSE){
  morseBuffer+="..";
  Keyboard.print("..");
  }else if(mode==GAME){
  isMoving = false;
  //Keyboard.releaseAll();
  
  gameMove();
  Keyboard.press(' ');
  delay(100);
  Keyboard.release(' ');
  
  }else{
    clicked();
    clicked();
   }
}

void held(){
  if(mode == MORSE){
      
      morseBuffer+="-";
      Keyboard.print("-");
   }else if(mode == WTF){
      Keyboard.print(copyPasta);
   }else if(mode == GAME){
    
   }
   
}

void longHeld(){
    DCgap = 150; 
    backspace(morseBuffer.length());
    prevChar = '\b';
    morseBuffer = "";
   if(mode==MORSE){
    Keyboard.print("Mode: ASCII");
    mode = ASCII;
    delayBlinkLed(0,255,0,20,100);
    backspace(11);
   }else if(mode == ASCII){
    Keyboard.print("Mode: WTF");
    mode = WTF;
    delayBlinkLed(0,0,255,20,100);
    backspace(9);
   }else if(mode == WTF){
    Keyboard.print("Mode: GAME");
    mode = GAME;
    delayBlinkLed(255,255,0,20,100);
    backspace(10);
   }else if(mode == GAME){
    DCgap = 300; 
    Keyboard.print("Mode: MORSE");
    mode = MORSE;
        delayBlinkLed(255,0,0,20,100);

    backspace(11);
   }
   
   
   
   
}

void withheld(){
  if(mode == MORSE){
  backspace(morseBuffer.length());
  prevChar = '\b';

  //Keyboard.print(morseBuffer);
  for(int i = 0; i<sizeMorseStrings; i++){
    if(morseStrings[i].equals(morseBuffer)){
      Keyboard.print(charMap[i]);
      prevChar = charMap[i];
      break;
    }
  }
  morseBuffer = "";
}else{
    
   }
}
void longWithheld(){
  if(mode == MORSE){
  if(prevChar != '\b')
    Keyboard.print(' ');
}else{
    
   }
}

void backspace(int n){
  String x = "";
  for(int i = 0; i<n; i++){
    x+="\b";
  }
  Keyboard.print(x);
}

void setup() {
    pinMode(KEY_PIN, INPUT_PULLUP);
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);
    writeLeds(0,0,0);
    Keyboard.begin(); // useful to detect host capslock state and LEDs
    Serial.begin(9600);
    delay(1000);  
}

void loop() {
updateColors();
  int event = checkButton();
  switch(event){
    case 0://nothing
    Serial.println(timeLastTouched);
    break;
    case 1://click
    Serial.println("click");
    clicked();
    break;
    case 2://double click
    Serial.println("double click");
    doubleClicked();
    break;
    case 3://
    Serial.println("short hold");
    held();
    break;
    case 4://long press
    Serial.println("long hold");
    longHeld();
    break;
    case 5:
    Serial.println("withheld");
    withheld();
    break;
    case 6:
    Serial.println("long withheld");
    longWithheld();
    break;
  }
  
  
 
}




int checkButton() {    
   int event = 0;
   buttonVal = digitalRead(KEY_PIN);
  

  
  

   // Button pressed down
   if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce)
   {    
       
       downTime = millis();
       ignoreUp = false;
       waitForUp = false;
       singleOK = true;
       holdEventPast = false;
       longHoldEventPast = false;
       if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
       else  DConUp = false;
       DCwaiting = false;

   }
   // Button released
   else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce)
   {        
      
       if (not ignoreUp)
       {
           upTime = millis();
           if (DConUp == false) DCwaiting = true;
           else
           {
               event = 2;
               DConUp = false;
               DCwaiting = false;
               singleOK = false;
           }
       }
   }
   // Test for normal click event: DCgap expired
   if ( buttonVal == HIGH && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
   {
       event = 1;
       DCwaiting = false;
   }
   // Test for hold
   if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
       // Trigger "normal" hold
       if (not holdEventPast)
       {
           event = 3;
           waitForUp = true;
           ignoreUp = true;
           DConUp = false;
           DCwaiting = false;
           //downTime = millis();
           holdEventPast = true;
       }
       // Trigger "long" hold
       if ((millis() - downTime) >= longHoldTime)
       {
           if (not longHoldEventPast)
           {
               event = 4;
               longHoldEventPast = true;
               
           }
       }
   }

  if(buttonVal == LOW){
    timeLastTouched = millis();
    withheldEventPast = false;
    longWithheldEventPast = false;
  }
  if( millis()>(timeLastTouched+1000) && !withheldEventPast &&event==0){
    withheldEventPast = true;
    event = 5;
  }
  if( millis()>(timeLastTouched+2000) && !longWithheldEventPast &&event==0){
    longWithheldEventPast = true;
    event = 6;
  }


   
   buttonLast = buttonVal;
   return event;
}


void updateColors(){
  
  if(millis()-timeSinceLastUpdate>rgbDelay){
    timeSinceLastUpdate = millis();
    wheelPos++;
    linearFade(2);
    if(digitalRead(KEY_PIN)==0){
      Wheel(wheelPos);
    }

  }
}

void delayBlinkLed(int r, int b, int g,  int numTimes, int blinkPeriod){
  for(int i = 0; i<numTimes;i++){
    writeLeds(r,g,b);
    delay(blinkPeriod/2);
    writeLeds(0,0,0);
    delay(blinkPeriod/2);
  }
  
}




void writeLeds(int r, int g, int b){
  redBrightness = r;
  greenBrightness = g;
  blueBrightness = b;
  if(redBrightness<0){
    redBrightness=0;
  }
  if(greenBrightness<0){
    greenBrightness=0;
  }
  if(blueBrightness<0){
    blueBrightness=0;
  }
    
    analogWrite(red, 255-redBrightness);
    analogWrite(green, 255-greenBrightness);
    analogWrite(blue, 255-blueBrightness);
}


void linearFade(int amount){
  redBrightness-=amount;    
  greenBrightness-=amount;
  blueBrightness-=amount;
  writeLeds(redBrightness,greenBrightness,blueBrightness);
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   redBrightness = WheelPos * 3;
   greenBrightness = 255 - WheelPos * 3;
   blueBrightness = 0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   redBrightness = 255 - WheelPos * 3;
   greenBrightness =0;
   blueBrightness = WheelPos * 3;
  } else {
   WheelPos -= 170;
   redBrightness =0;
   greenBrightness=WheelPos * 3;
   blueBrightness=255 - WheelPos * 3;
  }
}

