#define AUDIO_VISUALIZER_THEME 100
#define PLEX_COVER_ART_THEME 200
#define GIF_ART_THEME 210

// ******************************************* BEGIN MATRIX DISPLAY *******************************************
#pragma region MATRIX_DISPLAY

#include <FS.h>
#include <SPIFFS.h>
#include <JPEGDEC.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "picopixel.h"

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANELS_NUMBER 1

MatrixPanel_I2S_DMA *dma_display = nullptr;
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
JPEGDEC jpeg;

const char *ALBUM_ART = "/album.jpg";

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
void clearImage()
{
  dma_display->fillScreen(myBLACK);
}

void printCenter(const char *buf, int y)
{
  // Clear the screen
  displayRect(0, y - 5, PANEL_WIDTH, 8, 0);

  int16_t x1, y1;
  uint16_t w, h;
  dma_display->setFont(&Picopixel);
  dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);
  dma_display->setCursor(32 - (w / 2), y);
  dma_display->setTextColor(0xffff);
  dma_display->print(buf);
}

enum ScrollState
{
  SCROLL_IDLE,
  SCROLL_INIT,
  SCROLL_RUNNING
};

char previousScrollingText[100] = "";

void scrollingPrintCenter(const char *buf, int y)
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
    dma_display->setFont(&Picopixel);
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
        dma_display->setTextColor(0xffff);
        dma_display->print(buf);
        state = SCROLL_IDLE; // Reset state to IDLE if no scrolling is needed
        strcpy(previousScrollingText, buf);
      }
    }
    break;

  case SCROLL_RUNNING:
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
      dma_display->setTextColor(0xffff);
      dma_display->print(buf);

      // Update last scroll time
      lastScrollTime = currentTime;

      // Move the text to the left
      xPos--;

      if (xPos == -textWidth)
      {
        // If the text has scrolled completely off the screen, reset xPos to start over
        xPos = PANEL_WIDTH;
        state = SCROLL_IDLE;
      }
    }
    break;
  }
}

int drawImagefromFile(const char *imageFileUri)
{
  unsigned long lTime = millis();
  lTime = millis();
  jpeg.open((const char *)imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
  // int decodeStatus = jpeg.decode(0, 0, 0);
  int decodeStatus = jpeg.decode(8, 9, 0);
  jpeg.close();
  Serial.print("Time taken to decode and display Image (ms): ");
  Serial.println(millis() - lTime);
  return decodeStatus;
}

#pragma endregion
// ******************************************* END MATRIX DISPLAY *********************************************

// ******************************************* BEGIN WIFI *****************************************************
#pragma region WIFI

#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

#define WM_PLEX_SERVER_IP_LABEL "plexServerIp"
#define WM_PLEX_SERVER_PORT_LABEL "plexServerPort"
#define WM_PLEX_SERVER_TOKEN_LABEL "plexServerToken"
#define PMD_CONFIG_JSON "/pmd_config.json"

WiFiManager wifiManager;

std::vector<const char *> _menu = {"wifi", "exit"};
char plexServerIp[60];
char plexServerPort[60];
char plexServerToken[80];
char defaultPlexPort[60] = "32400";
bool shouldSaveConfig = false;

void restartDevice()
{
  clearImage();
  printCenter("REBOOTING..", 30);
  delay(3000);
  ESP.restart();
}

void resetWifi()
{
  printCenter("RESETTING WiFi..", 30);
  wifiManager.resetSettings();
}

void fetchPlexConfigFile()
{
  if (SPIFFS.exists(PMD_CONFIG_JSON))
  {
    // file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open(PMD_CONFIG_JSON, "r");
    if (configFile)
    {
      Serial.println("opened config file");
      StaticJsonDocument<512> json;
      DeserializationError error = deserializeJson(json, configFile);
      serializeJsonPretty(json, Serial);
      if (!error)
      {
        Serial.println("\nparsed json");

        if (json.containsKey(WM_PLEX_SERVER_IP_LABEL) && json.containsKey(WM_PLEX_SERVER_TOKEN_LABEL))
        {
          const char *tempPlexServerIp = json[WM_PLEX_SERVER_IP_LABEL];
          const char *tempPlexServerPort = json[WM_PLEX_SERVER_PORT_LABEL];
          const char *tempPlexServerToken = json[WM_PLEX_SERVER_TOKEN_LABEL];

          // Ensure null-termination and copy to plexServerIp and plexServerToken
          strlcpy(plexServerIp, tempPlexServerIp, sizeof(plexServerIp));
          strlcpy(plexServerPort, tempPlexServerPort ? tempPlexServerPort : defaultPlexPort, sizeof(plexServerPort)); // Assign default value if port is not present
          strlcpy(plexServerToken, tempPlexServerToken, sizeof(plexServerToken));
          Serial.println("Plex Server IP: " + String(plexServerIp));
          Serial.println("Plex Server Port: " + String(plexServerPort));
          Serial.println("Plex Server Token: " + String(plexServerToken));
        }
        else
        {
          Serial.println("Config missing Plex server IP or Auth token");
        }
      }
      else
      {
        Serial.println("failed to load json config");
      }
      configFile.close();
    }
    else
    {
      Serial.println("Failed to open config file");
    }
  }
  else
  {
    Serial.println("Config file does not exist");
  }
}

// Save wifi config to SPIFF
void saveConfig(const char *plexServerIp, const char *plexServerPort, const char *plexServerToken)
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json[WM_PLEX_SERVER_IP_LABEL] = plexServerIp;       // Assigning C-style strings directly
  json[WM_PLEX_SERVER_PORT_LABEL] = plexServerPort;   // Assigning C-style strings directly
  json[WM_PLEX_SERVER_TOKEN_LABEL] = plexServerToken; // Assigning C-style strings directly

  File configFile = SPIFFS.open(PMD_CONFIG_JSON, "w");
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
    return; // Exit the function early if file opening fails
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
  shouldSaveConfig = false;
}

// callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void wifiConnect()
{
  bool resp;
  // resetWifi();
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter plexServerIpParam(WM_PLEX_SERVER_IP_LABEL, "Plex Server IP Address", plexServerIp, 40);
  WiFiManagerParameter plexServerPortParam(WM_PLEX_SERVER_PORT_LABEL, "Plex Server Port Number (defaults to 32400)", plexServerPort, 40);
  WiFiManagerParameter plexServerTokenParam(WM_PLEX_SERVER_TOKEN_LABEL, "Plex Server Auth Token", plexServerToken, 60);

  wifiManager.setTitle("PMD Wifi Setup");
  wifiManager.setMenu(_menu);
  wifiManager.addParameter(&plexServerIpParam);
  wifiManager.addParameter(&plexServerPortParam);
  wifiManager.addParameter(&plexServerTokenParam);

  resp = wifiManager.autoConnect("PMD Wifi Setup");

  if (!resp)
  {
    Serial.println("Failed to connect");
    printCenter("FAILED to Connect", 30);
    restartDevice();
  }
  else
  {
    Serial.println("connected!");
    // save the custom parameters
    if (shouldSaveConfig)
    {
      strncpy(plexServerIp, plexServerIpParam.getValue(), 40);
      strncpy(plexServerPort, plexServerPortParam.getValue(), 40);
      strncpy(plexServerToken, plexServerTokenParam.getValue(), 60);

      saveConfig(plexServerIp, plexServerPort, plexServerToken);
      restartDevice();
    }
  }
}

boolean isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

#pragma endregion
// ******************************************* END WIFI *******************************************************

// ******************************************* BEGIN PREFERENCES **********************************************
#pragma region PREFERENCES

#include <Preferences.h>

Preferences preferences;
const char *const PREF_SELECTED_THEME = "selectedTheme";
uint8_t selectedTheme;
uint8_t currentlyRunningTheme;

void savePreferences()
{
  preferences.putUInt(PREF_SELECTED_THEME, selectedTheme);
}

void loadPreferences()
{
  selectedTheme = preferences.getUInt(PREF_SELECTED_THEME, 200);
}

#pragma endregion
// ******************************************* END PREFERENCES ************************************************

// ******************************************* BEGIN WEB SERVER ***********************************************
#pragma region WEBSERVER

#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "PMDWebPage.h"

WiFiServer server(80);

String httpBuffer;
bool force_restart;
const char *HEADER_TEMPLATE_D = "X-%s: %d\r\n";
const char *HEADER_TEMPLATE_S = "X-%s: %s\r\n";

