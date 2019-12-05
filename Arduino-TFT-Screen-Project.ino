//#include <DHT.h>

/* Automated House Security System
 * This is the Master Side that is the TFT LCD is programmed on
 * and used to control Sensors, and serially control Leds / Motors on the slave
 * side. Using SainSmart 3.2" TFT LCD Display SSD1289:240 RGB x 320 TFT Driver
 * Written By: Dominik Buczek, Carlos Guillen, Chris Diaz
 * Final Capstone Code, Fanshawe College
 * December 5th, 2019
 * All Rights Reserved
 */

#include <UTFT.h>  //Display Library containing functions used throughout the project
#include <URTouch.h> //Touch Library that complements the UTFT library and provide touch screen functionality
#include <Keypad.h> //Library 
//#include <DHT.h>

UTFT myGLCD(SSD1289,38,39,40,41); //This is a class constructor when using 8bit or 16bit display models. UTFT(Model,RS,WR,CS,RST). Model being used is the SSD1289.
//SSD1289 all in one TFT LCD Controller Driver that integrated the RAM, Power circuits, gate driver & source driver into a single chip. p38 Register select,
//p39 for writing, p40 for chip select and p41 for reset. This parameters must be configured based on TFT model and specifications. myGLCD instance of this class
                        
URTouch myTouch(6, 5, 4, 3, 2); //This is a class of the interface. Start an instance of the UTouch class. UTouch(TCLK, TCS, TDIN, TDOUT,IRQ). p6 touch clock,
//p5 touch chip select, p4 touch data input, p3 touch for data, p2 touch irq. Parameters must be configured based on TFT LCD model being used

/*Defining 3 different font sizes used throughout the display code. The extern modifier tells the compiler that the variable isn't defined in this file, but from the UTFT file.
uint8_t is used to precisely indicate what the type is (unsigned single-byte in this case). When we pass "BigFont" into the "setFont" function, the compiler sets aside a byte 
to hold the address of bigfont and marks that byte to tell the linker to fill in the value
whe  it knows where BigFont will land in memory. When our sketch runs, address of BigFont gets passed to setFont
which uses that adrdress as a pointer. This pointer can reach the BigFont array*/ 
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

unsigned char currentPage; //single character values that are used to indicate 'page numbers'. When we assign new character into this variable, we want to call new functions
//to simulate submenus. We used numbers from 0-9, then letters a-z (a=10, b=11, c=12 etc.) to change between submenus that all have different displays and touch features
/* GLOSSARY OF OUR PAGE NUMBERS
0 - Indicate we are the home screen
1 - Sensor Menu
2 - Light Control Menu
3 - Motor Control Menu
4 - Humidity Sensor Submenu 
5 - Distance Sensor Submenu  
6 - PIR Sensor Submenu 
7 - Smoke Sensor Submenu
8 - Water Sensor Submenu  
9 - Indoor lights Submenu 
a - Garage Motor Submenu
b - HVAC Motor Submenu
c - Keypad Menu
d - Outdoor lights Submenu
 */

/* DEFINIG 2 VARIABLES USED TO STORE THE DATA OF WHERE THE TOUCH HAS TAKEN PLACE ON THE TFT LCD
 * It is understood that the Capstone Standards do not want single character variable names but this case the single 'x' and single 'y' is used to store exactly where
 * on the X-axis of the screen was pressed and where on the Y-axis of the screen was pressed. Was much easier to write our code like this because we are constantly 
 * using these 2 varaibles to compare where the touch took place and what action to take according to the coordinates
 */
int x;
int y;

//================================KEYPAD VARIABLES CODE===============================//
//Used a variety of different global varaibles so breaking them up based  on which sensor/function they are used in

#define passwordLength 5 //there is a null character added to end of string so must be 1 greater than length of password. User can choose length of password
const int ROWS = 4; //keypad is 4row by 4 columns
const int COLS = 4; //keypad is 4row by 4 columns
char input[passwordLength];
char Master[passwordLength] = "1234";
int inputCount =0;
int master_count = 0;
bool passwordGood;
char customKey;
char hexaKeys[ROWS][COLS] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'O', 'F', 'E', 'D'}
};

char rowPins[ROWS] = {58,59,60,61}; 
char colPins[COLS] = {54,55,56,57}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); //returns a zero if keypad not press
//==================================EO KEYPAD =========================================//

//Ultrasonic Sensor
const int trigPin = 63;
const int echoPin = 62;
long duration;
int distance;

//Pir Sensor
int pirInputPin = 64;
int pirState = HIGH;             // we start, assuming no motion detected
int pirval = 0;                    // variable for reading the pin status

//Water sensor
const int waterRead = A0;
int waterlevel;

//Garage Motor variables
int motorOn = 0;
int motorOff = 0;

