//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
#include <Stepper.h>
#include <EEPROM.h>

/*******************************************************
General Variables
********************************************************/


int debug =0;
int wasRunningInt = 0;
boolean wasRunning = false;
/*******************************************************
EEPROM Variables
********************************************************/

int EEPROMKeyIndex=0;
int EEPROMCodeIndex=3;
boolean saveRunning=false;
int saveRunningInt=0;
/*******************************************************
Stepper Moter Variables
********************************************************/

//# Uno Connection Constants
int Xwire1=22; //Motor Black
int Xwire2=24; //Motor Green 
int Xwire3=26; //Motor Red
int Xwire4=28; //Motor Blue
int CurrentLocationRevolution=0;
int CurrentLocationMark=0;

//keyes L298 Controller

int L298MotAEnab = 8;
int L298L1 = 7;
int L298L2 = 6;
int L298L3 = 5;
int L298L4 = 4;
int L298MotBEnab = 3;



//#Motor Constants
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
const int stepsPerMark = stepsPerRevolution /100;
const int marksPerTest = 5;
const int stepsToOpen = -50;
const int motorSpeed=100;


int fullturn= stepsPerRevolution;
int halfturn=stepsPerRevolution/2;
int quarterturn=stepsPerRevolution/4;
int eightturn=stepsPerRevolution/8;  // for your motor

Stepper myXStepper(stepsPerRevolution, Xwire1,Xwire2,Xwire3,Xwire4);           

double xRotationDistance=0.079;




/*******************************************************
Optical Encoder Variables
********************************************************/


//Optical inputs
int debugswitch=53;

int interruptport1=2;
int switch1port=52; // HEDS-5505 pin 2

int interruptport2=3; // HEDS-5505 Pin 3
int switch2port=50; // HEDS-5505 pin 3


int interruptport3=4;
int switch3port=48; // HEDS-5505 pin 5


int interrupt1counter=0;
int interrupt2counter=0;
int interrupt3counter=0;
int intA = 0;
int intB = 0;
int int1 = 0;



/*******************************************************
LCD Shield Variables
********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
boolean buttonDown=false;
int lastButton=5;

String FirstLCDLine = "";
String SecondLCDLine = "";
/*******************************************************
Menu Related Variables
********************************************************/

int MenuID = 0;

String actions[3] = { "Start ","Cont. ","Config"};
int actionSize=3;
String currentAction="";
int currentActionInt=0;

String configactions[] = { "LockType      ","Manual Config ","Fine Tune     ","Reset         "};
int configActionSize=4;

String currentConfigAction="";
int currentConfigActionInt=0;
/*******************************************************
Safe Related Variables
********************************************************/
int currentdial1 = 0;
int currentdial2 = 0;
int currentdial3 = 0;
int currentdial4 = 0;


int wigglesize=2;
int wigglesleep=50;
int numberOfLocks = 4;
char* LockTypes[] = { "Legard 3X00   ", "S&G 6XXX      ", "Lagard 1947   ","Manual        "};
int LockTypeCount = numberOfLocks;

int UnlockPositions[] = {90,90,90,90};
int UnlockPositionCount = numberOfLocks;

int LockDials[] = {3,3,4,3};
int LockDialCount = numberOfLocks;

int WigglePositions[] = {5,5,5,5};
int WigglePositionCount = numberOfLocks;


String CurrentLockType="";
int CurrentLockTypeInt=0;
int IterationSize=5;
int lockdials = 3;
int wiggleposition = 5;

int retractposition = 90;
double  accuracymidsteps = 4;
double  allowabledeviation = 0.75;
boolean movefailed = true;


/*********************************************************************************************
*
*   **      *******     *****
*   **      *******     *******
*   **      **          **    **
*   **      **          **    **
*   ******  *******     *******
*   ******  *******     *****
*
**********************************************************************************************/

// Display Message on second line

void print_status2(String msg1, int msg2 )
{
  String _x = msg1 + msg2;
  SecondLCDLine = _x;
  clear_line2();
  lcd.setCursor(0,1);
  lcd.print(_x);
}


void print_status(String message )
{
  clear_line2();
  lcd.setCursor(0,1);
  lcd.print(message);
}