void processRequest(WiFiClient client, String method, String path, String key, String value)
{
  if (method == "GET" && path == "/")
  {
    client.println("HTTP/1.0 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println(SETTINGS_PAGE);
  }
  else if (method == "POST" && path == "/restart")
  {
    client.println("HTTP/1.0 204 No Content");
    force_restart = true;
  }
  else if (method == "POST" && path == "/set")
  {
    loadPreferences();

    if (key == PREF_SELECTED_THEME)
    {
      selectedTheme = value.toInt();
      if (selectedTheme != currentlyRunningTheme)
      {
        client.println("HTTP/1.0 204 No Content");
        savePreferences();
        currentlyRunningTheme = selectedTheme;

        force_restart = true;
        return;
      }
    }
    client.println("HTTP/1.0 204 No Content");
  }
}

void handleHttpRequest()
{
  if (force_restart)
  {
    restartDevice();
  }

  WiFiClient client = server.available();
  if (client)
  {
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        httpBuffer.concat(c);

        if (c == '\n')
        {
          uint8_t method_pos = httpBuffer.indexOf(' ');
          uint8_t path_pos = httpBuffer.indexOf(' ', method_pos + 1);

          String method = httpBuffer.substring(0, method_pos);
          String path = httpBuffer.substring(method_pos + 1, path_pos);
          String key = "";
          String value = "";

          if (path.indexOf('?') > 0)
          {
            key = path.substring(path.indexOf('?') + 1, path.indexOf('='));
            value = path.substring(path.indexOf('=') + 1);
            path = path.substring(0, path.indexOf('?'));
          }

          processRequest(client, method, path, key, value);
          httpBuffer = "";
          break;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

#pragma endregion
// ******************************************* END WEB SERVER *************************************************

// ******************************************* BEGIN GIF PLAYER ***********************************************
#pragma region GIF_PLAYER

#include <AnimatedGIF.h>

const char *GIF_ART = "/gifArt.gif";
AnimatedGIF gif;
// fs::File f;
int x_offset, y_offset;
String lastGifFilename = "";
bool shouldShowGifArt = false;

// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > MATRIX_WIDTH)
    iWidth = MATRIX_WIDTH;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < pDraw->iWidth)
    {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      }           // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++)
        {
          dma_display->drawPixel(x + xOffset, y, usTemp[xOffset]); // 565 Color Format
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount)
      {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else // does not have transparency
  {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++)
    {
      dma_display->drawPixel(x, y, usPalette[*s++]); // color 565
    }
  }
}

void *GIFOpenFile(const char *fname, int32_t *pSize)
{
  myfile = SPIFFS.open(fname);
  if (myfile)
  {
    *pSize = myfile.size();
    return (void *)&myfile;
  }
  return NULL;
}

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
}

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
}

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  // Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
}

void deleteGifArt()
{
  if (SPIFFS.exists(GIF_ART) == true)
  {
    Serial.println("Removing existing image");
    SPIFFS.remove(GIF_ART);
  }
}

enum GIFState
{
  STATE_IDLE,
  STATE_OPEN_GIF,
  STATE_PLAY_GIF,
  STATE_CLOSE_GIF,
};

GIFState gifState = STATE_IDLE;

