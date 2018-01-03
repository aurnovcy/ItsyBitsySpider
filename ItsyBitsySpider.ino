#include <SPI.h>
#include <SD.h>

// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <time.h> 
#include <stdlib.h>
int difficulty = 1;
boolean LLOn = false;
boolean LROn = false;
int scr;
boolean RLOn = false;
boolean RROn = false;
boolean gameOver = false;
double totTime = 0;
int lastRefreshTime;
int lives = 1000;
int previousTime;
double lastUpdateLeft;
double lastUpdateRight;
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines     to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define GRAY    0x8410
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

boolean isSpikeLL = false;
boolean isSpikeLR = false;
boolean isSpikeRL = false;
boolean isSpikeRR = false;
boolean isLClear = true;
boolean isRClear = true; 
byte spikeLHeight = 0;
byte spikeRHeight = 0; 

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;

boolean isSpiderRL = true;
boolean isSpiderLL = true;
boolean isSpiderRR = false;
boolean isSpiderLR = false;  

void setup(void) {
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  //SD.begin(10);
  Serial.begin(9600);
  tft.setRotation(2);
  //Serial.println("TFT LCD test"));
  /*
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(10)) {
    Serial.println(F("failed!"));
    return;
  }
  Serial.println(F("OK!"));
  */
  testText();
  while(digitalRead(11)==HIGH && digitalRead(12)==HIGH){
    delay(50);
  }
  if (digitalRead(11)==LOW){
    difficulty=2;
  }
}

void loop(void) {
  scr = 0;
  totTime = 0;
  if (difficulty==1){
    lives = 1000;
    scr = 0;
  }
  if (difficulty==2){
    lives = 600;
    scr = 0;
  }
  tft.setRotation(0);
  int refreshInterval = 500;
  setBackground(tft);
  while(gameOver == false && difficulty==1){
    unsigned long now = millis();
    //SD.begin(10);
    //bmpDraw("ArduinoFiredSpike.bmp",0,0,tft);
    if (now%500==00){
      updateScore(tft);
    }
    if (now%20==00){
      deductLives();
    }
    if (now%3000==0){
      startStream();
    }
    if ((now - lastRefreshTime) > refreshInterval)
    {   
        lastRefreshTime = now;
    }
    if (digitalRead(11)==LOW || digitalRead(12)==LOW){
      if (now - lastUpdateLeft > 1000 || now - lastUpdateRight > 1000){
        updateSpiders(now);
      }
    }
    totTime = now/10;
  }
  while(gameOver==false && difficulty==2){
    unsigned long now = millis();
    //SD.begin(10);
    //bmpDraw("ArduinoFiredSpike.bmp",0,0,tft);
    if (now%500==00){
      updateScore(tft);
    }
    if (now%20==00){
      deductLives();
    }
    if (now%3000==0){
      startStream();
    }
    if ((now - lastRefreshTime) > refreshInterval)
    {   
        lastRefreshTime = now;
    }
    if (digitalRead(11)==LOW || digitalRead(12)==LOW){
      if (now - lastUpdateLeft > 1000 || now - lastUpdateRight > 1000){
        updateSpiders(now);
      }
    }
    totTime = now/10;
  }
  gameText();
  while (digitalRead(11)==HIGH && digitalRead(12)==HIGH){
    delay(50);
  }
  gameOver = false;
}

void startStream(){
  if (LLOn == true){
    dryLL();
    LLOn = false;
  }
  if (LROn ==true){
    dryLR();
    LROn = false;
  }
  if (RLOn == true){
    dryRL();
    RLOn = false;
  }
  if (RROn == true){
    dryRR();
    RROn = false;
  }
  delay(50);
  int selection = random(1,5);
  if (selection==1){
    wetLL();
    LLOn = true;
  }
  else if(selection==2){
    wetLR();
    LROn = true;
  }
  else if (selection==3){
    wetRL();
    RLOn = true;
  }
  else{
    wetRR();
    RROn = true;
  }
}

boolean checkSame(){
  boolean isSame = false;
  if (LLOn==true && isSpiderLL==true){
    isSame = true;
  }
  if (RLOn==true && isSpiderRL==true){
    isSame = true;
  }
  if (RROn==true && isSpiderRR==true){
    isSame = true;
  }
  if (LROn==true && isSpiderLR==true){
    isSame = true;
  }
  return (isSame);
}

