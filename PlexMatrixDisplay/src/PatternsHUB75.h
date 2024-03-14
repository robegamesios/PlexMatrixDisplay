
/********************************************************************************************************************************************************
 *                                                                                                                                                       *
 *  Project:         FFT Spectrum Analyzer                                                                                                               *
 *  Target Platform: ESP32                                                                                                                               *
 *                                                                                                                                                       *
 *  Version: 1.0                                                                                                                                         *
 *  Hardware setup: See github                                                                                                                           *
 *  Spectrum analyses done with analog chips MSGEQ7                                                                                                      *
 *                                                                                                                                                       *
 *  Mark Donners                                                                                                                                         *
 *  The Electronic Engineer                                                                                                                              *
 *  Website:   www.theelectronicengineer.nl                                                                                                              *
 *  facebook:  https://www.facebook.com/TheelectronicEngineer                                                                                            *
 *  youtube:   https://www.youtube.com/channel/UCm5wy-2RoXGjG2F9wpDFF3w                                                                                  *
 *  github:    https://github.com/donnersm                                                                                                               *
 *                                                                                                                                                       *
 ********************************************************************************************************************************************************/

#pragma once

// only need to match panel settings above
 #define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
 #define PANE_HEIGHT PANEL_HEIGHT
 const uint8_t kMatrixWidth =   PANEL_WIDTH * PANELS_NUMBER;                // Matrix width --> number of columns in your led matrix
 const uint8_t kMatrixHeight =  PANEL_HEIGHT;                 // Matrix height --> number of leds per column

#define BAR_WIDTH  (kMatrixWidth /(numBands ))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP            (kMatrixHeight - 0)         // Don't allow the bars to go offscreen
#define NeededWidth (BAR_WIDTH * numBands)           // we need this to see if all bands fit or that we have left over space
#define NUM_LEDS   (kMatrixWidth * kMatrixHeight)   // Total number of LEDs
CRGB leds[NUM_LEDS];

/*
// Tools
 CRGB HsvToRgb(uint8_t h, uint8_t s, uint8_t v){
   CHSV hsv = CHSV(h, s, v);
    CRGB rgb;
    hsv2rgb_spectrum(hsv, rgb);
    return rgb;
 }

*/

/*
 * First all the bar patterns
 */
/******************************************************************
 * Pattern name: ColorBars Bars
 *******************************************************************/