// Display Message on first line
void print_msg(String message )
{
  FirstLCDLine = message;
  SecondLCDLine = "";
  clear_lcd();
   lcd.setCursor(0,0);
  lcd.print(message);
}
void clear_lcd() {
 clear_line1();
 clear_line2();
}
void clear_line1(){
  lcd.setCursor(0,0);
  lcd.print("                ");
  FirstLCDLine="                ";
}
void clear_line2(){
  lcd.setCursor(0,1);
  lcd.print("                ");
  SecondLCDLine="                ";
}

void print_lcd (String firstline, String secondline, int secondstodelay){
  String _firstLCDLine = FirstLCDLine;
  String _secondLCDLine = SecondLCDLine;
 clear_lcd(); 
 print_msg(firstline);
 print_status(secondline);
 
 delay(secondstodelay*1000);
 print_msg( _firstLCDLine);
 print_status(_secondLCDLine);
}
// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close

 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  
 return btnNONE;  // when all others fail, return this...
}


void print_menu(){
 //clear_lcd();
//Serial.println("]");

  if (MenuID == 0 ) {
  
   lcd.setCursor(0,0);
   lcd.print("Auto Dialer v1.0");
   lcd.setCursor(0,1);
   currentAction = actions[currentActionInt];
   lcd.print( "Action:["+currentAction+"]  ");
  
  }else if (MenuID == 1) {
    
    
  }else if (MenuID == 2) {
   lcd.setCursor(0,0);
   String _x = "C:[";
   _x += currentdial1;
   _x += "][";
   _x += currentdial2;
   _x += "][";
   _x += currentdial3;
   _x += "][";
   _x += currentdial4;
   _x += "]";
   lcd.print(_x);
   lcd.setCursor(0,1);
   currentActionInt=0;
   currentAction = actions[currentActionInt];
   lcd.print( "Action:["+currentAction+"]  ");
    
    
  } else if (MenuID == 10) {
    
   lcd.setCursor(0,0);
   String _x = "Config:          ";
   lcd.print(_x);
   lcd.setCursor(0,1);
   //currentConfigActionInt=0;
   currentConfigAction = configactions[currentConfigActionInt];
   lcd.print( "["+currentConfigAction+"]  ");    
  
  } else if (MenuID == 11) {
    
   lcd.setCursor(0,0);
   String _x = "LockType:          ";
   lcd.print(_x);
   lcd.setCursor(0,1);
   //currentConfigActionInt=0;
   CurrentLockType = LockTypes[CurrentLockTypeInt];
   lcd.print( "["+CurrentLockType+"]  ");    
  }
 
 
// lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
// lcd.print(millis()/1000);      // display seconds elapsed since power-up
// lcd.setCursor(0,1);            // move to the begining of the second line 
}

int get_keys(){
  lcd_key = read_LCD_buttons();  // read the buttons  
}