void updateSpiders (unsigned long now){
  if(digitalRead(11) == LOW){
      switchSpiderRight();
      lastUpdateRight = now;
  }
  if(digitalRead(12) == LOW){
      switchSpiderLeft(); 
      lastUpdateLeft = now;
  }
}

void deductLives(){
  if(checkSame()==true){
    lives--;
  }
  if (lives<0){
    gameOver = true;
  }
}

void updateScore(Adafruit_TFTLCD tft){
  tft.setRotation(2);
  scr = totTime/100 - 5;
  int tempTot = totTime;
  if (tempTot%2==0){
    tft.fillRect(250, 15, 50, 50, GRAY);
    tft.setCursor(260,20);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println(scr);
  }

  tft.setRotation(0);
  /*
  tft.setCursor(220,20);
  tft.setTextSize(2);
  tft.setTextColor(MAGENTA);
  tft.println(previousTime);
  tft.setTextColor(BLACK);
  tft.println(totTime/100);
  previousTime = totTime/100;
  */
}

void setBackground(Adafruit_TFTLCD tft){
  unsigned long start; 
  start = micros(); 
  tft.fillScreen(MAGENTA);

  tft.fillRect(0, 0, 80, 480, GRAY);
  tft.drawRect(0, 0, 80, 480, GRAY);

  tft.fillRect(80, 0, 160, 480, MAGENTA);
  tft.drawRect(80, 0, 160, 480, MAGENTA);

  tft.fillRect(240, 0, 80, 480, GRAY);
  tft.drawRect(240, 0, 80, 480, GRAY);

  tft.drawLine(120, 0, 120, 480, BLACK);
  tft.drawRect(160, 0, 160, 480, MAGENTA);
  tft.drawLine(200, 0, 200, 480, BLACK);

  drawSpiderLL(tft);
  drawSpiderRR(tft);
  //return micros() - start;
}

void drawSpider(int xPos, Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  
  // Draw the central core of the spider
  const byte yPosCenter = 80;
  const byte xPosCenter = xPos; 
  const byte centralCircleRadius = 10;
  tft.drawCircle(xPosCenter, yPosCenter, centralCircleRadius, BLACK); 
  tft.fillCircle(xPosCenter, yPosCenter, centralCircleRadius, BLACK);

  // Draw the first leg of the spider
  const byte leg1xStart = xPosCenter;
  const byte leg1yStart = yPosCenter;
  const byte leg1xEnd = xPosCenter + 20;   
  const byte leg1yEnd = yPosCenter + 20; 
  tft.drawLine(leg1xStart, leg1yStart, leg1xEnd, leg1yEnd, BLACK); 

  // Draw the second leg of the spider
  const byte leg2xStart = xPosCenter;
  const byte leg2yStart = yPosCenter;
  const byte leg2xEnd = xPosCenter + 20;   
  const byte leg2yEnd = yPosCenter + 8; 
  tft.drawLine(leg2xStart, leg2yStart, leg2xEnd, leg2yEnd, BLACK);

  // Draw the third leg of the spider
  const byte leg3xStart = xPosCenter;
  const byte leg3yStart = yPosCenter;
  const byte leg3xEnd = xPosCenter + 20;   
  const byte leg3yEnd = yPosCenter - 8; 
  tft.drawLine(leg3xStart, leg3yStart, leg3xEnd, leg3yEnd, BLACK); 

  // Draw the fourth leg of the spider
  const byte leg4xStart = xPosCenter;
  const byte leg4yStart = yPosCenter;
  const byte leg4xEnd = xPosCenter + 20;   
  const byte leg4yEnd = yPosCenter - 20; 
  tft.drawLine(leg4xStart, leg4yStart, leg4xEnd, leg4yEnd, BLACK); 

  // Draw the fifth leg of the spider
  const byte leg5xStart = xPosCenter;
  const byte leg5yStart = yPosCenter;
  const byte leg5xEnd = xPosCenter - 20;   
  const byte leg5yEnd = yPosCenter + 20; 
  tft.drawLine(leg5xStart, leg5yStart, leg5xEnd, leg5yEnd, BLACK); 

  // Draw the sixth leg of the spider
  const byte leg6xStart = xPosCenter;
  const byte leg6yStart = yPosCenter;
  const byte leg6xEnd = xPosCenter - 20;   
  const byte leg6yEnd = yPosCenter + 8; 
  tft.drawLine(leg6xStart, leg6yStart, leg6xEnd, leg6yEnd, BLACK);

  // Draw the seventh leg of the spider
  const byte leg7xStart = xPosCenter;
  const byte leg7yStart = yPosCenter;
  const byte leg7xEnd = xPosCenter - 20;   
  const byte leg7yEnd = yPosCenter - 8; 
  tft.drawLine(leg7xStart, leg7yStart, leg7xEnd, leg7yEnd, BLACK); 

  // Draw the eigth leg of the spider
  const byte leg8xStart = xPosCenter;
  const byte leg8yStart = yPosCenter;
  const byte leg8xEnd = xPosCenter - 20;   
  const byte leg8yEnd = yPosCenter - 20; 
  tft.drawLine(leg8xStart, leg8yStart, leg8xEnd, leg8yEnd, BLACK); 
}