//HVAC motor vairables
int hvacOn = 0;
int hvacOff =0;
int hvacStop =0;
int motorCount = 0;

//SERIAL COMM
int Pin69= 69;

//TEMPERATURE SENSOR VARIABLE
float tempLM35;

//HUMIDITY SENSOR
//dht DHT;
#define DHT11_PIN 66
int humcount = 0; 

//led count
int p1,p2,p3,p4,p5,p6,p7,p8,p9;

void setup() 
{
  
  Serial.begin(9600);
  Serial1.begin(4800);
  
  pinMode(pirInputPin, INPUT);
// Initial setup
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  myGLCD.print("Please Enter Security Code", LEFT, 50);

  
  // Defining Pin Modes
//  pinMode(VCC, OUTPUT); // VCC
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  //garageMotor
//  pinMode(in1, OUTPUT);
 // pinMode(in2, OUTPUT);

  //SERIAL COMM
  pinMode(Pin69, OUTPUT); // Set the pin as an output and initialize it
  digitalWrite(Pin69, HIGH); // Set the pin as a high to enable the TX terminal in the MAX285

//  pinMode(button, INPUT);
//  digitalWrite(VCC, HIGH); // +5V - Pin 13 as VCC
  //drawHomeScreen();  // Draws the Home Screen
 // keyPad();
  currentPage = 'c'; // Indicates that we are at Home Screen
} //END OF SETUP FUNCTION


void keyPad(void)
{
    // Title
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("Enter Password", CENTER, 10); // Prints the string on the screen

  customKey = customKeypad.getKey();
  if (customKey != NO_KEY)
    {
    input[inputCount] = customKey; 
    inputCount++; 
    }
  Serial.print(customKey);
  if(inputCount == passwordLength-1)
  {
    if(!strcmp(input, Master))
      {
      myGLCD.print("Correct", LEFT, 80);
      myGLCD.clrScr();
      Serial.print(customKey);
      drawHomeScreen();
      currentPage = '0';
      delay(1000);
      }
    else
      {
      myGLCD.print("Incorrect", CENTER, 100);
      myGLCD.clrScr();
      delay(1000);
      }
    while(inputCount !=0)
    {
    input[inputCount--] = 0; 
    } 
  }
} //EO KEYPAD 

void runMotor(void)
{
//   digitalWrite(in1, HIGH);
  // digitalWrite(in2, LOW);
   delay(20);
}

void stopMotor(void)
{
//   digitalWrite(in1, LOW);
  // digitalWrite(in2, LOW);
   delay(20);
}
void page0(void)
{
  if (myTouch.dataAvailable()) 
    {
      myTouch.read();
      x=myTouch.getX(); // X coordinate where the screen has been pressed
      y=myTouch.getY(); // Y coordinates where the screen has been pressed
      
      // IF WE PRESS THE SENSOR MENU
      if ((x>=35) && (x<=285) && (y>=90) && (y<=130))
        {
        drawFrame(35, 90, 285, 130); //Highlighs the buttons when it's pressed
        currentPage = '1'; // CHANGE THE PAGE 
        myGLCD.clrScr(); // Clears the screen
        drawSensor(); // It is called only once, DRAWS THE SENSOR SUBMENU 
        }

        //IF WE PRESS THE LIGHT CONTROL MENU ON THE HOME SCREEN
      if ((x>=35) && (x<=285) && (y>=140) && (y<=180)) 
        {
        drawFrame(35, 140, 285, 180);
        currentPage = '2';
        myGLCD.clrScr();
        led();
        }  
        
      //IF WE PRESS THE MOTOR CONTROL MENU ON THE HOME SCREEN
      if ((x>=35) && (x<=285) && (y>=190) && (y<=230))
        {
        drawFrame(35, 190, 285, 230); // Custom Function -Highlighs the buttons when it's pressed
        currentPage = '3'; // Indicates that we are ON motor page
        myGLCD.clrScr(); // Clears the screen
        drawMotor(); // It is called only once, because in the next iteration of the loop, this above if statement will be false so this funtion won't be called. This function will draw the graphics of the first example.
        }
     }
}// END OF HOMEPAGE MENU 


void page1(void)
{
 if (myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
       if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '0'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawHomeScreen(); // Draws the home screen
        }
       if ((x>=25) && (x<=295) &&(y>=90) && (y<=130))
          {
          currentPage = 'f';
          drawFrame(25,90,295,130);
          myGLCD.clrScr();
          sensorSub1();
         // garageMotor();
          }
       if ((x>=25) && (x<=295) &&(y>=150) && (y<=190))
          {
          currentPage = 'g';
          drawFrame(25,150,295,190);
          myGLCD.clrScr();
          sensorSub2();
         // hvacMotor();
          }
       } 
}