void decide_action(){
  if (buttonDown && lcd_key == btnNONE){
    buttonDown=false; 
    return;
  }

   if (!buttonDown && lastButton != lcd_key)
   {
     buttonDown=true;
     lastButton = lcd_key;
   
   
   
   
   if (MenuID == 0 ) {
     
   
   
       switch (lcd_key)               // depending on which button was pushed, we perform an action
       {
         case btnRIGHT:
           {
           lcd.print("RIGHT ");
           break;
           }
         case btnLEFT:
           {
           lcd.print("LEFT   ");
           break;
           }
         case btnUP:
           {
           currentActionInt--;
           if (currentActionInt < 0){
               currentActionInt = actionSize-1;
           }
           break;
           }
         case btnDOWN:
           {
             currentActionInt++;
             if (currentActionInt >= actionSize)
                 currentActionInt = 0;     
             break;
           }
         case btnSELECT:
           {
            if (currentAction == "Start "){         
               //tryCombo(60,20,40);
              //tryCombo(60,20,30);
              //tryCombo(50,20,30);
              //tryCombo(50,50,50);
              startDialing();
            
            } else if (currentAction == "Cont. " ) {
              clear_lcd();
              MenuID = 2;
             // contDialing();
              
            }else if (currentAction == "Config" ) {
              clear_lcd();
              MenuID = 10; 
              currentAction = "_config";
              
            }
           break;
           }
           case btnNONE:
           {
           buttonDown=false;
           lcd.print("NONE  ");
           break;
           }
       }
       return;
     }
     
     
     if (MenuID == 10 ) {
       switch (lcd_key)               // depending on which button was pushed, we perform an action
       {
         case btnRIGHT:
           {
           lcd.print("RIGHT ");
           break;
           }
         case btnLEFT:
          {
              clear_lcd();
              MenuID = 0;
           break;
           }
         case btnUP:
           {
           currentConfigActionInt--;
           if (currentConfigActionInt < 0){
               currentConfigActionInt = configActionSize-1;
           }
           break;
           }
         case btnDOWN:
           {
             currentConfigActionInt++;
             if (currentConfigActionInt >= configActionSize)
                 currentConfigActionInt = 0;     
             break;
           }
         case btnSELECT:
           {
            if (currentConfigAction == configactions[0]){         
              clear_lcd();
              MenuID = 11;
        
          } else if (currentConfigAction == configactions[1] ) {
              clear_lcd();
              MenuID = 12;
             // contDialing();
              
            }else if (currentConfigAction == configactions[2] ) {
              clear_lcd();
              MenuID = 13; 
                            
            }else if (currentConfigAction == configactions[3] ) {
              clear_lcd();
              MenuID = 14; 
            }
            
            
           break;
           }
           case btnNONE:
           {
           buttonDown=false;
           lcd.print("NONE  ");
           break;
           }
       }
       return;
     }
     if (MenuID == 11 ) {
       switch (lcd_key)               // depending on which button was pushed, we perform an action
       {
         case btnLEFT:
           {
              clear_lcd();
              MenuID = 10;
           break;
           }

         case btnUP:
           {
           CurrentLockTypeInt--;
           if (CurrentLockTypeInt < 0){
               CurrentLockTypeInt = LockTypeCount-1;
           }
           break;
           }
         case btnDOWN:
           {
             CurrentLockTypeInt++;
             if (CurrentLockTypeInt >= LockTypeCount)
                 CurrentLockTypeInt = 0;     
             break;
           }
         case btnSELECT:
           {
            if (CurrentLockType == LockTypes[0]){         
              clear_lcd();
              MenuID = 0;
            } else if (CurrentLockType == LockTypes[1] ) {
              clear_lcd();
              MenuID = 0;              
            }else if (CurrentLockType == LockTypes[2] ) {
              clear_lcd();
              MenuID = 0; 
              
            }else if (CurrentLockType == LockTypes[3] ) {
              clear_lcd();
              MenuID = 20; 
            }
            
            
           break;
           }
           case btnNONE:
           {
           buttonDown=false;
           lcd.print("NONE  ");
           break;
           }
       }
       return;
     }



   if (MenuID == 2) {
     

     switch (lcd_key)               // depending on which button was pushed, we perform an action
       {
         case btnRIGHT:
           {
           lcd.print("RIGHT ");
           break;
           }
         case btnLEFT:
           {
           lcd.print("LEFT   ");
           break;
           }
         case btnUP:
           {
           break;
           }
         case btnDOWN:
           {
             break;
           }
         case btnSELECT:
           {
            if (currentAction == "Start "){         
                continueDialing();
            }
           break;
           }
           case btnNONE:
           {
           buttonDown=false;
           lcd.print("NONE  ");
           break;
           }
       }


     
     
     
     
     
   }
     
     
   }  
}

/*********************************************************************************************
*
*   EEPROM
*
**********************************************************************************************/


//  int _currentkeytype = EEPROM.read(EEPROMKeyIndex);
void WriteLastCombo( int first, int second, int third, int forth) {
  
  EEPROM.write(EEPROMKeyIndex, first);
  EEPROM.write(EEPROMKeyIndex+1, second);
  EEPROM.write(EEPROMKeyIndex+2, third);
  EEPROM.write(EEPROMKeyIndex+3, forth);
  
}

void ReadLastCombo() {
  
  int _first = EEPROM.read(EEPROMKeyIndex);
  int _second = EEPROM.read(EEPROMKeyIndex+1);
  int _third = EEPROM.read(EEPROMKeyIndex+2);
  int _forth = EEPROM.read(EEPROMKeyIndex+3);
  currentdial1 = _first;
  currentdial2 = _second;
  currentdial3 = _third;
  currentdial4 = _forth;
}

