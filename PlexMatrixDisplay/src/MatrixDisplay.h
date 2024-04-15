// ******************************************* BEGIN MATRIX DISPLAY *******************************************
#pragma region MATRIX_DISPLAY

#include <stdint.h>
#include <FS.h>
#include <SPIFFS.h>
#include <JPEGDEC.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "picopixel.h"
#include "Fonts/FreeSerifBold9pt7b.h"

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANELS_NUMBER 1

MatrixPanel_I2S_DMA *dma_display = nullptr;
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
uint16_t myORANGE = dma_display->color565(255, 140, 0);
uint16_t myPURPLE = dma_display->color565(138, 43, 226);
uint16_t myCYAN = dma_display->color565(0, 204, 204);
uint16_t myGOLD = dma_display->color565(204, 204, 0);

JPEGDEC jpeg;

const char *ALBUM_ART = "/album.jpg";

#ifdef ENABLE_MATRIX_DISPLAY

void displaySetup()
{
  Serial.println("matrix display setup");
  HUB75_I2S_CFG mxconfig(
      PANEL_WIDTH,  // module width
      PANEL_HEIGHT, // module height
      PANELS_NUMBER // Chain length
  );
  mxconfig.gpio.e = 18;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->setTextWrap(false);
  dma_display->begin();
}

// This next function will be called during decoding of the jpeg file to render each block to the Matrix.
int JPEGDraw(JPEGDRAW *pDraw)
{
  // Stop further decoding as image is running off bottom of screen
  if (pDraw->y >= dma_display->height())
    return 1;
  dma_display->drawRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}

fs::File myfile;
void *myOpen(const char *filename, int32_t *size)
{
  myfile = SPIFFS.open(filename);
  *size = myfile.size();
  return &myfile;
}

void myClose(void *handle)
{
  if (myfile)
    myfile.close();
}

int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!myfile)
    return 0;
  return myfile.read(buffer, length);
}

int32_t mySeek(JPEGFILE *handle, int32_t position)
{
  if (!myfile)
    return 0;
  return myfile.seek(position);
}

void showDefaultScreen()
{
  dma_display->fillScreen(myBLACK);
}

void displayRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  dma_display->fillRect(x, y, w, h, color);
}

// Image Related
void clearScreen()
{
  dma_display->fillScreen(myBLACK);
}

void printCenter(const char *buf, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
  int16_t x1, y1;
  uint16_t w, h;
  dma_display->setFont(&font);
  dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);

  // Clear the screen
  displayRect(0, y - h - 1, PANEL_WIDTH, h + 4, myBLACK);

  dma_display->setCursor(PANEL_WIDTH / 2 - (w / 2), y);
  dma_display->setTextColor(textColor);
  dma_display->print(buf);
}

void printLeft(const char *buf, int x, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
  int16_t x1, y1;
  uint16_t w, h;
  dma_display->setFont(&font);
  dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);

  // Clear the screen
  displayRect(x, y - h - 1, w, h + 4, myBLACK);

  dma_display->setCursor(x, y);
  dma_display->setTextColor(textColor);
  dma_display->print(buf);
}

enum ScrollState
{
  SCROLL_IDLE,
  SCROLL_INIT,
  SCROLL_RUNNING
};

char previousScrollingText[100] = "";
char previousScrollingText2[100] = "";
bool isTextScrolling = false;
bool isTextScrolling2 = false;

void printScrolling(const char *buf, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
  static int16_t x1, y1;
  static uint16_t w, h;
  static uint16_t textWidth;
  static int clearOriginY = y - 5;
  static int clearHeight = 7;
  static int16_t xPos;
  static unsigned long lastScrollTime;
  static int scrollSpeed = 100; // Adjust scroll speed here (milliseconds per step)
  static ScrollState state = SCROLL_IDLE;

  switch (state)
  {
  case SCROLL_IDLE:
    // Set the font and get the width of the text
    dma_display->setFont(&font);
    dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);
    textWidth = w;

    // If the text width is greater than the screen width, start scrolling
    if (textWidth > PANEL_WIDTH)
    {
      // Initialize scrolling position
      xPos = PANEL_WIDTH;
      lastScrollTime = millis();
      state = SCROLL_RUNNING;
    }
    else
    {
      if (strcmp(previousScrollingText, buf) != 0)
      {
        // If the text width is not greater than the screen width, print it centered
        // Clear part of the screen for the scrolling text
        displayRect(0, clearOriginY, PANEL_WIDTH, clearHeight, 0);

        dma_display->setCursor(32 - (w / 2), y);
        dma_display->setTextColor(textColor);
        dma_display->print(buf);
        state = SCROLL_IDLE; // Reset state to IDLE if no scrolling is needed
        strcpy(previousScrollingText, buf);
      }
    }
    break;

  case SCROLL_RUNNING:
    isTextScrolling = true;
    // Calculate time since last scroll
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastScrollTime;

    // If enough time has elapsed, scroll the text
    if (elapsedTime >= scrollSpeed)
    {
      // Clear part of the screen for the scrolling text
      displayRect(0, clearOriginY, PANEL_WIDTH, clearHeight, 0);

      // Calculate the new position for the text
      int16_t displayX = xPos;

      // Print the text at the new position
      dma_display->setCursor(displayX, y);
      dma_display->setTextColor(textColor);
      dma_display->print(buf);

      // Update last scroll time
      lastScrollTime = currentTime;

      // Move the text to the left
      xPos--;

      if (xPos == -textWidth - 1) // added -1 so that the text completely clears the screen.
      {
        // If the text has scrolled completely off the screen, reset xPos to start over
        xPos = PANEL_WIDTH;
        state = SCROLL_IDLE;
        isTextScrolling = false;
      }
    }
    break;
  }
}

