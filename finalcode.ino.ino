#include <SPI.h>

#include <SD.h>

// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <time.h> 
#include <stdlib.h>
boolean gameOver = false;
double totTime = 0;
int lastRefreshTime;
int previousTime;
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
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
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

boolean isRightSpiderLeft = true;
boolean isLeftSpiderLeft = true; 

void setup(void) {
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  //SD.begin(10);
  Serial.begin(9600);
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

}

void loop(void) {
  int refreshInterval = 500;
  setBackground(tft);
  while(gameOver == false){
    unsigned long now = millis();
    //SD.begin(10);
    //bmpDraw("ArduinoFiredSpike.bmp",0,0,tft);
    updateScore(tft);
    //int r = rand() % 2;
    //int GapTime = (1+r) * 1000;
    if (now%50==0){
      updateSpikes();
    }
    if (digitalRead(11)==LOW || digitalRead(12)==LOW){
      if ((now - lastRefreshTime) > refreshInterval)
      {
        updateSpiders();    
        lastRefreshTime = now;
      }
    }
    totTime = now/10;
  }
  gameText();
  delay(5000);
}

void updateSpiders (){
  if(digitalRead(11) == LOW){
      switchSpiderLeft();
  }
  if(digitalRead(12) == LOW){
      switchSpiderRight(); 
  }
}

void updateScore(Adafruit_TFTLCD tft){
  int modTime = totTime/10;
  if (modTime%20==0){
    tft.fillRect(215, 15, 50, 50, GREEN);
    tft.setCursor(225,20);
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.println(modTime);
  }
  
  /*
  tft.setCursor(220,20);
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.println(previousTime);
  tft.setTextColor(BLACK);
  tft.println(totTime/100);
  previousTime = totTime/100;
  */
}

void setBackground(Adafruit_TFTLCD tft){
  unsigned long start; 
  start = micros(); 
  tft.fillScreen(GREEN);
  tft.fillRect(0, 0, 10, 480, GRAY);
  tft.drawRect(0, 0, 10, 480, GRAY);
  tft.fillRect(10, 0, 140, 480, GREEN);
  tft.drawRect(10, 0, 140, 480, GREEN);
  tft.fillRect(150, 0, 20, 480, GRAY);
  tft.drawRect(150, 0, 20, 480, GRAY);
  tft.fillRect(170, 0, 140, 480, GREEN);
  tft.drawRect(170, 0, 140, 480, GREEN); 
  tft.fillRect(310, 0, 10, 480, GRAY);
  tft.drawRect(310, 0, 10, 480, GRAY);
  drawSpiderLL(tft);
  drawSpiderRR(tft);
  //return micros() - start;
}