void GetConfig() {
  Serial.print("Reading EEPROM: ");

   saveRunningInt = EEPROM.read(EEPROMKeyIndex+4);
  if (saveRunningInt > 0)
    saveRunningInt = true;
  Serial.print("Reading EEPROM [SaveRunning]: ");
  Serial.print(saveRunningInt);

  wasRunningInt = EEPROM.read(EEPROMKeyIndex+5);    
  if (wasRunningInt > 0 )
     wasRunning = true;

  Serial.print("Reading EEPROM [WasRunning]: ");
  Serial.print(wasRunningInt);
   
  CurrentLockTypeInt = EEPROM.read(EEPROMKeyIndex+6);

  Serial.print(CurrentLockType);
  Serial.print(":");
  
  
  CurrentLockType = LockTypes[CurrentLockTypeInt];
  Serial.print(CurrentLockType);
  IterationSize = EEPROM.read(EEPROMKeyIndex+7);
  currentdial4 = -1;

  accuracymidsteps=  EEPROM.read(EEPROMKeyIndex+8); 
  Serial.print("Reading EEPROM [AccuracyMidSteps]: ");
  Serial.print(accuracymidsteps);
  
  allowabledeviation =  EEPROM.read(EEPROMKeyIndex+9); 
  allowabledeviation = allowabledeviation / 100;
  Serial.print("Reading EEPROM [AllowableDeviation]: ");
  Serial.print(floatToString(allowabledeviation,2));

  wiggleposition=EEPROM.read(EEPROMKeyIndex+10); 
  Serial.print("Reading EEPROM [wiggleposition]: ");
  Serial.print(wiggleposition);


  retractposition = EEPROM.read(EEPROMKeyIndex+11); 
  Serial.print("Reading EEPROM [retractposition]: ");
  Serial.print(retractposition);
  
  
  Serial.println(CurrentLockType);
}

void PrintConfig(){
  Serial.print("SaveRunning:");
  Serial.print(saveRunningInt);
  Serial.print(" wasRunning:");
  Serial.print(wasRunningInt);
  Serial.print(" Locktype:");
  Serial.print(CurrentLockTypeInt);
  Serial.print(" IterationSiz:");
  Serial.print(IterationSize);
  Serial.print(" Deviation");
  Serial.print(floatToString(allowabledeviation,2));
  Serial.print(" WigglePosition:");
  Serial.print(wiggleposition);
  Serial.print("RetractPosition:");
  Serial.print(retractposition);
  Serial.println("");

}

void WriteConfig() {
  Serial.print("Writing: ");
  EEPROM.write(EEPROMKeyIndex+4, saveRunningInt);
  EEPROM.write(EEPROMKeyIndex+5, wasRunningInt);
  EEPROM.write(EEPROMKeyIndex+6, CurrentLockTypeInt);
  EEPROM.write(EEPROMKeyIndex+7, IterationSize);
  EEPROM.write(EEPROMKeyIndex+8, accuracymidsteps);
  EEPROM.write(EEPROMKeyIndex+9, allowabledeviation*100);
  EEPROM.write(EEPROMKeyIndex+9, wiggleposition);
  EEPROM.write(EEPROMKeyIndex+9, retractposition);

//  for (int x=0;x<KeyTypesCount;x++){
//    if ( CurrentKeyType == KeyTypes[x])
//    {
//      EEPROM.write(EEPROMKeyIndex, x);
//      Serial.print(x);
//    }
//  }
//  
  Serial.print(":");

//  for (int x=0; x<CurrentKeyCodeCount ;x++) {
//
//    EEPROM.write(EEPROMCodeIndex+x, CurrentKeyCode[x]);
//    Serial.print(CurrentKeyCode[x]);
//    Serial.print("-");
//  }
  Serial.println("");
}


 
/*********************************************************************************************
*
*   MOTOR
*
**********************************************************************************************/