void printScrolling2(const char *buf, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
  static int16_t x1, y1;
  static uint16_t w, h;
  static uint16_t textWidth;
  static int clearOriginY = y - 5;
  static int clearHeight = 7;
  static int16_t xPos;
  static unsigned long lastScrollTime;
  static int scrollSpeed = 100; // Adjust scroll speed here (milliseconds per step)
  static ScrollState state = SCROLL_IDLE;

  switch (state)
  {
  case SCROLL_IDLE:
    // Set the font and get the width of the text
    dma_display->setFont(&font);
    dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);
    textWidth = w;

    // If the text width is greater than the screen width, start scrolling
    if (textWidth > PANEL_WIDTH)
    {
      // Initialize scrolling position
      xPos = PANEL_WIDTH;
      lastScrollTime = millis();
      state = SCROLL_RUNNING;
    }
    else
    {
      if (strcmp(previousScrollingText2, buf) != 0)
      {
        // If the text width is not greater than the screen width, print it centered
        // Clear part of the screen for the scrolling text
        displayRect(0, clearOriginY, PANEL_WIDTH, clearHeight, 0);

        dma_display->setCursor(32 - (w / 2), y);
        dma_display->setTextColor(textColor);
        dma_display->print(buf);
        state = SCROLL_IDLE; // Reset state to IDLE if no scrolling is needed
        strcpy(previousScrollingText2, buf);
      }
    }
    break;

  case SCROLL_RUNNING:
    if (!isTextScrolling)
    {
      isTextScrolling2 = false;
    }
    else
    {
      isTextScrolling2 = true;
    }
    // Calculate time since last scroll
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastScrollTime;

    // If enough time has elapsed, scroll the text
    if (elapsedTime >= scrollSpeed)
    {
      // Clear part of the screen for the scrolling text
      displayRect(0, clearOriginY, PANEL_WIDTH, clearHeight, 0);

      // Calculate the new position for the text
      int16_t displayX = xPos;

      // Print the text at the new position
      dma_display->setCursor(displayX, y);
      dma_display->setTextColor(textColor);
      dma_display->print(buf);

      // Update last scroll time
      lastScrollTime = currentTime;

      // Move the text to the left
      xPos--;

      if (xPos == -textWidth - 1) // added -1 so that the text completely clears the screen.
      {
        // If the text has scrolled completely off the screen, reset xPos to start over
        xPos = PANEL_WIDTH;
        state = SCROLL_IDLE;
        isTextScrolling2 = false;
      }
    }
    break;
  }
}

int drawImagefromFile(const char *imageFileUri, int offset)
{
  unsigned long lTime = millis();
  lTime = millis();
  jpeg.open((const char *)imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
  // int decodeStatus = jpeg.decode(0, 0, 0);
  int decodeStatus = jpeg.decode(offset, 8, 0);
  jpeg.close();
  Serial.print("Time taken to decode and display Image (ms): ");
  Serial.println(millis() - lTime);
  return decodeStatus;
}

uint16_t color565(uint32_t rgb)
{
  return (((rgb >> 16) & 0xF8) << 8) |
         (((rgb >> 8) & 0xFC) << 3) |
         ((rgb & 0xFF) >> 3);
};

void drawBitmap(int startx, int starty, int width, int height, uint32_t *bitmap)
{
  int counter = 0;
  for (int yy = 0; yy < height; yy++)
  {
    for (int xx = 0; xx < width; xx++)
    {
      dma_display->drawPixel(startx + xx, starty + yy, color565(bitmap[counter]));
      counter++;
    }
  }
}

// Draw the bitmap, with an option to enlarge it by a factor of two
void drawBitmap(int startx, int starty, int width, int height, uint32_t *bitmap, bool enlarged)
{
  int counter = 0;
  if (enlarged)
  {
    for (int yy = 0; yy < height; yy++)
    {
      for (int xx = 0; xx < width; xx++)
      {
        dma_display->drawPixel(startx + 2 * xx, starty + 2 * yy, color565(bitmap[counter]));
        dma_display->drawPixel(startx + 2 * xx + 1, starty + 2 * yy, color565(bitmap[counter]));
        dma_display->drawPixel(startx + 2 * xx, starty + 2 * yy + 1, color565(bitmap[counter]));
        dma_display->drawPixel(startx + 2 * xx + 1, starty + 2 * yy + 1, color565(bitmap[counter]));
        counter++;
      }
    }
  }
  else
    drawBitmap(startx, starty, width, height, bitmap);
}

#pragma endregion
// ******************************************* END MATRIX DISPLAY *********************************************

#else

void displaySetup()
{
#warning "Header file not defined, function implementation ignored"
}

void clearScreen()
{
#warning "Header file not defined, function implementation ignored"
}

void printCenter(const char *buf, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
#warning "Header file not defined, function implementation ignored"
}

void drawBitmap(int startx, int starty, int width, int height, uint32_t *bitmap)
{
#warning "Header file not defined, function implementation ignored"
}

void drawBitmap(int startx, int starty, int width, int height, uint32_t *bitmap, bool enlarged)
{
#warning "Header file not defined, function implementation ignored"
}

void displayRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
#warning "Header file not defined, function implementation ignored"
}

void printLeft(const char *buf, int x, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
#warning "Header file not defined, function implementation ignored"
}

void printScrolling(const char *buf, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
#warning "Header file not defined, function implementation ignored"
}

void printScrolling2(const char *buf, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
#warning "Header file not defined, function implementation ignored"
}

#endif