void ColorBars(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        dma_display->drawPixelRGB888(x, y, (band + 1) * 40, (band + 1) * 30, 255 - ((band + 1) * 70)); // middle
        //   dma_display->drawPixelRGB888(x,y,band*40,band*30,150-(band*10));      //middle
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Red Bars
 *******************************************************************/
void RedBars(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        dma_display->drawPixelRGB888(x, y, 250, 0, 0); // middle
        //   dma_display->drawPixelRGB888(x,y,band*40,band*30,150-(band*10));      //middle
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Twins
 *******************************************************************/
void Twins(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {
        if ((band & 1) == 1)
          dma_display->drawPixelRGB888(x, y, 250, 0, 0);
        else
          dma_display->drawPixelRGB888(x, y, 250, 250, 0); // middle
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Twins 2
 *******************************************************************/
void Twins2(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {
        if ((band & 1) == 1)
          dma_display->drawPixelRGB888(x, y, 250, 0, 250);
        else
          dma_display->drawPixelRGB888(x, y, 0, 250, 250); // middle
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Tri Color Bars
 *******************************************************************/
void TriBars(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        if (y < (PANEL_HEIGHT / 4))
          dma_display->drawPixelRGB888(x, y, TriBar_RGB_Top); // Top
        else if (y > (PANEL_HEIGHT / 2))
          dma_display->drawPixelRGB888(x, y, TriBar_RGB_Bottom); // bottom
        else
          dma_display->drawPixelRGB888(x, y, TriBar_RGB_Middle); // middle
        // else  dma_display->drawPixelRGB888(x,y,TriBar_Color_Middle_RGB);      //middle
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Boxed bars
 *******************************************************************/
void BoxedBars(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        if (y == (TOP - barHeight))
          dma_display->drawPixelRGB888(x, y, 250, 0, 0);
        else if (x == xStart)
          dma_display->drawPixelRGB888(x, y, 250, 0, 0); // Border left side of the bars
        else if (x == xStart + BAR_WIDTH - 1)
          dma_display->drawPixelRGB888(x, y, 250, 0, 0); // Border right side of the bars
        else
          dma_display->drawPixelRGB888(x, y, 0, 0, 250);
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Boxed bars 2
 *******************************************************************/
void BoxedBars2(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        if (y == (TOP - barHeight))
          dma_display->drawPixelRGB888(x, y, 250, 250, 250);
        else if (x == xStart)
          dma_display->drawPixelRGB888(x, y, 250, 250, 250); // Border left side of the bars
        else if (x == xStart + BAR_WIDTH - 1)
          dma_display->drawPixelRGB888(x, y, 250, 250, 250); // Border right side of the bars
        else
          dma_display->drawPixelRGB888(x, y, 0, 0, 250);
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Boxed bars 3
 *******************************************************************/
void BoxedBars3(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;

  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        if (y == (TOP - barHeight))
          dma_display->drawPixelRGB888(x, y, 0, 255, 0);
        else if (x == xStart)
          dma_display->drawPixelRGB888(x, y, 0, 255, 0); // Border left side of the bars
        else if (x == xStart + BAR_WIDTH - 1)
          dma_display->drawPixelRGB888(x, y, 0, 255, 0); // Border right side of the bars
        else
          dma_display->drawPixelRGB888(x, y, 200, 200, 0);
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/******************************************************************
 * Pattern name: Center Bars
 *******************************************************************/

void centerBars(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int center = TOP / 2;

  if (barHeight > (kMatrixHeight - 6))
    barHeight = kMatrixHeight - 6;
  // Serial.printf( "barheight: %d \n",barHeight);
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = 0; y <= (barHeight / 2); y++)
    {
      // if(y >= TOP - barHeight){

      if (y == (barHeight / 2))
      {
        dma_display->drawPixelRGB888(x, center + y, Center_RGB_Edge);     // edge
        dma_display->drawPixelRGB888(x, center - y - 1, Center_RGB_Edge); // edge
      }
      else
      {
        dma_display->drawPixelRGB888(x, center + y, Center_RGB_Middle);     // middle
        dma_display->drawPixelRGB888(x, center - y - 1, Center_RGB_Middle); // middle
      }
    }
    for (int y = barHeight / 2; y < TOP; y++)
    {
      dma_display->drawPixelRGB888(x, center + y + 1, 0, 0, 0); // make unused pixel bottom black
      if ((center - y - 2) > 1)
        dma_display->drawPixelRGB888(x, center - y - 2, 0, 0, 0); // make unused pixel top black except those of the VU meter
    }
  }
}

/******************************************************************
 * Pattern name: Center Bars 2
 *******************************************************************/

void centerBars2(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int center = TOP / 2;

  if (barHeight > (kMatrixHeight - 6))
    barHeight = kMatrixHeight - 6;
  // Serial.printf( "barheight: %d \n",barHeight);
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = 0; y <= (barHeight / 2); y++)
    {
      // if(y >= TOP - barHeight){

      if (y == (barHeight / 2))
      {
        dma_display->drawPixelRGB888(x, center + y, Center_RGB_Edge2);     // edge
        dma_display->drawPixelRGB888(x, center - y - 1, Center_RGB_Edge2); // edge
      }
      else
      {
        dma_display->drawPixelRGB888(x, center + y, Center_RGB_Middle2);     // middle
        dma_display->drawPixelRGB888(x, center - y - 1, Center_RGB_Middle2); // middle
      }
    }
    for (int y = barHeight / 2; y < TOP; y++)
    {
      dma_display->drawPixelRGB888(x, center + y + 1, 0, 0, 0); // make unused pixel bottom black
      if ((center - y - 2) > 1)
        dma_display->drawPixelRGB888(x, center - y - 2, 0, 0, 0); // make unused pixel top black except those of the VU meter
    }
  }
}

/******************************************************************
 * Pattern name: Black Bars
 *******************************************************************/
void BlackBars(int band, int barHeight)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    for (int y = TOP; y >= 2; y--)
    {
      if (y >= TOP - barHeight)
      {

        dma_display->drawPixelRGB888(x, y, 0, 0, 0); // middle
      }
      else
      {
        // leds[i].fadeToBlackBy( 64 );

        dma_display->drawPixelRGB888(x, y, 0, 0, 0);
      }
    }
  }
}

/*
 * All the Peak Patterns
 */

/******************************************************************
 * Peak name: Red Peaks
 *******************************************************************/

void RedPeak(int band)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    // matrix->drawPixel(x, peakHeight, CHSV(0,255,0));
    dma_display->drawPixelRGB888(x, peakHeight, 255, 0, 0);
  }
}

/******************************************************************
 * Peak name: White Peaks
 *******************************************************************/

void WhitePeak(int band)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    // matrix->drawPixel(x, peakHeight, CHSV(0,255,0));
    dma_display->drawPixelRGB888(x, peakHeight, 255, 255, 255);
  }
}

/******************************************************************
 * Peak name: Blue peaks
 *******************************************************************/

void BluePeak(int band)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    // matrix->drawPixel(x, peakHeight, CHSV(0,255,0));
    dma_display->drawPixelRGB888(x, peakHeight, 0, 0, 255);
  }
}
/******************************************************************
 * Peak name: Double Peak
 *******************************************************************/

void DoublePeak(int band)
{

  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {
    // matrix->drawPixel(x, peakHeight, CHSV(0,255,0));
    dma_display->drawPixelRGB888(x, peakHeight, 0, 0, 255);
    dma_display->drawPixelRGB888(x, peakHeight + 1, 0, 0, 255);
  }
}
/******************************************************************
 * Pattern name: Tri Color Peak
 *******************************************************************/

void TriPeak(int band)
{
  int xStart = BAR_WIDTH * band;
  if (NeededWidth < kMatrixWidth)
    xStart += (kMatrixWidth - NeededWidth) / 2;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++)
  {

    if (peakHeight < (PANEL_HEIGHT / 4))
      dma_display->drawPixelRGB888(x, peakHeight, TriBar_RGB_Top); // Top red
    else if (peakHeight > (PANEL_HEIGHT / 2))
      dma_display->drawPixelRGB888(x, peakHeight, TriBar_RGB_Bottom); // green
    else
      dma_display->drawPixelRGB888(x, peakHeight, TriBar_RGB_Middle); // yellow
  }
}