void moveDialStep(int steps, boolean count){
  interrupt1counter=0;
  interrupt2counter=0;
  interrupt3counter=0;
  if (steps == 0) {
   movefailed=false;
  return; 
    
  }
  if (count) {
      
    int onestep=1;
    if (steps < 0){ 
     onestep = -1;
    }
    for (int x = 0; x < steps; x++)  {
      myXStepper.step(onestep);  
      int Optical1 = 0;
      Optical1 = digitalRead(switch1port);
      int Optical2 = 0;
      Optical2 = digitalRead(switch2port);
      int Optical3 = 0;
      Optical3 = digitalRead(switch3port);
      Serial.print("Optical: 1[");
      Serial.print(Optical1);
      Serial.print("] A[");
      Serial.print(Optical2);
      Serial.print("] B[");
      Serial.print(Optical3);
      Serial.println("]");
      delay(200);
    }
  } else {
    myXStepper.step(steps);
    
  }
  Serial.println("----------------------------------------------");
    Serial.println("------Interrupts: ");
    Serial.print("   Int 1: ");
    Serial.println(interrupt1counter);
    
    Serial.print("   Int 2: ");
    Serial.print(interrupt2counter);
    Serial.print("  -- Ratio: ");
    
    float Z=0;
    float X=interrupt2counter;
    if (steps < 0)
      steps = steps * -1;
    float Y=steps;
    Z=X/Y;
    //Serial.print(Z);
    printFloat(Z,5);
    
    
    Serial.println(" -----------");

    
    Serial.print("   Int 3: ");
    Serial.println(interrupt3counter);
    
    Serial.println("----------------------------------------------");
    
    if (Z < (accuracymidsteps - allowabledeviation) || Z > (accuracymidsteps + allowabledeviation)) {
      movefailed = true;      
      Serial.println("move failed:");
      Serial.print(Z);
      Serial.print(" < ");
      Serial.print(accuracymidsteps - allowabledeviation);
      Serial.print(" || ");
      Serial.print(Z);
      Serial.print(" > ");
      Serial.print(accuracymidsteps - allowabledeviation);
      
    }else {
     movefailed = false; 
    }


}

void doEncoder1(){    //every time a change happens on encoder pin 1 doEncoder will run.

interrupt1counter++;
} 
 
void doEncoder2(){    //every time a change happens on encoder pin A doEncoder will run.
interrupt2counter++;
//Serial.println("A");

}

void doEncoder3(){    //every time a change happens on encoder pin b doEncoder will run.
interrupt3counter++;
}
 
 
void OneRotation(int times, String dialdirection) {

 if (dialdirection == "left") {
   moveDial(fullturn * times); 
 } else {
  moveDial(fullturn * times * -1); 
 }  
  
  
}

void MoveDialTo(int mark, String dialdirection) {
 int _currentDialLocation = CurrentLocationMark;
 int _marks = calcMoveDistance(mark,  dialdirection);
 int _revolutions =  convertMarktoRevolutions(_marks);
 moveDial(_revolutions); 
 setCurrentLocation(mark);
}

int convertMarktoRevolutions( int mark) {
 return (mark*2); 
}

void setCurrentLocation(int mark){
  CurrentLocationMark = mark;
}

int calcMoveDistance( int mark, String dialdirection) {
 int _current = CurrentLocationMark;  
 int marks = 0;

 if (dialdirection == "right") {
   //Cockwise Dial
   Serial.print("Moving Dial: [right] from: ");
   Serial.print(_current);
   Serial.print(" to: ");
   Serial.println(mark);
   marks = CurrentLocationMark - mark;
   if ( mark > CurrentLocationMark) {
    marks =   marks + 100;
   }
   marks = marks * -1;
 } else {
  Serial.print("Moving Dial: [left] from: ");
   Serial.print(_current);
   Serial.print(" to: ");
   Serial.println(mark);

   //Counter Clockwise Dial   
   marks = mark - CurrentLocationMark;
   if ( mark < CurrentLocationMark) {
    marks =   marks + 100;
   }
   
 }
return (marks);   
}




/*********************************************************************************************
*
*   SAFE
*
**********************************************************************************************/

 void printdials() {
   
    String _x = "[";
    _x += currentdial1;
    _x += " - ";
    _x += currentdial2;
    _x += " - ";
    _x += currentdial3;
    _x += " - ";
    _x += currentdial4;
    _x += "]";
   String _y = "Going to Dial[";
   _y += IterationSize;
   _y += "]";
    print_lcd(_y,_x,5);
   
   
 }
boolean isValidCombo(){
 return true; 
}