void showGIF()
{

  switch (gifState)
  {
  case STATE_IDLE:
    if (SPIFFS.exists(GIF_ART))
    {
      gifState = STATE_OPEN_GIF;
    }
    break;

  case STATE_OPEN_GIF:
    if (gif.open(GIF_ART, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
    {
      x_offset = (MATRIX_WIDTH - gif.getCanvasWidth()) / 2;
      if (x_offset < 0)
        x_offset = 0;
      y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight()) / 2;
      if (y_offset < 0)
        y_offset = 0;
      // Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      gifState = STATE_PLAY_GIF;
    }
    else
    {
      Serial.println("******** Failed to show GIF");
      clearImage();
      printCenter("FAILED TO", 20);
      printCenter("SHOW GIF", 30);
      gifState = STATE_IDLE;
    }
    break;

  case STATE_PLAY_GIF:
    // Play a single frame
    if (!gif.playFrame(true, NULL))
    {
      // If the playback reaches the end, close the GIF file
      gif.close();
      gifState = STATE_IDLE;
    }
    break;

  case STATE_CLOSE_GIF:
    // Close the GIF file
    gif.close();
    gifState = STATE_IDLE;
    break;
  }
}

void downloadGifArt(String filename)
{
  // Initialize HTTP client
  HTTPClient http;

  // Specify the base URL of the GIF files on raw.githubusercontent.com
  String baseURL = "https://raw.githubusercontent.com/robegamesios/PlexMatrixDisplay/main/shared/gifs/";

  // Construct the full URL by appending the filename to the base URL
  String gifURL = baseURL + filename + ".gif";
  // Serial.print("*************GIF url = ");
  // Serial.println(gifURL);

  // Start the HTTP request to download the GIF file
  if (http.begin(gifURL))
  {
    int httpCode = http.GET(); // Send GET request

    // Check if the request was successful
    if (httpCode == HTTP_CODE_OK)
    {

      deleteGifArt();

      // Open a file to save the downloaded GIF
      File file = SPIFFS.open(GIF_ART, FILE_WRITE);
      if (file)
      {
        // Write the response body to the file
        http.writeToStream(&file);
        file.close();

        // Check if the file exists
        if (SPIFFS.exists(GIF_ART))
        {
          Serial.println("Successfully downloaded and saved GIF file to SPIFFS");
        }
        else
        {
          Serial.println("Failed to save image to SPIFFS");
          clearImage();
          printCenter("FAILED TO", 20);
          printCenter("SAVE IMAGE", 30);
        }
      }
      else
      {
        Serial.println("Failed to create file");
        clearImage();
        printCenter("FAILED TO", 20);
        printCenter("CREATE FILE", 30);
      }
    }
    else
    {
      Serial.print("Failed to download image. HTTP error code: ");
      Serial.println(httpCode);
      clearImage();
      printCenter("HTTP ERROR", 20);
      printCenter(String(httpCode).c_str(), 30);
    }
    // End the HTTP client
    http.end();
  }
  else
  {
    Serial.println("Failed to connect to image URL");
    clearImage();
    printCenter("FAILED TO", 20);
    printCenter("CONNECT TO", 30);
    printCenter("IMAGE URL", 40);
  }
}

#pragma endregion
// ******************************************* END GIF PLAYER *************************************************

// ******************************************* BEGIN MAIN *****************************************************
#pragma region MAIN

#include <HTTPUpdate.h>

int failedConnectionAttempts = 0;
const int MAX_FAILED_ATTEMPTS = 5;

void update_progress(int cur, int total)
{
  // Clear screen
  displayRect(0, 30, PANEL_WIDTH, 10, 0);

  // Display progress
  float progress = cur * 100.0 / total;
  char buffer[30];
  sprintf(buffer, "%.0f%%", progress);
  printCenter(buffer, 40);
}

void setup()
{
  Serial.begin(115200);

  displaySetup();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount file system");
    printCenter("SPIFFS FAILED", 20);
    printCenter(" Restarting", 30);
    restartDevice();
    return;
  }

  preferences.begin("PMD", false);
  selectedTheme = preferences.getUInt(PREF_SELECTED_THEME, PLEX_COVER_ART_THEME);
  currentlyRunningTheme = selectedTheme;

  printCenter("GIF Art", 30);

  wifiConnect();

  while (!isConnected())
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    printCenter("Connecting to WiFi.", 40);

    failedConnectionAttempts++;
    if (failedConnectionAttempts >= MAX_FAILED_ATTEMPTS)
    {
      Serial.println("Failed to connect to WiFi after multiple attempts. Restarting device...");
      printCenter("WiFi FAILED", 20);
      printCenter(" Restarting", 30);
      restartDevice();
    }
  }
  Serial.println("Connected to WiFi");
  printCenter("Connected to WiFi.", 40);
  delay(3000);

  clearImage();
  Serial.println("\r\nInitialisation done.");

  // if (selectedTheme == AUDIO_VISUALIZER_THEME || selectedTheme == PLEX_COVER_ART_THEME)
  // {
  //   Serial.print("will update firmware to Plex cover art or audio visualizer");
  //   // update firmware to canvas
  //   WiFiClientSecure client;
  //   client.setInsecure();

  //   // Reading data over SSL may be slow, use an adequate timeout
  //   client.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

  //   // Display IP address
  //   char ipAddressLabel[16];
  //   sprintf(ipAddressLabel, "IP Address:");
  //   printCenter(ipAddressLabel, 5);

  //   IPAddress ipAddress = WiFi.localIP();
  //   char ipAddressString[16];
  //   sprintf(ipAddressString, "%s", ipAddress.toString().c_str());
  //   printCenter(ipAddressString, 15);

  //   // Display Loading text
  //   const char *loadingText = "Loading..";
  //   printCenter(loadingText, 30);

  //   httpUpdate.onProgress(update_progress);

  //   t_httpUpdate_return ret = httpUpdate.update(client, "https://raw.githubusercontent.com/robegamesios/clock-club/main/binFiles/canvasPlusFirmware.bin");

  //   switch (ret)
  //   {
  //   case HTTP_UPDATE_FAILED:
  //     Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
  //     break;

  //   case HTTP_UPDATE_NO_UPDATES:
  //     Serial.println("HTTP_UPDATE_NO_UPDATES");
  //     break;

  //   case HTTP_UPDATE_OK:
  //     Serial.println("HTTP_UPDATE_OK");
  //     break;
  //   }
  //   return;
  // }

  server.begin();

  downloadGifArt("bugcat-crowd");
}

void loop()
{
  if (isConnected())
  {
    handleHttpRequest();
    showGIF();
  }
}

#pragma endregion
// ******************************************* END MAIN *******************************************************