void page2(void)
{
if (myTouch.dataAvailable()) 
        {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();

        //IF WE PRESS THE  BACK BUTTON
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
          {
          drawFrame(10, 10, 60, 36);
          currentPage = '0'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawHomeScreen(); // Draws the home screen
          }

       //IF WE PRESS INDOOR LIGHTS
       if ((x>=20) && (x<=300) &&(y>=60) && (y<=120)) //coords for the indoor lights box
       {
         drawFrame(20, 60, 300, 120);
         myGLCD.clrScr();
         currentPage = '9';
       //  Serial1.write('1');
         //indoorLights();
       } 

      //IF WE PRESS THE OUT DOOR LIGHTING
      if ((x>=20) && (x<=300) &&(y>=140) && (y<=200)) //coords for the out door lights box
        {
         drawFrame(20, 140, 300, 200);
         myGLCD.clrScr();
         currentPage = 'd';
       //  Serial1.write('+'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        // delay(2000); // Wait 2 seconds before sending the next symbol
  //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED
  //delay(2000); // Wait 2 seconds before restarting the loop 
         outdoorLights();
         } 
        } //END OF TOUCH AVAILABLE 
} //END OF PAGE 2 LIGHTING
  

void page3(void)
{
 if (myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
       if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '0'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawHomeScreen(); // Draws the home screen
        }
       if ((x>=25) && (x<=295) &&(y>=90) && (y<=130))
          {
          currentPage = 'a';
          drawFrame(25,90,295,130);
          myGLCD.clrScr();
          garageMotor();
          }
       if ((x>=25) && (x<=295) &&(y>=150) && (y<=190))
          {
          currentPage = 'b';
          drawFrame(25,150,295,190);
          myGLCD.clrScr();
          hvacMotor();
          }
       } 
 } //END OF PAGE 3 MOTOR CONTROL

void page4(void)
{
    if (myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
        drawBackButton();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '1'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor();
        }
      } 
}

void page5(void)
{
 if(myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
       // drawBackButton();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '1'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor();
        }
      }  
}

void page6(void)
{
   if(myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
     //   drawBackButton();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '1'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor();
        }
      }  
}

void page7(void)
{
  if(myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
      //  drawBackButton();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '2'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor();
         // smokeSensor();
        }
      } 
}

void page8(void)
{
  if(myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
     //   drawBackButton();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '2'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor();   
        }
      } 
}
 