void goDial(){

  boolean _running = true;
  wasRunning=true;
  wasRunningInt=1;
  WriteConfig();
  while (_running) {
    
    //printdials();
    boolean _success = tryCombo(currentdial1,currentdial2,currentdial3,currentdial4);
    WriteLastCombo(currentdial1,currentdial2,currentdial3,currentdial4);
    
    if (_success){

       _running = false;
       wasRunning=false;
       wasRunningInt=0;
       WriteConfig();
       MenuID=99;
       return;
    }
    
    if (lockdials == 4) {
      currentdial4 += IterationSize;
      if (currentdial4 > 100) {
         currentdial4 = 0;
         currentdial3 += IterationSize; 
      }
    } else {
       currentdial3 += IterationSize; 
      }
//    printdials();
    if (currentdial3 >= 100) {
     currentdial2 += IterationSize;
     currentdial3 = 0; 
    }
//      printdials();
    if (currentdial2 >= 100) {
     currentdial1 += IterationSize;
     currentdial2 = 0; 
    }
//      printdials();
    if (currentdial1 >= 100) {
     return; 
    }
  }
  
}

void startDialing() {
 currentdial1 = 0;
 currentdial2 = 0;
 currentdial3 = 0;
 currentdial4 = 0;
 WriteLastCombo(0,0,0,0); 
 goDial();
}
void continueDialing(){
 goDial(); 
}


boolean checkMoveFailed(){
 if (movefailed){
   Serial.println("==========================================================================================================");
   Serial.println("==========================================================================================================");
   Serial.println("======================= MOVE FAILED ======================================================================");
   Serial.println("==========================================================================================================");
   Serial.println("==========================================================================================================");
 }
 return movefailed;
}


boolean tryCombo(int one, int two, int three, int four){
//   lcd.print("LEFT   ");
  String lcdstr = "D:";
  //Serial.print("debug=");
  //debug = digitalRead(debugswitch);
  //Serial.println(debug);
  
  Serial.print("Trying Combo: ");
  Serial.print(one);
  Serial.print("-");
  Serial.print(two);
  Serial.print("-");
  Serial.println(three);
  Serial.print("-");
  Serial.println(four);

  String _lcd = lcdstr;
  _lcd += "[";
  _lcd += one;
  _lcd += "-";
  _lcd += two;
  _lcd += "-";
  _lcd += three;
  _lcd += "-";
  _lcd += four;
  _lcd += "]";
  
  print_msg(_lcd);  
  print_status("Sleeping");

  delay(1000);
  setCurrentLocation(0);
  
  
  // ------------- if 4 dials --------------------------
  
  int fullrotations = lockdials;
  
  if (four >= 0) {
    
  print_status("Rotate: 4 right");
  OneRotation(fullrotations--,"right");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  print_status2("Rotate to:",one);
  MoveDialTo(one,"left");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  // shift them all down one
  one = two;
  two = three;
  three = four;  
    
  } else {
    
    
    
   
  }
  // -------------- if 3 dials --------------------------
  print_status("Rotate: 3 left");
  OneRotation(fullrotations,"left");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  print_status2("Rotate to:",one);
  MoveDialTo(one,"left");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  
  print_status("Rotate: 2 right");
  OneRotation(fullrotations--,"right");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  print_status2("Rotate to:",two);
 
  MoveDialTo(two,"right");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  print_status("Rotate: 1 left");
  
  OneRotation(fullrotations--,"left");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  print_status2("Rotate to:",three);
 
  MoveDialTo(three,"left");
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(500);
  
  
  //----------------------------------- finished dials---------------------
  
  
  print_status("Move to: wiggle");
  Serial.println("Moving to Wiggle Position");
  MoveDialTo(wiggleposition,"right");
  delay(1000);
  
  print_status("Wiggling");
  wiggle();
  
  //delay(200);
  //change after testing
  delay(2000);
  
  print_status("Retracting");
    Serial.println("Retracting");
  MoveDialTo(retractposition,"right");  
  delay(200);
  
  // TEST FOR OPEN HERE SET TO TRUE IF OPEN
  // return true;
  if (checkMoveFailed())
  {
  print_msg(" COMPLETE ");
  print_status(" LOCK OPEN ");
  Serial.println("");
  Serial.println("");
  Serial.println("                        ---------------------------   ");
  Serial.println("                        -        LOCK OPEN        -   ");
  Serial.println("                        ---------------------------   ");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  
   return true;
   
  }
  Serial.println("Done Retracting");
  print_status("Resetting to 0");
  
  MoveDialTo(0,"left");  
  if (checkMoveFailed()){
    print_msg(" COMPLETE ");
    print_status(" ERROR ");
    return true;
  }
  delay(200);

  //clear_lcd();  
  return false;
}