void drawMAGENTASpider(int xPos, Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  
  // Draw the central core of the spider
  const byte yPosCenter = 80;
  const byte xPosCenter = xPos; 
  const byte centralCircleRadius = 10;
  tft.drawCircle(xPosCenter, yPosCenter, centralCircleRadius, MAGENTA); 
  tft.fillCircle(xPosCenter, yPosCenter, centralCircleRadius, MAGENTA);

  // Draw the first leg of the spider
  const byte leg1xStart = xPosCenter;
  const byte leg1yStart = yPosCenter;
  const byte leg1xEnd = xPosCenter + 20;   
  const byte leg1yEnd = yPosCenter + 20; 
  tft.drawLine(leg1xStart, leg1yStart, leg1xEnd, leg1yEnd, MAGENTA); 

  // Draw the second leg of the spider
  const byte leg2xStart = xPosCenter;
  const byte leg2yStart = yPosCenter;
  const byte leg2xEnd = xPosCenter + 20;   
  const byte leg2yEnd = yPosCenter + 8; 
  tft.drawLine(leg2xStart, leg2yStart, leg2xEnd, leg2yEnd, MAGENTA);

  // Draw the third leg of the spider
  const byte leg3xStart = xPosCenter;
  const byte leg3yStart = yPosCenter;
  const byte leg3xEnd = xPosCenter + 20;   
  const byte leg3yEnd = yPosCenter - 8; 
  tft.drawLine(leg3xStart, leg3yStart, leg3xEnd, leg3yEnd, MAGENTA); 

  // Draw the fourth leg of the spider
  const byte leg4xStart = xPosCenter;
  const byte leg4yStart = yPosCenter;
  const byte leg4xEnd = xPosCenter + 20;   
  const byte leg4yEnd = yPosCenter - 20; 
  tft.drawLine(leg4xStart, leg4yStart, leg4xEnd, leg4yEnd, MAGENTA); 

  // Draw the fifth leg of the spider
  const byte leg5xStart = xPosCenter;
  const byte leg5yStart = yPosCenter;
  const byte leg5xEnd = xPosCenter - 20;   
  const byte leg5yEnd = yPosCenter + 20; 
  tft.drawLine(leg5xStart, leg5yStart, leg5xEnd, leg5yEnd, MAGENTA); 

  // Draw the sixth leg of the spider
  const byte leg6xStart = xPosCenter;
  const byte leg6yStart = yPosCenter;
  const byte leg6xEnd = xPosCenter - 20;   
  const byte leg6yEnd = yPosCenter + 8; 
  tft.drawLine(leg6xStart, leg6yStart, leg6xEnd, leg6yEnd, MAGENTA);

  // Draw the seventh leg of the spider
  const byte leg7xStart = xPosCenter;
  const byte leg7yStart = yPosCenter;
  const byte leg7xEnd = xPosCenter - 20;   
  const byte leg7yEnd = yPosCenter - 8; 
  tft.drawLine(leg7xStart, leg7yStart, leg7xEnd, leg7yEnd, MAGENTA); 

  // Draw the eigth leg of the spider
  const byte leg8xStart = xPosCenter;
  const byte leg8yStart = yPosCenter;
  const byte leg8xEnd = xPosCenter - 20;   
  const byte leg8yEnd = yPosCenter - 20; 
  tft.drawLine(leg8xStart, leg8yStart, leg8xEnd, leg8yEnd, MAGENTA); 
}

void drawSpiderLL(Adafruit_TFTLCD tft)
{
  drawSpider(100, tft);
  isSpiderLL = true;
  isSpiderLR = false;
}

void drawSpiderLR(Adafruit_TFTLCD tft)
{
  drawSpider(140, tft);
  isSpiderLL = false;
  isSpiderLR = true; 
}