void drawSpider(int xPos, Adafruit_TFTLCD tft)
{

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

void drawGreenSpider(int xPos, Adafruit_TFTLCD tft)
{

  // Draw the central core of the spider
  const byte yPosCenter = 80;
  const byte xPosCenter = xPos; 
  const byte centralCircleRadius = 10;
  tft.drawCircle(xPosCenter, yPosCenter, centralCircleRadius, GREEN); 
  tft.fillCircle(xPosCenter, yPosCenter, centralCircleRadius, GREEN);

  // Draw the first leg of the spider
  const byte leg1xStart = xPosCenter;
  const byte leg1yStart = yPosCenter;
  const byte leg1xEnd = xPosCenter + 20;   
  const byte leg1yEnd = yPosCenter + 20; 
  tft.drawLine(leg1xStart, leg1yStart, leg1xEnd, leg1yEnd, GREEN); 

  // Draw the second leg of the spider
  const byte leg2xStart = xPosCenter;
  const byte leg2yStart = yPosCenter;
  const byte leg2xEnd = xPosCenter + 20;   
  const byte leg2yEnd = yPosCenter + 8; 
  tft.drawLine(leg2xStart, leg2yStart, leg2xEnd, leg2yEnd, GREEN);

  // Draw the third leg of the spider
  const byte leg3xStart = xPosCenter;
  const byte leg3yStart = yPosCenter;
  const byte leg3xEnd = xPosCenter + 20;   
  const byte leg3yEnd = yPosCenter - 8; 
  tft.drawLine(leg3xStart, leg3yStart, leg3xEnd, leg3yEnd, GREEN); 

  // Draw the fourth leg of the spider
  const byte leg4xStart = xPosCenter;
  const byte leg4yStart = yPosCenter;
  const byte leg4xEnd = xPosCenter + 20;   
  const byte leg4yEnd = yPosCenter - 20; 
  tft.drawLine(leg4xStart, leg4yStart, leg4xEnd, leg4yEnd, GREEN); 

  // Draw the fifth leg of the spider
  const byte leg5xStart = xPosCenter;
  const byte leg5yStart = yPosCenter;
  const byte leg5xEnd = xPosCenter - 20;   
  const byte leg5yEnd = yPosCenter + 20; 
  tft.drawLine(leg5xStart, leg5yStart, leg5xEnd, leg5yEnd, GREEN); 

  // Draw the sixth leg of the spider
  const byte leg6xStart = xPosCenter;
  const byte leg6yStart = yPosCenter;
  const byte leg6xEnd = xPosCenter - 20;   
  const byte leg6yEnd = yPosCenter + 8; 
  tft.drawLine(leg6xStart, leg6yStart, leg6xEnd, leg6yEnd, GREEN);

  // Draw the seventh leg of the spider
  const byte leg7xStart = xPosCenter;
  const byte leg7yStart = yPosCenter;
  const byte leg7xEnd = xPosCenter - 20;   
  const byte leg7yEnd = yPosCenter - 8; 
  tft.drawLine(leg7xStart, leg7yStart, leg7xEnd, leg7yEnd, GREEN); 

  // Draw the eigth leg of the spider
  const byte leg8xStart = xPosCenter;
  const byte leg8yStart = yPosCenter;
  const byte leg8xEnd = xPosCenter - 20;   
  const byte leg8yEnd = yPosCenter - 20; 
  tft.drawLine(leg8xStart, leg8yStart, leg8xEnd, leg8yEnd, GREEN); 
}

void drawSpiderLL(Adafruit_TFTLCD tft)
{
  drawSpider(50, tft);
}

void drawSpiderLR(Adafruit_TFTLCD tft)
{
  drawSpider(110, tft); 
}

void drawSpiderRL(Adafruit_TFTLCD tft)
{
  drawSpider(200, tft);
}

void drawSpiderRR(Adafruit_TFTLCD tft)
{
  drawSpider(220, tft);
}

void drawSpiderGreenLL(Adafruit_TFTLCD tft)
{
  drawGreenSpider(50, tft);
}

void drawSpiderGreenLR(Adafruit_TFTLCD tft)
{
  drawGreenSpider(110, tft); 
}

void drawSpiderGreenRL(Adafruit_TFTLCD tft)
{
  drawGreenSpider(200, tft);
}

void drawSpiderGreenRR(Adafruit_TFTLCD tft)
{
  drawGreenSpider(220, tft);
}

void switchSpiderRight()
{
  if(isRightSpiderLeft == true)
  {
    isRightSpiderLeft = false; 
    drawSpiderGreenRL(tft); 
    drawSpiderRR(tft); 
  }
  else
  {
    
    isRightSpiderLeft = true; 
    drawSpiderGreenRR(tft); 
    drawSpiderRL(tft); 
  }
}

void switchSpiderLeft()
{
  if(isLeftSpiderLeft == true)
  {
    isLeftSpiderLeft = false; 
    drawSpiderGreenLL(tft); 
    drawSpiderLR(tft); 
  }
  else
  {
    isLeftSpiderLeft = true; 
    drawSpiderGreenLR(tft); 
    drawSpiderLL(tft); 
  }
}

unsigned long gameText() {
  tft.fillScreen(BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  
  tft.setTextColor(WHITE);  tft.setTextSize(5);
  tft.println("GAME OVER");
  tft.println(); 
}

unsigned long testText() {
  tft.fillScreen(BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  
  tft.setTextColor(WHITE);  tft.setTextSize(5);
  tft.println("MAKE THEM FALL");
  tft.println(); 
  
  tft.setTextColor(RED); 
  tft.setTextSize(3);
  tft.println("Press LEFT for beginner");
  tft.println("Press RIGHT for advanced"); 
  tft.println(); 
  
  tft.setTextColor(GREEN);
  tft.setTextSize(4);
  tft.println(); 
  tft.println("By: Engenies");
  
  return micros() - start;
}


void spikeLL()
{
  tft.drawTriangle(20, 480, 60, 480, 40, 480 - spikeLHeight, GRAY);
  tft.fillTriangle(20, 480, 60, 480, 40, 480 - spikeLHeight, GRAY);
}

void spikeLR()
{
  tft.drawTriangle(60, 480, 100, 480, 80, 480 - spikeLHeight, GRAY);
  tft.fillTriangle(60, 480, 100, 480, 80, 480 - spikeLHeight, GRAY);
}

void spikeRL()
{
  tft.drawTriangle(160, 480, 200, 480, 180, 480 - spikeRHeight, GRAY);
  tft.fillTriangle(160, 480, 200, 480, 180, 480 - spikeRHeight, GRAY);
}
void spikeRR()
{
  tft.drawTriangle(200, 480, 240, 480, 220, 480 - spikeRHeight, GRAY);
  tft.fillTriangle(200, 480, 240, 480, 220, 480 - spikeRHeight, GRAY);
}

void clearspikeLL()
{
  tft.drawTriangle(20, 480, 60, 480, 40, 480 - spikeLHeight, GREEN);
  tft.fillTriangle(20, 380, 60, 480, 40, 480 - spikeLHeight, GREEN);
}

void clearspikeLR()
{
  tft.drawTriangle(60, 480, 100, 480, 80, 480 - spikeLHeight, GREEN);
  tft.fillTriangle(60, 480, 100, 480, 80, 480 - spikeLHeight, GREEN);
}

void clearspikeRL()
{
  tft.drawTriangle(160, 480, 200, 480, 180, 480-spikeRHeight, GREEN);
  tft.fillTriangle(160, 480, 200, 480, 180, 480-spikeRHeight, GREEN);
}
void clearspikeRR()
{
  tft.drawTriangle(200, 480, 240, 480, 220, 480 - spikeRHeight, GREEN);
  tft.fillTriangle(200, 480, 240, 480, 220, 480 - spikeRHeight, GREEN);
}

void updateSpikes()
{
  updateLSpikes();
  updateRSpikes();  
}

void updateLSpikes()
{
  if(isLClear) 
  {
    if (rand() < 0.5) isSpikeLL = true;
    else isSpikeLR = true; 

    if(isSpikeLL) 
    {
      spikeLHeight = 20; 
      spikeLL(); 
    }

    else
    {
      spikeLHeight = 20; 
      spikeLR(); 
    }

    isLClear = false; 
  }

  if(spikeLHeight > 900)
  {
    if(isSpikeLL) 
    {
      clearspikeLL();
      isSpikeLL = false;
      spikeLHeight = 0; 
      isLClear = true;
    }
    else
    {
      clearspikeLR();
      isSpikeLR = false;
      spikeLHeight = 0; 
      isLClear = true;
    }
    updateLSpikes();
  }

  else
  {
    if(isSpikeLL)
    {
      spikeLHeight = spikeLHeight + 800; 
      spikeLL();
    }
    else
    {
      spikeLHeight = spikeLHeight + 800;
      spikeLR();
    }
  }
}

void updateRSpikes()
{
  if(isRClear) 
  {
    if (rand() < 0.5) isSpikeLL = true;
    else isSpikeRR = true; 

    if(isSpikeRL) 
    {
      spikeRHeight = 20; 
      spikeRL(); 
    }

    else
    {
      spikeRHeight = 20; 
      spikeRR(); 
    }

    isRClear = false; 
  }

  if(spikeRHeight > 900)
  {
    if(isSpikeRL) 
    {
      clearspikeRL();
      isSpikeRL = false;
      spikeRHeight = 0; 
      isRClear = true;
    }
    else
    {
      clearspikeRR();
      isSpikeRR = false;
      spikeRHeight = 0; 
      isRClear = true;
    }
    updateRSpikes();
  }

  else
  {
    if(isSpikeRL)
    {
      spikeRHeight = spikeRHeight + 20; 
      spikeRL();
    }
    else
    {
      spikeRHeight = spikeRHeight + 20;
      spikeRR();
    }
  }
}