void moveDial(int revolutions){
    Serial.print("Moving Dial: ");
    Serial.println(revolutions);
 //   moveDialStep(revolutions, false);
    moveDialStep(revolutions, debug);
}
void wiggle(){
    Serial.println("wiggling");
  myXStepper.step(wigglesize);
  delay(wigglesleep);
  myXStepper.step(wigglesize*-2);
  delay(wigglesleep);
  myXStepper.step(wigglesize*2);
  delay(wigglesleep);
  myXStepper.step(wigglesize*-1);
  delay(wigglesleep);
    Serial.println("wiggling Done");
}

/*********************************************************************************************
*
*   GENERAL
*
**********************************************************************************************/




void setup()
{

  Serial.begin(9600);
  pinMode(debugswitch, INPUT);
    EEPROM.write(EEPROMKeyIndex+8, 4);
  EEPROM.write(EEPROMKeyIndex+9, 75);


 //LCD Setup
 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.print("Push the buttons"); // print a simple message
 //Motor Setup
    
  pinMode(Xwire1, OUTPUT); 
  pinMode(Xwire2, OUTPUT);
  pinMode(Xwire3, OUTPUT);
  pinMode(Xwire4, OUTPUT);
  
  
  pinMode(L298L1,OUTPUT);
  pinMode(L298L2,OUTPUT);
  pinMode(L298L3,OUTPUT);
  pinMode(L298L4,OUTPUT);
  // set the speed at 60 rpm:
  myXStepper.setSpeed(motorSpeed);

 // Optical Setup
 
  pinMode(switch1port, INPUT);
  pinMode(switch2port, INPUT);
  pinMode(switch3port, INPUT);

  digitalWrite(switch1port,HIGH);
  digitalWrite(switch2port,HIGH);
  digitalWrite(switch3port,HIGH);

  attachInterrupt(interruptport1, doEncoder1, CHANGE); 
  attachInterrupt(interruptport2, doEncoder2, CHANGE); 
  attachInterrupt(interruptport3, doEncoder3, CHANGE); 

//  WriteLast  delay(5000);Combo(60,20,40,99);
  ReadLastCombo();
//  wasRunningInt=1;
//  wasRunning=true;
  
 // WriteConfig();
  GetConfig();  
//  PrintConfig();  
//  printdials();
  
  if (wasRunning) {
    currentActionInt=1;
    
  }

}

void loop()
{
 print_menu();
 get_keys();
 decide_action();
  
}



// printFloat prints out the float 'value' rounded to 'places' places after the decimal point
String floatToString(float value, int places){
 
  // this is used to cast digits 
  int digit;
  float tens = 0.1;
  int tenscount = 0;
  int i;
  float tempfloat = value;

String str="";
    // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
  // if this rounding step isn't here, the value  54.321 prints as 54.3209

  // calculate rounding term d:   0.5/pow(10,places)  
  float d = 0.5;
  if (value < 0)
    d *= -1.0;
  // divide by ten for each decimal place
  for (i = 0; i < places; i++)
    d/= 10.0;    
  // this small addition, combined with truncation will round our values properly 
  tempfloat +=  d;

  // first get value tens to be the large power of ten less than value
  // tenscount isn't necessary but it would be useful if you wanted to know after this how many chars the number will take

  if (value < 0)
    tempfloat *= -1.0;
  while ((tens * 10.0) <= tempfloat) {
    tens *= 10.0;
    tenscount += 1;
  }


  // write out the negative if needed
  if (value < 0)
    str += "-";
  // Serial.print('-');

  if (tenscount == 0)
    str+="0";
   // Serial.print(0, DEC);

  for (i=0; i< tenscount; i++) {
    digit = (int) (tempfloat/tens);
    //Serial.print(digit, DEC);
    str+=digit;
    tempfloat = tempfloat - ((float)digit * tens);
    tens /= 10.0;
  }

  // if no places after decimal, stop now and return
  if (places <= 0)
    return str;

  // otherwise, write the point and continue on
  //Serial.print('.');  
  str+=".";
  // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
  for (i = 0; i < places; i++) {
    tempfloat *= 10.0; 
    digit = (int) tempfloat;
    str += digit;
    //Serial.print(digit,DEC);  
    // once written, subtract off that digit
    tempfloat = tempfloat - (float) digit; 
  }
  
  return str;
}

void printFloat(float value, int places) {
  
  Serial.print(floatToString(value,places));
  
}