void drawSpiderRL(Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  drawSpider(180, tft);
  isSpiderRL = true;
  isSpiderRR = false;
}

void drawSpiderRR(Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  drawSpider(220, tft);
  isSpiderRL = false;
  isSpiderRR = true;
}

void drawSpiderMAGENTALL(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(100, tft);
  isSpiderLL = false;
  isSpiderLR = true;
}

void drawSpiderMAGENTALR(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(140, tft); 
  isSpiderLL = true;
  isSpiderLR = false;
}

void drawSpiderMAGENTARL(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(180, tft);
  isSpiderRL = false;
  isSpiderRR = true;
}

void drawSpiderMAGENTARR(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(220, tft);
  isSpiderRL = true;
  isSpiderRR = false;
}

void switchSpiderRight()
{
  if(isSpiderRL == true)
  {
    isSpiderRL = false;
    isSpiderRR = true; 
    drawSpiderMAGENTARL(tft); 
    drawSpiderRR(tft); 
  }
  else
  {  
    isSpiderRL = true; 
    isSpiderRR = false; 
    drawSpiderMAGENTARR(tft); 
    drawSpiderRL(tft); 
  }
}

void switchSpiderLeft()
{
  if(isSpiderLL == true)
  {
    isSpiderLL = false;
    isSpiderLR = true;
    drawSpiderMAGENTALL(tft); 
    drawSpiderLR(tft); 
  }
  else
  {
    isSpiderLL = true;
    isSpiderLR = false; 
    drawSpiderMAGENTALR(tft); 
    drawSpiderLL(tft); 
  }
}

unsigned long gameText() {
  tft.setRotation(2);
  tft.fillScreen(RED);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE); 
  tft.setTextSize(5);
  tft.println("********************");
  tft.setTextColor(BLACK);
  tft.setTextSize(8);
  tft.println("GAME");
  tft.println("OVER");
  tft.setTextSize(3);
  tft.println("Unplug and replug to play again.");
  tft.println("Your final score was:");
  tft.println(scr);
  tft.setTextColor(WHITE);
  tft.setTextSize(5);
  tft.println("********************");
  tft.setRotation(0);
  scr = 0;
}

unsigned long testText() {
  tft.fillScreen(BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  
  tft.setTextColor(WHITE);  tft.setTextSize(5);
  tft.println("********************");
  tft.println("ITSY BITSY");
  tft.println("SPIDER");
  tft.println(); 
  
  tft.setTextColor(RED); 
  tft.setTextSize(3);
  tft.println("Press BLUE for");
  tft.println("beginner");
  tft.println("Press RED for");
  tft.println("advanced"); 
  tft.println(); 
  
  tft.setTextColor(MAGENTA);
  tft.setTextSize(4);
  tft.println(); 
  tft.println("By: Engenies");

  tft.setTextColor(WHITE);  
  tft.setTextSize(5);
  tft.println("********************");
  
  return micros() - start;
}


int xLL = 80;
int xLMiddle = 120; 
int xCenter = 160;
int xRMiddle = 200;
int xRR = 240; 

int yBase = 480;
 
void wetLL()
{
  tft.fillRect(80, 0, 40, 480, CYAN);
  tft.drawRect(80, 0, 40, 480, CYAN);
}

void wetLR()
{
  tft.fillRect(120, 0, 40, 480, CYAN);
  tft.drawRect(120, 0, 40, 480, CYAN);
}

void wetRL()
{
  tft.fillRect(160, 0, 40, 480, CYAN);
  tft.drawRect(160, 0, 40, 480, CYAN);
}

void wetRR()
{
  tft.fillRect(200, 0, 40, 480, CYAN);
  tft.drawRect(200, 0, 40, 480, CYAN);
}

void dryLL()
{
  tft.fillRect(80, 0, 40, 480, MAGENTA);
  tft.drawRect(80, 0, 40, 480, MAGENTA);
}

void dryLR()
{
  tft.fillRect(120, 0, 40, 480, MAGENTA);
  tft.drawRect(120, 0, 40, 480, MAGENTA);
}

void dryRL()
{
  tft.fillRect(160, 0, 40, 480, MAGENTA);
  tft.drawRect(160, 0, 40, 480, MAGENTA);
}

void dryRR()
{
  tft.fillRect(200, 0, 40, 480, MAGENTA);
  tft.drawRect(200, 0, 40, 480, MAGENTA);
}

void updateSpikes()
{
   
}
   