void page9(void) //PAGE 9 IS THE INDOOR LIGHTING MENU 
{
    if(myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
     //   drawBackButton();
        indoorLights();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '2'; // Indicates we are at home screen
          myGLCD.clrScr();
          page2();
        }

       if ((x>=0) && (x<=125) &&(y>=70) && (y<=95)) 
        {
        drawFrame(0,70,125,95);
        Serial1.write('3'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

       if ((x>=0) && (x<=125) &&(y>=100) && (y<=125)) 
        {
        drawFrame(0,100,125,125);
        Serial1.write('4'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

      if ((x>=0) && (x<=125) &&(y>=130) && (y<=155)) 
        {
        drawFrame(0,130,125,155);
        Serial1.write('5'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

     if ((x>=0) && (x<=125) &&(y>=160) && (y<=185)) 
        {
        drawFrame(0,160,125,185);
        Serial1.write('6'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

     if ((x>=0) && (x<=125) &&(y>=190) && (y<=215)) 
        {
        drawFrame(0,190,125,215);
       Serial1.write('7'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

     if ((x>=180) && (x<=315) &&(y>=70) && (y<=95)) 
        {
        drawFrame(180,70,315,95);
         Serial1.write('8'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }
        
    if ((x>=180) && (x<=315) &&(y>=100) && (y<=125)) 
        {
        drawFrame(180,100,315,125);
        Serial1.write('9'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

    if ((x>=180) && (x<=315) &&(y>=130) && (y<=155)) 
        {
        drawFrame(180,130,315,155);
      Serial1.write('a'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

    if ((x>=180) && (x<=315) &&(y>=160) && (y<=185)) 
        {
        drawFrame(180,160,315,185);
       Serial1.write('b'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

   if ((x>=180) && (x<=315) &&(y>=190) && (y<=215)) 
        {
        drawFrame(180,190,315,215);
        Serial1.write('c'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

   if ((x>=140) && (x<=170) &&(y>=65) && (y<=95)) 
        {
        drawFrame(180,190,315,215);
        Serial1.write('d'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

    if ((x>=140) && (x<=170) &&(y>=103) && (y<=133)) 
        {
        drawFrame(180,190,315,215);
        Serial1.write('e'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

    if ((x>=140) && (x<=170) &&(y>=141) && (y<=171)) 
        {
        drawFrame(180,190,315,215);
        Serial1.write('f'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        }

     if ((x>=140) && (x<=170) &&(y>=65) && (y<=95)) 
       {
       drawFrame(180,190,315,215);
       Serial1.write('g'); // Send symbol to the slave to turn on the first LED and turn off the second LED
       }
      } 
} //EO PAGE 9

void pagea(void)
{
  if(myTouch.dataAvailable()) //wait for a touch
      {
        myTouch.read(); //read data from touch screen, this function is normally called when dataAvailable is TRUE
        x=myTouch.getX(); //assign where on the x-axis the touch was taken place and assign it into variable X
        y=myTouch.getY(); //assign where on the y-axis the touch was taken place and assign it into variable Y
        drawBackButton(); //call function that draws a back button on the top left of the TFT screen
       // garageMotor();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36))  //if the data fell between these limits, do the following work
        {
          drawFrame(10, 10, 60, 36); //pass in the coordinates to a function called 'drawFrame' that draws a redbox around the button to simulate that it was pressed
          currentPage = '3'; // set the current back to 3 (motor menu)
          myGLCD.clrScr(); //clear the screen
          drawMotor(); //draw the motor menu
          page3(); //call the page3 function to put functionality in the motor screen
        }

       if ((x>=190) && (x<=290) &&(y>=96) && (y<=196)) //if we hit between these coordinates
        {
          motorOn = 1; //raise motoron variable
          motorOff =0; //lower motoroff variable
          myGLCD.clrScr(); //clear the screen (so we can put a new print in the bottom of the screen in a different function
          garageMotor(); //call the garagemotor again
       // runMotor();
        }

      if ((x>=20) && (x<=120) &&(y>=96) && (y<=196)) 
        {
          motorOff = 1;
          motorOn = 0;
          myGLCD.clrScr();
          garageMotor();
          //runMotor();
        }
       motorOn = 0;
       motorOff = 0;
     }       
}//EO PAGEA

void pageb(void)
{
  if(myTouch.dataAvailable()) 
      {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();
        drawBackButton();
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
        {
          drawFrame(10, 10, 60, 36);
          currentPage = '3'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawMotor();
          page3();
        }

       if ((x>=190) && (x<=290) &&(y>=96) && (y<=196)) 
        {
          hvacOn = 1;
          hvacOff =0;
          myGLCD.clrScr();
          hvacMotor();
        }

      if ((x>=20) && (x<=120) &&(y>=96) && (y<=196)) 
        {
          motorCount++;
          hvacOff = 1;
          hvacOn = 0;
          myGLCD.clrScr();
          hvacMotor();
        }

    if ((x>=135) && (x<=185) &&(y>=96) && (y<=196)) 
        {
          hvacStop = 1;
          myGLCD.clrScr();
          hvacMotor();
        }
     }       
}//EO PAGEB

void pageC(void)
{
  keyPad();
}

void pageD(void) //OUTDOOR MENU
{
 if (myTouch.dataAvailable()) 
        {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();

        //IF WE PRESS THE  BACK BUTTON
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
          {
          drawFrame(10, 10, 60, 36);
          currentPage = '2'; // Indicates we are at home screen
          myGLCD.clrScr();
          led(); // Draws the home screen
          }

       if ((x>=35) && (x<=285) &&(y>=70) && (y<=90)) 
        {
        drawFrame(35,70,285,90);
        Serial1.write('m'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED */
        }

       if ((x>=35) && (x<=285) &&(y>=95) && (y<=115)) 
        {
        drawFrame(35,95,285,115);
        Serial1.write('n'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED */
        }

      if ((x>=35) && (x<=285) &&(y>=120) && (y<=140)) 
        {
        drawFrame(35,120,285,140);
        Serial1.write('o'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED */
        }

     if ((x>=35) && (x<=285) &&(y>=145) && (y<=165)) 
        {
        drawFrame(35,145,285,165);
        Serial1.write('p'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED */
        }

     if ((x>=35) && (x<=285) &&(y>=170) && (y<=190)) 
        {
        drawFrame(35,170,285,190);
        Serial1.write('q'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED */
        }

     if ((x>=35) && (x<=285) &&(y>=195) && (y<=215)) 
        {
        drawFrame(35,195,285,215);
        Serial1.write('r'); // Send symbol to the slave to turn on the first LED and turn off the second LED
        //Serial1.write('-'); // Sends a minus symbol to the slave to turn the second LED off and turn the first LED */
        }

  }
}

void pageE(void)
{
   if (myTouch.dataAvailable()) 
     {
        myTouch.read();
        x=myTouch.getX();
        y=myTouch.getY();

        //IF WE PRESS THE  BACK BUTTON
        if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
          {
          drawFrame(10, 10, 60, 36);
          currentPage = '1'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor(); // Draws the home screen
          }
    }
}

void pageF(void)
{
  if (myTouch.dataAvailable()) 
    {
      myTouch.read();
      x=myTouch.getX(); // X coordinate where the screen has been pressed
      y=myTouch.getY(); // Y coordinates where the screen has been pressed
     // drawBackButton();
      // IF WE PRESS THE SENSOR MENU

     if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
          {
          drawFrame(10, 10, 60, 36);
          currentPage = '1'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor(); // Draws the home screen
          }
      if ((x>=35) && (x<=285) && (y>=90) && (y<=130))
        {
        drawFrame(35, 90, 285, 130); // Custom Function -Highlighs the buttons when it's pressed
        currentPage = '5'; // CHANGE THE PAGE 
        myGLCD.clrScr(); // Clears the screen
        DistanceSensor(); // It is called only once, DRAWS THE SENSOR SUBMENU 
        }

        //IF WE PRESS THE LIGHT CONTROL MENU ON THE HOME SCREEN
      if ((x>=35) && (x<=285) && (y>=140) && (y<=180)) 
        {
        drawFrame(35, 140, 285, 180);
        currentPage = 'e';
        myGLCD.clrScr();
        tempSensor();
        }  
        
      //IF WE PRESS THE MOTOR CONTROL MENU ON THE HOME SCREEN
      if ((x>=35) && (x<=285) && (y>=190) && (y<=230))
        {
        drawFrame(35, 190, 285, 230); // Custom Function -Highlighs the buttons when it's pressed
        currentPage = '8'; // Indicates that we are ON motor page
        myGLCD.clrScr(); // Clears the screen
        waterSensor();// It is called only once, because in the next iteration of the loop, this above if statement will be false so this funtion won't be called. This function will draw the graphics of the first example.
        }
     }
}

void pageG(void)
{
   if (myTouch.dataAvailable()) 
    {
      myTouch.read();
      x=myTouch.getX(); // X coordinate where the screen has been pressed
      y=myTouch.getY(); // Y coordinates where the screen has been pressed
     // drawBackButton();

     if ((x>=10) && (x<=60) &&(y>=10) && (y<=36)) 
          {
          drawFrame(10, 10, 60, 36);
          currentPage = '1'; // Indicates we are at home screen
          myGLCD.clrScr();
          drawSensor(); // Draws the home screen
          }
      
      // IF WE PRESS THE SENSOR MENU
      if ((x>=35) && (x<=285) && (y>=90) && (y<=130))
        {
        drawFrame(35, 90, 285, 130); // Custom Function -Highlighs the buttons when it's pressed
        currentPage = '5'; // CHANGE THE PAGE 
        myGLCD.clrScr(); // Clears the screen
        DistanceSensor(); // It is called only once, DRAWS THE SENSOR SUBMENU 
        }

        //IF WE PRESS THE LIGHT CONTROL MENU ON THE HOME SCREEN
      if ((x>=35) && (x<=285) && (y>=140) && (y<=180)) 
        {
        drawFrame(35, 140, 285, 180);
        currentPage = '5';
        myGLCD.clrScr();
        DistanceSensor();
        }  
        
      //IF WE PRESS THE MOTOR CONTROL MENU ON THE HOME SCREEN
      if ((x>=35) && (x<=285) && (y>=190) && (y<=230))
        {
        drawFrame(35, 190, 285, 230); // Custom Function -Highlighs the buttons when it's pressed
        currentPage = '5'; // Indicates that we are ON motor page
        myGLCD.clrScr(); // Clears the screen
        DistanceSensor();// It is called only once, because in the next iteration of the loop, this above if statement will be false so this funtion won't be called. This function will draw the graphics of the first example.
        }
     }
}

void sensorSub1(void)
{
  drawBackButton();
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (35, 90, 285, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (35, 90, 285, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("DISTANCE SENSOR", CENTER, 102); // Prints the string
  
  // Button - LIGHT CONTROL
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 140, 285, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 140, 285, 180);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("PIR SENSOR", CENTER, 152);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 190, 285, 230);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 190, 285, 230);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("TEMP SENSOR", CENTER, 202);
}

void sensorSub2(void)
{
  drawBackButton();
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (35, 90, 285, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (35, 90, 285, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("HUMIDITY SENSOR", CENTER, 102); // Prints the string
  
  // Button - LIGHT CONTROL
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 140, 285, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 140, 285, 180);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("WATER SENSOR", CENTER, 152);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 190, 285, 230);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 190, 285, 230);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("FLAME SENSOR", CENTER, 202);
}
void loop()
{ 
  // HOME SCREEN PAGE = 0
  if (currentPage == '0') 
    {
    page0();
    }

//==========================================BEGINNING OF SENSOR MENU -------================================================================================================-----------------------------------------------------------------------------------------------------
  if (currentPage == '1') 
    {    
    page1();
    }
     

  if(currentPage == '2')
    {
    page2();
    }
    
 if(currentPage=='3')
     {
     page3();
     }

  if(currentPage == '4')
    {
    page4();
    }

  if(currentPage == '5')
    {
    getDistance();
    page5();
    }

  if(currentPage == '6')
    {
      getPir();
   // pirSensor();
    page6();
    }

  if(currentPage == '7')
    {
    page7();
    }

  if(currentPage == '8')
    {
    //getWater();
    page8();
    }

  if(currentPage == '9')
    {
     page9();
    }

  if(currentPage == 'a')
    {
     pagea();
    }

  if(currentPage == 'b')
    {
     pageb();
    }

  if(currentPage == 'c')
    {
     pageC();
    }

   if(currentPage == 'd')
   {
    pageD();
   }

   if(currentPage == 'e')
   {
    getTemp();
    pageE();
   }

   if(currentPage == 'f')
   {
    pageF();
   }

   if(currentPage == 'g')
   {
    pageG();
   }
} //EO FUNCTION

// ====== Custom Funtions ======
// drawHomeScreen - Custom Function
void drawHomeScreen() 
{
  // Title
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("House System", CENTER, 10); // Prints the string on the screen
  myGLCD.setColor(255, 0, 0); // Sets color to red
  myGLCD.drawLine(0,32,319,32); // Draws the red line
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(SmallFont); // Sets the font to small
  myGLCD.print("by Dominik, Carlos, Chris", CENTER, 41); // Prints the string
  myGLCD.setFont(BigFont);
  myGLCD.print("Select Menu Control", CENTER, 64);
  
  // Button - SENSOR MENU
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (35, 90, 285, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (35, 90, 285, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("SENSOR MENU", CENTER, 102); // Prints the string
  
  // Button - LIGHT CONTROL
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 140, 285, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 140, 285, 180);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("LIGHT CONTROL", CENTER, 152);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 190, 285, 230);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 190, 285, 230);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("MOTOR CONTROL", CENTER, 202);
}

// Highlights the button when pressed
void drawFrame(int x1, int y1, int x2, int y2) 
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (x1, y1, x2, y2);
}
//====================================================


void drawBackButton(void)
{
  myGLCD.setColor(100, 155, 203);
  myGLCD.fillRoundRect (10, 10, 40, 36);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 10, 40, 36);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(100, 155, 203);
  myGLCD.print("<-", 9, 18);
  myGLCD.setBackColor(0, 0, 0);
}
void drawSensor() 
{
  drawBackButton();
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("Select Sensor Menu", CENTER, 50); // Prints the string on the screen
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (25, 90, 295, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (25, 90, 295, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("Sensor Sub 1", CENTER, 102); // Prints the string

  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (25, 150, 295, 190); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (25, 150, 295, 190); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("Sensor Sub 2", CENTER, 162); // Prints the string
  
} 

void drawMotor(void)
{
  drawBackButton();

  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("Select Motor Control", CENTER, 50); // Prints the string on the screen
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (25, 90, 295, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (25, 90, 295, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("GARAGE MOTOR", CENTER, 102); // Prints the string

  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (25, 150, 295, 190); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (25, 150, 295, 190); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("HVAC MOTOR", CENTER, 162); // Prints the string
}

void led(void)
{
  drawBackButton();
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(225, 120, 120); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("LIGHTING", CENTER, 10); // Prints the string on the screen
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (20, 60, 300, 120);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (20, 60, 300, 120);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("INDOOR LIGHTING", CENTER, 90);
  myGLCD.setColor(100, 155, 203);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (20, 140, 300, 200);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (20, 140, 300, 200);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("OUTDOOR LIGHTING", CENTER, 170);
  myGLCD.setColor(100, 155, 203);
  
}
void humiditySensor(void)
{
  drawBackButton();
  myGLCD.setFont(BigFont);
  myGLCD.print("Temperature Sensor", CENTER, 50);
  myGLCD.print("LM35", CENTER, 76);
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawLine(0,100,319,100);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("Temperature in The", 5, 120);
  myGLCD.print("House is:", 20, 150);
}


void getDistance(void)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  int  distanceCm= duration*0.034/2;
  if (distanceCm <=300)
    {
    myGLCD.setFont(SevenSegNumFont);
    myGLCD.setColor(0, 255, 0);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.printNumI(distanceCm,110, 145, 3,'0');
    myGLCD.setFont(BigFont);
    myGLCD.print("cm  ", 235, 178);
      }
}

void DistanceSensor() 
{
  myGLCD.setColor(100, 155, 203);
  myGLCD.fillRoundRect (10, 10, 60, 36);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 10, 60, 36);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(100, 155, 203);
  myGLCD.print("<-", 18, 15);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Back to Sensor Menu", 70, 18);
  myGLCD.setFont(BigFont);
  myGLCD.print("Ultrasonic Sensor", CENTER, 50);
  myGLCD.print("HC-SR04", CENTER, 76);
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawLine(0,100,319,100);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("Distance from Front", 5, 120);
  myGLCD.print("Door:", 20, 150);
}


void tempSensor(void)
{
  drawBackButton();
  myGLCD.setFont(BigFont);
  myGLCD.print("Temperature Sensor", CENTER, 50);
  myGLCD.print("LM35", CENTER, 76);
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawLine(0,100,319,100);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("Temperature in The", 5, 120);
  myGLCD.print("House is:", 20, 150);
}

void getTemp(void)
{
  float tempLM35 = analogRead(A11);
  tempLM35 = ((tempLM35/1024)*5000)/10;
 // Serial.print("Temperature:"); Serial.print(tempLM35);
  //Serial.print("*C"); Serial.println();
  float sum = 0;
  int count = 0;
  float average = (sum / 10);
  //float average = 0;
   sum = sum + tempLM35;
   count++;
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
  
  if(tempLM35 >= 20 && tempLM35 <27)
    {
  Serial.print("Temperature:"); Serial.print(tempLM35);
  myGLCD.printNumI(tempLM35,160, 145, 3,'0');
  myGLCD.setFont(BigFont);
  myGLCD.print("*C", 260, 145);
  delay(1000);
    }
 
  if(count == 10)
   {
   myGLCD.setFont(SevenSegNumFont);
   myGLCD.setColor(0, 255, 0);
   myGLCD.setBackColor(0, 0, 0);
   myGLCD.printNumI(average,120, 145, 3,'0');
   myGLCD.setFont(BigFont);
   myGLCD.print("*C ", 175, 178);
   delay(1000);
   sum = 0;
   count =0;
   }
}
void pirSensor(void)
{
  myGLCD.setColor(100, 155, 203);
  myGLCD.fillRoundRect (10, 10, 60, 36);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 10, 60, 36);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(100, 155, 203);
  myGLCD.print("<-", 18, 15);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Back to Sensor Menu", 70, 18);
  myGLCD.setFont(BigFont);
  myGLCD.print("PIR Sensor", CENTER, 50);
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawLine(0,100,319,100);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  //myGLCD.print("", 5, 120);
 // myGLCD.print("Basement:", 20, 150);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
 // myGLCD.printNumI(waterlevel,130, 145, 3,'0');
  myGLCD.setFont(BigFont);
  delay(1000); // Check for new value every 1 sec
  myGLCD.setColor(100, 155, 203);
  drawBackButton();
  
}
void smokeSensor(void)
{
  myGLCD.print("SMOKE SENSOR", CENTER, 60);
  myGLCD.setColor(100, 155, 203);
  drawBackButton();
}

void waterSensor(void)
{
  drawBackButton();
  myGLCD.setFont(BigFont);
  myGLCD.print("Water Level Sensor", CENTER, 50);
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawLine(0,100,319,100);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("Water level in", 5, 120);
  myGLCD.print("Basement:", 20, 150);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
  
  
  myGLCD.setFont(BigFont);
  delay(1000); // Check for new value every 1 sec
  myGLCD.setColor(100, 155, 203);
  
}
 

void indoorLights(void)
{
  drawBackButton();
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("House System", CENTER, 10); // Prints the string on the screen
  myGLCD.setColor(255, 0, 0); // Sets color to red
  myGLCD.drawLine(0,32,319,32); // Draws the red line
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont);
  myGLCD.print("Select Lighting", CENTER, 34);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (0, 70, 125, 95);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (0, 70, 125, 95);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("KITCHEN ", LEFT, 72);
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (0, 100, 125, 125);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (0, 100, 125, 125);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Master 2", LEFT, 102);
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (0, 130, 125, 155);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (0, 130, 125, 155);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Bathroom", LEFT, 132);
 
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (0, 160, 125, 185);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (0, 160, 125, 185);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("LivingRM", LEFT, 162);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (0, 190, 125, 215);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (0, 190, 125, 215);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Guest RM", LEFT, 192);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (180, 70, 315, 95);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (180, 70, 315, 95);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("BATHROOM2", RIGHT, 72);
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (180, 100, 315, 125);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (180, 100, 315, 125);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("STUDIO", RIGHT, 102);
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (180, 130, 315, 155);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (180, 130, 315, 155);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("GARAGE", RIGHT, 132);
 
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (180, 160, 315, 185);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (180, 160, 315, 185);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("BASEMENT", RIGHT, 162);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (180, 190, 315, 215);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (180, 190, 315, 215);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("MASTER 1", RIGHT, 192);

  myGLCD.setColor(VGA_BLUE);
  myGLCD.fillCircle(155,80,15);
  myGLCD.drawCircle(155,80,15);
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(255, 255, 255);
  myGLCD.printNumI(1,147,74);
   
  myGLCD.setColor(VGA_BLUE);
  myGLCD.fillCircle(155,118,15);
  myGLCD.drawCircle(155,118,15);
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(255, 255, 255);
  myGLCD.printNumI(2,147,104);

  myGLCD.setColor(VGA_BLUE);
  myGLCD.fillCircle(155,156,15);
  myGLCD.drawCircle(155,156,15);
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(255, 255, 255);
  myGLCD.printNumI(3,147,144);

  myGLCD.setColor(VGA_BLUE);
  myGLCD.fillCircle(155,194,15);
  myGLCD.drawCircle(155,194,15);
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(255, 255, 255);
  myGLCD.printNumI(4,147,184);

}

void outdoorLights(void)
{
  drawBackButton();
//  myGLCD.print("OUTDOOR LIGHTS", CENTER, 60);
  myGLCD.setColor(100, 155, 203);
  myGLCD.setBackColor(0,0,0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("Select Lighting", CENTER, 10); // Prints the string on the screen
  myGLCD.setColor(255, 0, 0); // Sets color to red
  myGLCD.drawLine(0,32,319,32); // Draws the red line
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont);
  myGLCD.print("OUTDOOR LIGHTS", CENTER, 34);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 70, 285, 90);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 70, 285, 90);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("BACKYARD 1", CENTER, 72);
  
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (35, 95, 285, 115); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (35, 95, 285, 115); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("BACKYARD 2", CENTER, 97); // Prints the string
  
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 120, 285, 140);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 120, 285, 140);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Right Side 1", CENTER, 122);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 145, 285, 165);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 145, 285, 165);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Right Side 2", CENTER, 147);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 170, 285, 190);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 170, 285, 190);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Left Side 1", CENTER, 172);

  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 195, 285, 215);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 195, 285, 215);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("Lef Side 2", CENTER, 197);
}

void garageMotor(void)
{
  drawBackButton();
  myGLCD.print("HouseSecuritySystem", CENTER, 50);
  myGLCD.print("Garage Motor", CENTER, 80);
  myGLCD.setColor(VGA_RED);
  myGLCD.fillCircle(70,146,50);
  myGLCD.drawCircle(70,146,50);
  myGLCD.setBackColor(VGA_RED);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Close",30,144);

  myGLCD.setColor(VGA_GREEN);
  myGLCD.fillCircle(240,146,50);
  myGLCD.drawCircle(240,146,50);
  myGLCD.setBackColor(VGA_GREEN);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Open",210,140);

  if(motorOff == 1 && motorOn == 0)
     {
     motorOn = 0;
     myGLCD.setBackColor(VGA_RED);
     myGLCD.setColor(VGA_WHITE);
     myGLCD.print("Garage: Closing", 20, 220);
     Serial1.write('1');
  // runMotor();
     }

  if(motorOn ==1 && motorOff == 0)
     {
     motorOff =0;
     myGLCD.print("Garage: Opening", 20, 220);
     Serial1.write('2');
    //stopMotor();
     }
}

void hvacMotor(void)
{
  drawBackButton();
  myGLCD.print("HouseSecuritySystem", CENTER, 50);
  myGLCD.print("HVAC Motor", CENTER, 80);
  myGLCD.setColor(VGA_GREEN);
  myGLCD.fillCircle(70,146,50);
  myGLCD.drawCircle(70,146,50);
  myGLCD.setBackColor(VGA_GREEN);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("START",35,144);

  myGLCD.setColor(VGA_RED);
  myGLCD.fillCircle(240,146,50);
  myGLCD.drawCircle(240,146,50);
  myGLCD.setBackColor(VGA_RED);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("STOP",210,140);
 
          if(motorCount == 1 or 3 or 5 or 7 or 9)
     {
     
     myGLCD.print("HEATING IS ON", 20, 220);
     } 
   if(hvacOff == 1)
     {


    // myGLCD.setColor(VGA_RED);
   //  delay(2000);
     Serial1.write('+');
     myGLCD.setBackColor(VGA_RED);
    
     }
              if(motorCount == 2 or 4 or 6 or 8)
     {
     
     myGLCD.print("COOLING IS ON", 20, 220);
     }
  if(hvacOn ==1)
        {
motorCount++;

     //    delay(2000);
         Serial1.write('-'); 
         if(motorCount == 2 or 4 or 6 or 8)
         {
      //   myGLCD.print("A/C IS ON", 20, 220);
         }
        }

    if(hvacStop == 1)
    {
       Serial1.write('!');
       myGLCD.print("MOTOR STOPPED", 20, 220);
    }
}
//====================================================


//====================================================


//====================================================
