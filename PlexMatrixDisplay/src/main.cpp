#define WEATHER_STATION_THEME 90
#define AUDIO_VISUALIZER_THEME 100
#define PLEX_ALBUM_ART_THEME 200
#define SPOTIFY_ALBUM_ART_THEME 201
#define GIF_ART_THEME 210

// #define AV // Uncomment to use Audio Visualizer
// #define DEBUG // UNComment to see debug prints

// ******************************************* BEGIN MATRIX DISPLAY *******************************************
#pragma region MATRIX_DISPLAY

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

// ******************************************* BEGIN UTILS ****************************************************
#pragma region UTILS

#include <cstring>

std::string capitalizeString(const char *str)
{
  std::string capitalizedStr;
  if (str != nullptr && strlen(str) > 0)
  {
    capitalizedStr += std::toupper(str[0]);
    capitalizedStr += &str[1]; // Append the rest of the string
  }
  return capitalizedStr;
}

std::string toUpperCase(const char *str)
{
  std::string upperStr;
  if (str != nullptr)
  {
    size_t length = strlen(str);
    for (size_t i = 0; i < length; ++i)
    {
      upperStr += std::toupper(str[i]);
    }
  }
  return upperStr;
}

const char *strrstr(const char *haystack, const char *needle)
{
  if (!*needle)
    return haystack;
  const char *result = nullptr;
  const char *p;
  const char *q;
  for (; *haystack; ++haystack)
  {
    if (*haystack == needle[0])
    {
      p = haystack;
      q = needle;
      while (*p && *q && *p == *q)
      {
        ++p;
        ++q;
      }
      if (!*q)
      {
        result = haystack;
      }
    }
  }
  return result;
}

const char *extractStringValue(const char *jsonString, const char *fieldName, bool findLast = false, const char *defaultValue = nullptr)
{
  const char *fieldStart = nullptr;

  // Find the start index of the field
  if (findLast)
  {
    fieldStart = strrstr(jsonString, fieldName);
  }
  else
  {
    fieldStart = strstr(jsonString, fieldName);
  }

  if (fieldStart == nullptr)
  {
    printf("Error: Unable to find field '%s'.\n", fieldName);
    return defaultValue;
  }

  // Move to the actual value
  fieldStart += strlen(fieldName) + 2; // Account for the field name and 1 colon and 1 quote: "\"fieldName\":"

  // Find the end index of the field value
  const char *fieldValueEnd = strchr(fieldStart, '"');
  if (fieldValueEnd == nullptr)
  {
    printf("Error: Invalid JSON format for field '%s'.\n", fieldName);
    return defaultValue;
  }

  // Calculate the length of the field value
  int fieldValueLength = fieldValueEnd - fieldStart;

  // Allocate memory for the field value string
  char *fieldValue = (char *)malloc(fieldValueLength + 1);
  if (fieldValue == nullptr)
  {
    // Handle memory allocation failure
    printf("Error: Failed to allocate memory.\n");
    return defaultValue;
  }

  // Copy the value
  strncpy(fieldValue, fieldStart, fieldValueLength);
  fieldValue[fieldValueLength] = '\0';

  return fieldValue;
}

float extractFloatValue(const char *jsonString, const char *fieldName, float defaultValue = 0.0f)
{
  // Find the start index of the field
  const char *fieldStart = strstr(jsonString, fieldName);
  if (fieldStart == nullptr)
  {
    printf("Error: Unable to find field '%s'.\n", fieldName);
    return defaultValue;
  }

  // Move to the actual value
  fieldStart += strlen(fieldName) + 1; // Account for the field name and colon: "\"fieldName\":"

  // Find the end index of the field value
  float extractedValue;
  if (sscanf(fieldStart, "%f", &extractedValue) != 1)
  {
    printf("Error: Unable to extract float value for field '%s'.\n", fieldName);
    return defaultValue;
  }

  return extractedValue;
}

void floatToChar(float floatValue, char *charArray, int bufferSize)
{
  snprintf(charArray, bufferSize, "%.0f", floatValue);
}

#pragma endregion
// ******************************************* END UTILS ******************************************************

// ******************************************* BEGIN GLOBAL VARS AND COMMON FUNC ******************************
#pragma region GLOBAL_VARS_AND_UTILS

String scrollingText = "";
String lowerScrollingText = "";
String lastAlbumArtURL = ""; // Variable to store the last downloaded album art URL

bool isScreenSaverMode = false;

String decodeHtmlEntities(String text)
{
  String decodedText = text;
  decodedText.replace("&quot;", "\"");
  decodedText.replace("&amp;", "&");
  decodedText.replace("&apos;", "'");
  decodedText.replace("&lt;", "<");
  decodedText.replace("&gt;", ">");
  decodedText.replace("&#8217;", "'");
  decodedText.replace("&#8216;", "'");
  decodedText.replace("&#39;", "'");
  // Add more replacements as needed

  return decodedText;
}

void resetAlbumArtVariables()
{
  scrollingText = "";
  lowerScrollingText = "";
  lastAlbumArtURL = "";
}

void deleteAlbumArt()
{
  if (SPIFFS.exists(ALBUM_ART) == true)
  {
    Serial.println("Removing existing image");
    SPIFFS.remove(ALBUM_ART);
  }
}

void displayNoTrackPlaying()
{
#ifdef DEBUG
  Serial.println("No track is currently playing.");
#endif

  resetAlbumArtVariables();
  clearScreen();
  printCenter("NO TRACK IS", 20);
  printCenter("CURRENTLY", 30);
  printCenter("PLAYING", 40);
}

void displayCheckWeatherCredentials()
{
#ifdef DEBUG
  Serial.println("Check Weather credentials");
#endif

  resetAlbumArtVariables();
  clearScreen();
  printCenter("CHECK WEATHER", 30);
  printCenter("CREDENTIALS", 40);
}

void displayCheckSpotifyCredentials()
{
#ifdef DEBUG
  Serial.println("Check Spotify credentials");
#endif

  resetAlbumArtVariables();
  clearScreen();
  printCenter("CHECK SPOTIFY", 30);
  printCenter("CREDENTIALS", 40);
}

void displayMusicPaused()
{
  String musicPaused = "Music Paused";
  if (lowerScrollingText != musicPaused)
  {
    lowerScrollingText = musicPaused;
  }
}

// Function to escape double quote characters, remove newlines, and extra whitespace
String escapeSpecialCharacters(const String &jsonString)
{
  String escapedString;
  bool insideQuotes = false;
  for (char c : jsonString)
  {
    if (c == '"')
    {
      insideQuotes = !insideQuotes;
      escapedString += c;
    }
    else if (c == '\n' || c == '\r')
    {
      // Skip newlines
      continue;
    }
    else if (c == ' ' && !insideQuotes)
    {
      // Skip extra whitespace outside quotes
      continue;
    }
    else
    {
      escapedString += c;
    }
  }
  return escapedString;
}

String urlEncode(const char *msg)
{
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";

  while (*msg != '\0')
  {
    if (('a' <= *msg && *msg <= 'z') ||
        ('A' <= *msg && *msg <= 'Z') ||
        ('0' <= *msg && *msg <= '9') ||
        *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~')
    {
      encodedMsg += *msg;
    }
    else
    {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }

  return encodedMsg;
}

#pragma endregion
// ******************************************* END GLOBAL VARS AND COMMON FUNC ********************************

// ******************************************* BEGIN WIFI *****************************************************
#pragma region WIFI

#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

WiFiManager wifiManager;

std::vector<const char *> _menu = {"wifi", "exit"};

void restartDevice()
{
  clearScreen();
  printCenter("RESTARTING..", 30);
  delay(3000);
  ESP.restart();
}

void resetWifi()
{
  printCenter("RESETTING WiFi..", 30);
  wifiManager.resetSettings();
}

void wifiConnect()
{
  bool resp;

  wifiManager.setTitle("TUNEFRAME Wifi Setup");
  wifiManager.setMenu(_menu);

  resp = wifiManager.autoConnect("TUNEFRAME Wifi Setup");

  if (!resp)
  {
    Serial.println("Failed to connect");
    printCenter("FAILED to Connect", 30);
    restartDevice();
  }
  else
  {
    Serial.println("connected!");
  }
}

boolean isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

#pragma endregion
// ******************************************* END WIFI *******************************************************

// ******************************************* BEGIN NETWORKING ***********************************************
#pragma region NETWORKING

void httpGet(const String &url, const String &authorizationHeaderKey, const String &authorizationHeaderValue, std::function<void(int, const String &)> callback)
{
  HTTPClient http;
  http.begin(url);
  http.addHeader(authorizationHeaderKey, authorizationHeaderValue);

  int httpCode = http.GET();
  String response;

  if (httpCode == HTTP_CODE_OK)
  {
    response = http.getString();
  }

  callback(httpCode, response);

  http.end();
}

#pragma endregion
// ******************************************* END NETWORKING *************************************************

// ******************************************* BEGIN PREFERENCES **********************************************
#pragma region PREFERENCES

#include <Preferences.h>

Preferences preferences;
const char *PREF_SELECTED_THEME = "selectedTheme";
const char *PREF_AV_PATTERN = "avPattern";
const char *PREF_GIF_ART_NAME = "gifArtName";
const char *PREF_WEATHER_STATION_CREDENTIALS = "weatherStationCredentials";
const char *PREF_PLEX_CREDENTIALS = "plexCredentials";
const char *PREF_SPOTIFY_CREDENTIALS = "spotifyCredentials";

uint8_t selectedTheme;
uint8_t currentlyRunningTheme;

void savePreferences()
{
  preferences.putUInt(PREF_SELECTED_THEME, selectedTheme);
}

void loadPreferences()
{
  selectedTheme = preferences.getUInt(PREF_SELECTED_THEME, AUDIO_VISUALIZER_THEME);
}

#pragma endregion
// ******************************************* END PREFERENCES ************************************************

// ******************************************* BEGIN CLOCK ****************************************************
#pragma region CLOCK

#include "time.h"

String getLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
#ifdef DEBUG
    Serial.println("Failed to obtain time");
#endif

    return "";
  }
  char buffer[20];                                                // Create a buffer to hold the formatted time
  strftime(buffer, sizeof(buffer), "%I:%M %P  %b %d", &timeinfo); // Format time into buffer
  String localTime = String(buffer);                              // Convert buffer to String
  return localTime;
}

void getDateAndTime()
{
  String localTime = getLocalTime();
  scrollingText = localTime;
}

#pragma endregion
// ******************************************* END CLOCK ******************************************************

// ******************************************* BEGIN WEATHER **************************************************
#pragma region WEATHER

#include "weatherIcons.h"

#define WM_WEATHER_CITY_NAME_LABEL "weatherCityName"
#define WM_WEATHER_COUNTRY_CODE_LABEL "weatherCountryCode"
#define WM_WEATHER_API_KEY_LABEL "weatherApiKey"
#define WM_WEATHER_UNIT_LABEL "weatherUnit"
#define WEATHER_CONFIG_JSON "/weather_config.json"

char weatherCityName[32];
char weatherCountryCode[6];
char weatherApikey[64];
char weatherUnit[2] = "0";

bool weatherConfigExist = false;

const int daylightOffset_sec = 3600; // daylight savings time is ON, 0 for OFF

void fetchWeatherConfigFile()
{
  if (SPIFFS.exists(WEATHER_CONFIG_JSON))
  {
    // file exists, reading and loading
    File configFile = SPIFFS.open(WEATHER_CONFIG_JSON, "r");
    if (configFile)
    {
      StaticJsonDocument<512> json;
      DeserializationError error = deserializeJson(json, configFile);
      serializeJsonPretty(json, Serial);
      if (!error)
      {
        if (json.containsKey(WM_WEATHER_CITY_NAME_LABEL) && json.containsKey(WM_WEATHER_COUNTRY_CODE_LABEL) && json.containsKey(WM_WEATHER_API_KEY_LABEL) && json.containsKey(WM_WEATHER_UNIT_LABEL))
        {
          const char *tempCityName = json[WM_WEATHER_CITY_NAME_LABEL];
          const char *tempCountryCode = json[WM_WEATHER_COUNTRY_CODE_LABEL];
          const char *tempApiKey = json[WM_WEATHER_API_KEY_LABEL];
          const char *tempUnit = json[WM_WEATHER_UNIT_LABEL];

          // Ensure null-termination and copy to plexServerIp and plexServerToken
          strlcpy(weatherCityName, tempCityName, sizeof(weatherCityName));
          strlcpy(weatherCountryCode, tempCountryCode, sizeof(weatherCountryCode));
          strlcpy(weatherApikey, tempApiKey, sizeof(weatherApikey));
          strlcpy(weatherUnit, tempUnit, sizeof(weatherUnit));

#ifdef DEBUG
          Serial.println("Weather City Name: " + String(weatherCityName));
          Serial.println("Weather Country Code: " + String(weatherCountryCode));
          Serial.println("Weather API Key: " + String(weatherApikey));
#endif

          weatherConfigExist = true;
        }
        else
        {
          Serial.println("Config missing Weather credentials");
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

// Save Plex config to SPIFF
void saveWeatherConfig(const char *cityName, const char *countryCode, const char *apiKey, const char *unit)
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json[WM_WEATHER_CITY_NAME_LABEL] = cityName;       // Assigning C-style strings directly
  json[WM_WEATHER_COUNTRY_CODE_LABEL] = countryCode; // Assigning C-style strings directly
  json[WM_WEATHER_API_KEY_LABEL] = apiKey;           // Assigning C-style strings directly
  json[WM_WEATHER_UNIT_LABEL] = unit;                // Assigning C-style strings directly

  File configFile = SPIFFS.open(WEATHER_CONFIG_JSON, "w");
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
}

// Draw one of the available weather icons in the specified space
void drawWeatherIcon(int startx, int starty, int width, int height, const char *icon, bool enlarged)
{
  // Perform switch-case based on the entire string
  if (strcmp(icon, "01d") == 0)
  {
    drawBitmap(startx, starty, width, height, sun_24x24, enlarged);
  }
  else if (strcmp(icon, "01n") == 0)
  {
    drawBitmap(startx, starty, width, height, moon_24x24, enlarged);
  }
  else if (strcmp(icon, "02d") == 0)
  {
    drawBitmap(startx, starty, width, height, fewClouds_24x24, enlarged);
  }
  else if (strcmp(icon, "02n") == 0)
  {
    drawBitmap(startx, starty, width, height, fewCloudsNight_24x24, enlarged);
  }
  else if (strcmp(icon, "03d") == 0 || strcmp(icon, "03n") == 0)
  {
    drawBitmap(startx, starty, 24, 24, scatteredClouds_24x24, enlarged);
  }
  else if (strcmp(icon, "04d") == 0 || strcmp(icon, "04n") == 0)
  {
    drawBitmap(startx, starty, 24, 24, brokenClouds_24x24, enlarged);
  }
  else if (strcmp(icon, "09d") == 0)
  {
    drawBitmap(startx, starty, width, height, showers_24x24, enlarged);
  }
  else if (strcmp(icon, "09n") == 0)
  {
    drawBitmap(startx, starty, width, height, showersNight_24x24, enlarged);
  }
  else if (strcmp(icon, "10d") == 0 || strcmp(icon, "10n") == 0)
  {
    drawBitmap(startx, starty, width, height, rain_24x24, enlarged);
  }
  else if (strcmp(icon, "11d") == 0 || strcmp(icon, "11n") == 0)
  {
    drawBitmap(startx, starty, width, height, thunderstorm_24x24, enlarged);
  }
  else if (strcmp(icon, "13d") == 0 || strcmp(icon, "13n") == 0)
  {
    drawBitmap(startx, starty, width, height, snow_24x24, enlarged);
  }
  else if (strcmp(icon, "50d") == 0)
  {
    drawBitmap(startx, starty, width, height, mist_24x24, enlarged);
  }
}

void printTemperature(const char *icon, const char *buf, int x, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
{
  int padding = 2;
  int degreeSymbolWidth = 3;
  int iconWidth = 24;
  int iconHeight = 24;

  int16_t x1, y1;
  uint16_t w, h;
  dma_display->setFont(&font);
  dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);

  int iconStartX = (PANEL_WIDTH - iconWidth - padding - w - degreeSymbolWidth) / 2;
  int iconStartY = y - iconHeight + (iconHeight - h) / 2;

  // Clear the screen
  displayRect(0, iconStartY, PANEL_WIDTH, iconHeight, myBLACK); // clear full width of screen and height of icon

  drawWeatherIcon(iconStartX, iconStartY, iconWidth, iconHeight, icon, false);

  int textStartX = iconStartX + iconWidth + padding;
  dma_display->setCursor(textStartX, y);
  dma_display->setTextColor(textColor);
  dma_display->print(buf);

  // Calculate the position for the circle
  int circleX = textStartX + w + padding; // Add some padding between text and circle
  int circleY = y - h;                    // Center the circle vertically

  dma_display->drawCircle(circleX, circleY, 1, textColor);
}

std::string degreesToDirection(int degrees)
{
  std::string direction;
  int val = (degrees / 22.5) + 0.5;
  switch (val % 16)
  {
  case 0:
  case 15:
    direction = "N";
    break;
  case 1:
  case 2:
    direction = "NNE";
    break;
  case 3:
  case 4:
    direction = "NE";
    break;
  case 5:
  case 6:
    direction = "ENE";
    break;
  case 7:
  case 8:
    direction = "E";
    break;
  case 9:
  case 10:
    direction = "ESE";
    break;
  case 11:
  case 12:
    direction = "SE";
    break;
  case 13:
  case 14:
    direction = "SSE";
    break;
  default:
    direction = "";
  }
  return direction;
}

String getWeatherUnit()
{
  if (strcmp(weatherUnit, "1") == 0)
  {
    return "metric";
  }
  else if (strcmp(weatherUnit, "2") == 0)
  {
    return "standard";
  }
  else
  {
    return "imperial";
  }
}

void processWeatherJson(const char *response)
{
  // we need the timezone to setup the Time
  float timezone = extractFloatValue(response, "\"timezone\"");
  // printf("timezone: %.2f\n", timezone);
  configTime(long(timezone), daylightOffset_sec, "pool.ntp.org");

  if (selectedTheme == WEATHER_STATION_THEME || isScreenSaverMode)
  {
    std::string tempUnit = "";
    std::string pressureUnit = "";
    std::string windUnit = "";

    if (strcmp(weatherUnit, "1") == 0)
    {
      tempUnit = "C";
      pressureUnit = "hPa";
      windUnit = "m/s";
    }
    else if (strcmp(weatherUnit, "2") == 0)
    {
      tempUnit = "K";
      pressureUnit = "inHg";
      windUnit = "MPH";
    }
    else
    {
      tempUnit = "F";
      pressureUnit = "inHg";
      windUnit = "MPH";
    }

    const char *description = extractStringValue(response, "\"description\"", "Unknown");
    // printf("Weather Description: %s\n", description);
    std::string uppercasedDescription = toUpperCase(description);

    const char *icon = extractStringValue(response, "\"icon\"", "??");
    // printf("Weather Icon: %s\n", icon);

    float temp = extractFloatValue(response, "\"temp\"");
    // printf("Temperature: %.2f\n", temp);
    int tempInt = (int)temp;
    std::string tempString = std::to_string(tempInt);

    float feelsLike = extractFloatValue(response, "\"feels_like\"");
    // printf("Temperature min: %.2f\n", temp_min);
    int feelsLikeInt = (int)feelsLike;
    std::string feelsLikeString = "FL: " + std::to_string(feelsLikeInt) + tempUnit;

    float temp_min = extractFloatValue(response, "\"temp_min\"");
    // printf("Temperature min: %.2f\n", temp_min);
    int tempMinInt = (int)temp_min;
    std::string tempMinString = "L: " + std::to_string(tempMinInt) + tempUnit;

    float temp_max = extractFloatValue(response, "\"temp_max\"");
    // printf("Temperature max: %.2f\n", temp_max);
    int tempMaxInt = (int)temp_max;
    std::string tempMaxString = "H: " + std::to_string(tempMaxInt) + tempUnit;

    float pressure = extractFloatValue(response, "\"pressure\"");
    // printf("Pressure: %.2f\n", pressure);
    int pressureInt = (int)pressure;
    std::string pressureString = "P: " + std::to_string(pressureInt) + pressureUnit;

    float humidity = extractFloatValue(response, "\"humidity\"");
    // printf("Humidity: %.2f\n", humidity);
    int humidityInt = (int)humidity;
    std::string humidityString = "RH: " + std::to_string(humidityInt) + "%";

    float wind_direction = extractFloatValue(response, "\"deg\"");
    // printf("Wind direction: %.2f\n", wind_direction);
    int windDirectionInt = (int)wind_direction;

    float wind_speed = extractFloatValue(response, "\"speed\"");
    // printf("Wind speed: %.2f\n", wind_speed);
    int windSpeedInt = (int)wind_speed;
    std::string windString = "Wind: " + degreesToDirection(windDirectionInt) + " " + std::to_string(windSpeedInt) + windUnit;

    std::string extraInfo = uppercasedDescription + "    " + pressureString + "    " + windString;

    // display the info to LED Matrix
    String cleanCityName = String(weatherCityName);
    cleanCityName.replace("%20", " ");
    std::string uppercasedCityName = toUpperCase(cleanCityName.c_str());
    printCenter(uppercasedCityName.c_str(), 13, myORANGE);

    printTemperature(icon, tempString.c_str(), 22, 35, myGREEN, FreeSerifBold9pt7b);

    printLeft(feelsLikeString.c_str(), 5, 47, myPURPLE);
    printLeft(humidityString.c_str(), 5, 54, myGOLD);

    printLeft(tempMinString.c_str(), 40, 47, myCYAN);
    printLeft(tempMaxString.c_str(), 40, 54, myRED);

    lowerScrollingText = uppercasedDescription.c_str();

    lowerScrollingText = extraInfo.c_str();

    delay(500);
    getDateAndTime();
  }
}

void getWeatherInfo()
{
  String city = String(weatherCityName);
  city.replace("%20", "+"); // change space to + for cities with more than 1 word.
  String countryCode = String(weatherCountryCode);
  String apiKey = String(weatherApikey);
  String unit = getWeatherUnit();

  String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + apiKey + "&units=" + unit;

  httpGet(endpoint, "", "", [](int httpCode, const String &response)
          {
    if (httpCode == HTTP_CODE_OK) {
      String httpResponse = response;

#ifdef DEBUG
      Serial.println("http code: " + httpResponse);
#endif

      const char *jsonCString = httpResponse.c_str();
      processWeatherJson(jsonCString);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpCode);
    } });
}

void displayScreenSaver()
{
  if (weatherConfigExist)
  {
    getDateAndTime();
    if (!isScreenSaverMode)
    {
      isScreenSaverMode = true;
      resetAlbumArtVariables();
      clearScreen();
      getWeatherInfo();
    }
  }
  else
  {
    if (!isScreenSaverMode)
    {
      isScreenSaverMode = true;
      displayNoTrackPlaying();
    }
  }
}

#pragma endregion
// ******************************************* END WEATHER ****************************************************

// ******************************************* BEGIN PLEX ALBUM ART *******************************************
#pragma region PLEX_ALBUM_ART

#ifndef AV

#define WM_PLEX_SERVER_IP_LABEL "plexServerIp"
#define WM_PLEX_SERVER_PORT_LABEL "plexServerPort"
#define WM_PLEX_SERVER_TOKEN_LABEL "plexServerToken"
#define PLEX_CONFIG_JSON "/plex_config.json"

char plexServerIp[20];
char plexServerPort[10];
char plexServerToken[100];

void fetchPlexConfigFile()
{
  if (SPIFFS.exists(PLEX_CONFIG_JSON))
  {
    // file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open(PLEX_CONFIG_JSON, "r");
    if (configFile)
    {
      Serial.println("opened config file");
      StaticJsonDocument<512> json;
      DeserializationError error = deserializeJson(json, configFile);
      serializeJsonPretty(json, Serial);
      if (!error)
      {
        if (json.containsKey(WM_PLEX_SERVER_IP_LABEL) && json.containsKey(WM_PLEX_SERVER_PORT_LABEL) && json.containsKey(WM_PLEX_SERVER_TOKEN_LABEL))
        {
          const char *tempPlexServerIp = json[WM_PLEX_SERVER_IP_LABEL];
          const char *tempPlexServerPort = json[WM_PLEX_SERVER_PORT_LABEL];
          const char *tempPlexServerToken = json[WM_PLEX_SERVER_TOKEN_LABEL];

          // Ensure null-termination and copy to plexServerIp and plexServerToken
          strlcpy(plexServerIp, tempPlexServerIp, sizeof(plexServerIp));
          strlcpy(plexServerPort, tempPlexServerPort, sizeof(plexServerPort));
          strlcpy(plexServerToken, tempPlexServerToken, sizeof(plexServerToken));

#ifdef DEBUG
          Serial.println("Plex Server IP: " + String(plexServerIp));
          Serial.println("Plex Server Port: " + String(plexServerPort));
          Serial.println("Plex Server Token: " + String(plexServerToken));
#endif
        }
        else
        {
          Serial.println("Config missing Plex server IP or Port number or Auth token");
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

// Save Plex config to SPIFF
void savePlexConfig(const char *serverIp, const char *serverPort, const char *serverToken)
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json[WM_PLEX_SERVER_IP_LABEL] = serverIp;       // Assigning C-style strings directly
  json[WM_PLEX_SERVER_PORT_LABEL] = serverPort;   // Assigning C-style strings directly
  json[WM_PLEX_SERVER_TOKEN_LABEL] = serverToken; // Assigning C-style strings directly

  File configFile = SPIFFS.open(PLEX_CONFIG_JSON, "w");
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
}

void downloadPlexAlbumArt(const char *relativeUrl, const char *trackTitle, const char *artistName)
{
  String imageUrl = "http://" + String(plexServerIp) + ":" + String(plexServerPort) + "/photo/:/transcode?width=48&height=48&url=" + String(relativeUrl);
  String headerKey = "X-Plex-Token";
  String headerValue = plexServerToken;

  // Send GET request to the image URL
  httpGet(imageUrl, headerKey, headerValue, [&](int httpCode, const String &response)
          {
    if (httpCode == HTTP_CODE_OK) {
      // Successfully downloaded the image, now save it to SPIFFS
      File file = SPIFFS.open(ALBUM_ART, FILE_WRITE);
      if (!file) {
        Serial.println("Failed to create file");
        return;
      }
      file.print(response);
      file.close();
      // Check if the file exists
      if (SPIFFS.exists(ALBUM_ART)) {
        Serial.println("Image downloaded and saved successfully");
        // Update the last downloaded album art URL using the captured parameter
        clearScreen();
        lastAlbumArtURL = imageUrl;
        drawImagefromFile(ALBUM_ART, 8);
        scrollingText = trackTitle;
        lowerScrollingText = artistName;
      } else {
        Serial.println("Failed to save image to SPIFFS");
      }
    } else {
      Serial.print("Failed to download image. HTTP error code: ");
      Serial.println(httpCode);
    } });
}

void processPlexResponse(const String &payload)
{
  // Parse the XML response to get the last track title, artist name, and thumbnail URL

  int playerStateIndex = payload.indexOf("state=\"");

  String playerState = "";
  if (playerStateIndex != -1)
  {
    int stateStartIndex = playerStateIndex + 7; // Length of "state=\""
    int stateEndIndex = payload.indexOf("\"", stateStartIndex);
    playerState = payload.substring(stateStartIndex, stateEndIndex);
  }

  int lastTrackIndex = payload.lastIndexOf("<Track ");
  if (lastTrackIndex != -1 && playerState == "playing")
  {
    // Extract the last track XML block
    int trackEndIndex = payload.indexOf("</Track>", lastTrackIndex);
    String trackXML = payload.substring(lastTrackIndex, trackEndIndex);
    // Extract the track title
    int titleIndex = trackXML.indexOf("title=\"");
    if (titleIndex != -1)
    {
      int titleStartIndex = titleIndex + 7; // Length of "title=\""
      int titleEndIndex = trackXML.indexOf("\"", titleStartIndex);
      String trackTitle = trackXML.substring(titleStartIndex, titleEndIndex);
      // Extract the artist name
      int artistIndex = trackXML.indexOf("grandparentTitle=\"");
      if (artistIndex != -1)
      {
        int artistStartIndex = artistIndex + 18; // Length of "grandparentTitle=\""
        int artistEndIndex = trackXML.indexOf("\"", artistStartIndex);
        String artistName = trackXML.substring(artistStartIndex, artistEndIndex);
        // Extract the thumbnail URL
        int thumbIndex = trackXML.indexOf("thumb=\"");
        if (thumbIndex != -1)
        {
          int thumbStartIndex = thumbIndex + 7; // Length of "thumb=\""
          int thumbEndIndex = trackXML.indexOf("\"", thumbStartIndex);
          String coverArtURL = trackXML.substring(thumbStartIndex, thumbEndIndex);
          // Display or use the last track title, artist name, and thumbnail URL as needed
          if (!trackTitle.isEmpty() && !artistName.isEmpty() && !coverArtURL.isEmpty())
          {
#ifdef DEBUG
            Serial.println("Last Track Title: " + trackTitle);
            Serial.println("Artist Name: " + artistName);
            Serial.println("Last Thumbnail URL: " + coverArtURL);
#endif

            String cleanTrackTitle = decodeHtmlEntities(trackTitle);
            char trackTitleCharArray[cleanTrackTitle.length() + 1];
            cleanTrackTitle.toCharArray(trackTitleCharArray, cleanTrackTitle.length() + 1);

            String cleanArtistName = decodeHtmlEntities(artistName);
            char artistNameCharArray[cleanArtistName.length() + 1];
            cleanArtistName.toCharArray(artistNameCharArray, artistName.length() + 1);

            // Check if the current album art URL is different from the last one
            if (coverArtURL != lastAlbumArtURL)
            {
              // Delete old cover art
              deleteAlbumArt();

              // Allocate a character array with extra space for null-terminator
              char coverArtCharArray[coverArtURL.length() + 1];

              // Copy the content of the String to the char array
              coverArtURL.toCharArray(coverArtCharArray, coverArtURL.length() + 1);

              // Download and save the thumbnail image
              downloadPlexAlbumArt(coverArtCharArray, trackTitleCharArray, artistNameCharArray);
              lastAlbumArtURL = coverArtURL; // Update the last downloaded album art URL
            }
            else
            {
#ifdef DEBUG
              Serial.println("Album art hasn't changed. Skipping download.");
#endif

              // Trigger scrolling song title
              scrollingText = trackTitleCharArray;
              lowerScrollingText = artistNameCharArray;
            }
          }
          else
          {
            Serial.println("Incomplete information for the last played song.");
          }
        }
      }
    }
  }
  else if (lastAlbumArtURL != "")
  {
    getDateAndTime();
    displayMusicPaused();
  }
  else
  {
    displayScreenSaver();
  }
}

void getPlexCurrentTrack()
{
  String apiUrl = "http://" + String(plexServerIp) + ":" + String(plexServerPort) + "/status/sessions";
  String headerKey = "X-Plex-Token";
  String headerValue = String(plexServerToken);

  httpGet(apiUrl, headerKey, headerValue, [](int httpCode, const String &response)
          {
#ifdef DEBUG
            Serial.println("plex http code: " + httpCode);
#endif
    if (httpCode == HTTP_CODE_OK) {
      processPlexResponse(response);
    } else {
      displayScreenSaver();
    } });
}

#endif

#pragma endregion
// ******************************************* END PLEX ALBUM ART *********************************************

// ******************************************* BEGIN SPOTIFY ALBUM ART ****************************************
#pragma region SPOTIFY_ALBUM_ART

#ifndef AV

#include <base64.h>

#define WM_SPOTIFY_CLIENT_ID_LABEL "spotifyClientId"
#define WM_SPOTIFY_CLIENT_SECRET_LABEL "spotifyClientSecret"
#define WM_SPOTIFY_REFRESH_TOKEN_LABEL "spotifyRefreshToken"
#define SPOTIFY_CONFIG_JSON "/spotify_config.json"

char spotifyClientId[64];
char spotifyClientSecret[64];
char spotifyRefreshToken[200];

char spotifyImageUrl[128];      // Assuming the URL won't exceed 128 characters
char refreshedAccessToken[256]; // Adjust the size as needed

void getRefreshToken()
{
  Serial.println("***********Fetching Refresh Token*********");
  printCenter("REFRESHING", 25);
  printCenter("ACCESS TOKEN", 35);

  // Construct the authorization header
  String credentials = String(spotifyClientId) + ":" + String(spotifyClientSecret);
  String authHeader = "Basic " + base64::encode(credentials);

  // Construct the request payload
  String postData = "grant_type=refresh_token&refresh_token=" + String(spotifyRefreshToken);

  // Configure HTTP client
  HTTPClient http;
  http.begin("https://accounts.spotify.com/api/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", authHeader);

  // Send the POST request
  int httpResponseCode = http.POST(postData);
#ifdef DEBUG
  Serial.println("HTTP Response code: " + String(httpResponseCode));
#endif

  if (httpResponseCode == HTTP_CODE_OK)
  {
    String response = http.getString();
#ifdef DEBUG
    Serial.println("Response payload: " + response);
#endif
    // Extract the refresh_token using index-based parsing
    const char *responseChar = response.c_str();
    const char *refreshTokenStart = strstr(responseChar, "\"access_token\"");

    if (refreshTokenStart != nullptr)
    {
      refreshTokenStart = strstr(refreshTokenStart, ":") + 2; // Move to the actual token value
      const char *refreshTokenEnd = strchr(refreshTokenStart, '\"');

      if (refreshTokenEnd != nullptr)
      {
        strncpy(refreshedAccessToken, refreshTokenStart, refreshTokenEnd - refreshTokenStart);
        refreshedAccessToken[refreshTokenEnd - refreshTokenStart] = '\0'; // Null-terminate the string
#ifdef DEBUG
        Serial.println("Refreshed Access Token: " + String(refreshedAccessToken));
#endif
        clearScreen();
      }
      else
      {
        Serial.println("Unable to find the end of refreshed access token");
      }
    }
    else
    {
      Serial.println("Unable to find refreshed access token in the response");
    }
  }
  else
  {
    displayCheckSpotifyCredentials();
  }

  http.end();
}

void fetchSpotifyConfigFile()
{
  if (SPIFFS.exists(SPOTIFY_CONFIG_JSON))
  {
    // file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open(SPOTIFY_CONFIG_JSON, "r");
    if (configFile)
    {
      Serial.println("opened config file");
      StaticJsonDocument<512> json;
      DeserializationError error = deserializeJson(json, configFile);
      serializeJsonPretty(json, Serial);
      if (!error)
      {
        if (json.containsKey(WM_SPOTIFY_CLIENT_ID_LABEL) && json.containsKey(WM_SPOTIFY_CLIENT_SECRET_LABEL) && json.containsKey(WM_SPOTIFY_REFRESH_TOKEN_LABEL))
        {
          const char *tempSpotifyClientId = json[WM_SPOTIFY_CLIENT_ID_LABEL];
          const char *tempSpotifyClientSecret = json[WM_SPOTIFY_CLIENT_SECRET_LABEL];
          const char *tempSpotifyRefreshToken = json[WM_SPOTIFY_REFRESH_TOKEN_LABEL];

          // Ensure null-termination and copy to plexServerIp and plexServerToken
          strlcpy(spotifyClientId, tempSpotifyClientId, sizeof(spotifyClientId));
          strlcpy(spotifyClientSecret, tempSpotifyClientSecret, sizeof(spotifyClientSecret));
          strlcpy(spotifyRefreshToken, tempSpotifyRefreshToken, sizeof(spotifyRefreshToken));
#ifdef DEBUG
          Serial.println("Spotify Client ID: " + String(spotifyClientId));
          Serial.println("Spotify Client Secret: " + String(spotifyClientSecret));
          Serial.println("Spotify Refresh Token: " + String(spotifyRefreshToken));
#endif
        }
        else
        {
          Serial.println("Config missing Spotify credentials");
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

// Save Plex config to SPIFF
void saveSpotifyConfig(const char *clientId, const char *clientSecret, const char *refreshToken)
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json[WM_SPOTIFY_CLIENT_ID_LABEL] = clientId;         // Assigning C-style strings directly
  json[WM_SPOTIFY_CLIENT_SECRET_LABEL] = clientSecret; // Assigning C-style strings directly
  json[WM_SPOTIFY_REFRESH_TOKEN_LABEL] = refreshToken; // Assigning C-style strings directly

  File configFile = SPIFFS.open(SPOTIFY_CONFIG_JSON, "w");
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
}

void downloadSpotifyAlbumArt(String imageUrl)
{
  if (imageUrl == lastAlbumArtURL)
  {
#ifdef DEBUG
    Serial.println("Album art hasn't changed. Skipping download");
#endif
    return;
  }
  deleteAlbumArt();

  // Send GET request to the image URL
  httpGet(imageUrl, "", "", [&](int httpCode, const String &response)
          {
    if (httpCode == HTTP_CODE_OK) {
      // Successfully downloaded the image, now save it to SPIFFS
      File file = SPIFFS.open(ALBUM_ART, FILE_WRITE);
      if (!file) {
        Serial.println("Failed to create file");
        return;
      }
      file.print(response);
      file.close();
      // Check if the file exists
      if (SPIFFS.exists(ALBUM_ART)) {
        Serial.println("Image downloaded and saved successfully");
        // Update the last downloaded album art URL using the captured parameter
        lastAlbumArtURL = imageUrl;
        drawImagefromFile(ALBUM_ART, 0);
      } else {
        Serial.println("Failed to save image to SPIFFS");
      }
    } else {
      Serial.print("Failed to download image. HTTP error code: ");
      Serial.println(httpCode);
    } });
}

void processSpotifyJson(const char *response)
{
  char songName[64]; // Assuming the song name won't exceed 64 characters
  songName[0] = '\0';
  char artistName[64]; // Assuming the artist name won't exceed 64 characters
  artistName[0] = '\0';
  spotifyImageUrl[0] = '\0';

  // Find the start index of the last occurrence of the "name" key
  const char *nameKeyStart = strstr(response, "\"name\":");

  const char *isPlayingStart = strstr(response, "\"is_playing\"");
  if (isPlayingStart != nullptr)
  {
    // Move to the value part after ":"
    isPlayingStart = strchr(isPlayingStart, ':');

    if (isPlayingStart != nullptr)
    {
      // Move to the actual value
      isPlayingStart++;

      // Check if it's 'true' or 'false'
      if (strncmp(isPlayingStart, "true", 4) == 0)
      {
        if (isScreenSaverMode)
        {
          isScreenSaverMode = false;
        }
      }
      else if (strncmp(isPlayingStart, "false", 5) == 0)
      {
        // music player is paused.
        getDateAndTime();
        displayMusicPaused();
        return;
      }
    }
  }

  if (nameKeyStart == nullptr)
  {
    Serial.println("*******Null name key, nothing to process");
    getWeatherInfo();
    return;
  }

  const char *lastNameKeyStart = NULL;
  while (nameKeyStart != NULL)
  {
    lastNameKeyStart = nameKeyStart;
    nameKeyStart = strstr(nameKeyStart + 1, "\"name\":");
  }

  // If the last "name" key is found, get its associated value
  if (lastNameKeyStart != NULL)
  {
    const char *songNameStart = lastNameKeyStart + 8;       // Move to the start of the value
    const char *songNameEnd = strstr(songNameStart, "\","); // Find the end of the value
    if (songNameEnd != NULL && songNameStart < songNameEnd)
    {
      strncpy(songName, songNameStart, songNameEnd - songNameStart);
      songName[songNameEnd - songNameStart] = '\0'; // Null-terminate the string
#ifdef DEBUG
      Serial.println("Song Name: " + String(songName));
#endif
      scrollingText = String(songName);
    }
    else
    {
      Serial.println("Song name not found!");
    }
  }
  else
  {
    Serial.println("No 'name' key found in the JSON response!");
  }

  // Find the start and end indices of the "name" field for the artist
  const char *artistNameStart = strstr(response, "\"artists\"");
  artistNameStart = strstr(artistNameStart, "\"name\":\"") + 8;
  const char *artistNameEnd = strstr(artistNameStart, "\",");
  if (artistNameStart != NULL && artistNameEnd != NULL && artistNameStart < artistNameEnd)
  {
    strncpy(artistName, artistNameStart, artistNameEnd - artistNameStart);
    artistName[artistNameEnd - artistNameStart] = '\0'; // Null-terminate the string
    lowerScrollingText = String(artistName);
  }
  else
  {
    Serial.println("Artist name not found!");
  }

  // Extract the URL of the 64x64 image
  // Find the start index of the "images" key
  const char *imagesKeyStart = strstr(response, "\"images\"");
  if (imagesKeyStart != NULL)
  {
    // Find the start index of the array of images
    const char *imagesArrayStart = strstr(imagesKeyStart, "[");
    if (imagesArrayStart != NULL)
    {
      // Find the start index of the last image URL
      const char *lastImageUrlStart = NULL;
      const char *nextImageUrl = strstr(imagesArrayStart, "\"url\"");
      while (nextImageUrl != NULL)
      {
        lastImageUrlStart = nextImageUrl;
        nextImageUrl = strstr(lastImageUrlStart + 1, "\"url\"");
      }

      if (lastImageUrlStart != NULL)
      {
        // Find the start and end indices of the last image URL
        const char *lastImageUrlValueStart = strstr(lastImageUrlStart, "https://");
        const char *lastImageUrlEnd = strstr(lastImageUrlValueStart, "\",");
        if (lastImageUrlValueStart != NULL && lastImageUrlEnd != NULL)
        {
          strncpy(spotifyImageUrl, lastImageUrlValueStart, lastImageUrlEnd - lastImageUrlValueStart);
          spotifyImageUrl[lastImageUrlEnd - lastImageUrlValueStart] = '\0'; // Null-terminate the string
#ifdef DEBUG
          Serial.println("Image URL: " + String(spotifyImageUrl));
#endif
        }
        else
        {
          Serial.println("Invalid format for the last image URL!");
        }
      }
      else
      {
        Serial.println("No image URLs found in the 'images' array!");
      }
    }
    else
    {
      Serial.println("No array found for 'images' key!");
    }
  }
  else
  {
    Serial.println("No 'images' key found in the JSON response!");
  }
}

void getSpotifyCurrentTrack()
{
  // Your API endpoint
  String endpoint = "https://api.spotify.com/v1/me/player/currently-playing";

  // Authorization header
  String headerKey = "Authorization";
  String headerValue = "Bearer " + String(refreshedAccessToken);

  httpGet(endpoint, headerKey, headerValue, [](int httpCode, const String &response)
          {
#ifdef DEBUG
            Serial.println("http code: " + httpCode);
#endif

    if (httpCode == HTTP_CODE_UNAUTHORIZED) {
      Serial.println("***** Access token expired");
      restartDevice();
    } else if (httpCode == HTTP_CODE_NO_CONTENT) {
      displayScreenSaver();
    } else if (httpCode == HTTP_CODE_OK) {
      isScreenSaverMode = false;
      // Escape double quotes
      String escapedResponse = escapeSpecialCharacters(response);

      // Convert to const char*
      const char *jsonCString = escapedResponse.c_str();
      processSpotifyJson(jsonCString);

    } else {
      displayCheckSpotifyCredentials();
    } });
}

#endif

#pragma endregion
// ******************************************* END SPOTIFY ALBUM ART ******************************************

// ******************************************* BEGIN AUDIO VISUALIZER *****************************************
#pragma region AUDIO_VISUALIZER

#ifdef AV

#include <FastLED_NeoMatrix.h>
#include <arduinoFFT.h>
#include "I2SPLUGIN.h"
#include <math.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "FFT.h"
#include "Settings.h"
#include "PatternsHUB75.h"

#define up 1
#define down 0
int PeakDirection = 0;

long LastDoNothingTime = 0; // only needed for screensaver
int DemoModeMem = 0;        // to remember what mode we are in when going to demo, in order to restore it after wake up
bool AutoModeMem = false;   // same story
bool DemoFlag = false;      // we need to know if demo mode was manually selected or auto engadged.

char LCDPrintBuf[30];

// BucketFrequency
//
// Return the frequency corresponding to the Nth sample bucket.  Skips the first two
// buckets which are overall amplitude and something else.

int BucketFrequency(int iBucket)
{
  if (iBucket <= 1)
    return 0;
  int iOffset = iBucket - 2;
  return iOffset * (samplingFrequency / 2) / (SAMPLEBLOCK / 2);
}

void DrawVUPixels(int i, int yVU, int fadeBy = 0)
{
  CRGB VUC;
  if (i > (PANE_WIDTH / 3))
  {
    VUC.r = 255;
    VUC.g = 0;
    VUC.b = 0;
  }
  else if (i > (PANE_WIDTH / 5))
  {
    VUC.r = 255;
    VUC.g = 255;
    VUC.b = 0;
  }
  else
  { // green
    VUC.r = 0;
    VUC.g = 255;
    VUC.b = 0;
  }

  int xHalf = PANE_WIDTH / 2;
  dma_display->drawPixelRGB888(xHalf - i - 2, yVU, VUC.r, VUC.g, VUC.b);     // left side of screen line 0
  dma_display->drawPixelRGB888(xHalf - i - 2, yVU + 1, VUC.r, VUC.g, VUC.b); // left side of screen line 1
  dma_display->drawPixelRGB888(xHalf + i + 1, yVU, VUC.r, VUC.g, VUC.b);     // right side of screen line 0
  dma_display->drawPixelRGB888(xHalf + i + 1, yVU + 1, VUC.r, VUC.g, VUC.b); // right side of screen line 1
}

void DrawVUMeter(int yVU)
{
  static int iPeakVUy = 0;           // size (in LED pixels) of the VU peak
  static unsigned long msPeakVU = 0; // timestamp in ms when that peak happened so we know how old it is
  const int MAX_FADE = 256;
  for (int x = 0; x < PANE_WIDTH; x++)
  {
    dma_display->drawPixelRGB888(x, yVU, 0, 0, 0);
    dma_display->drawPixelRGB888(x, yVU + 1, 0, 0, 0);
  }
  if (iPeakVUy > 1)
  {
    int fade = MAX_FADE * (millis() - msPeakVU) / (float)1000;
    DrawVUPixels(iPeakVUy, yVU, fade);
  }
  int xHalf = (PANE_WIDTH / 2) - 1;
  int bars = map(gVU, 0, MAX_VU, 1, xHalf);
  bars = min(bars, xHalf);
  if (bars > iPeakVUy)
  {
    msPeakVU = millis();
    iPeakVUy = bars;
  }
  else if (millis() - msPeakVU > 1000)
    iPeakVUy = 0;
  for (int i = 0; i < bars; i++)
    DrawVUPixels(i, yVU);
}

void Calibration(void)
{
  Serial.printf("BandCalibration_XXXX[%1d]=\n{", numBands);
  long Totalbnd = 0;

  for (int g = 0; g < numBands; g++)
  {
    if (bndcounter[g] > Totalbnd)
      Totalbnd = bndcounter[g];
  }

  for (int g = 0; g < numBands; g++)
  {
    bndcounter[g] = Totalbnd / bndcounter[g];
    Serial.printf(" %2.2f", bndcounter[g]);
    if (g < numBands - 1)
      Serial.printf(",");
    else
      Serial.print(" };\n");
  }
}

void loopAudioVisualizer()
{
  size_t bytesRead = 0;

  // ############ Step 1: read samples from the I2S Buffer ##################
  i2s_read(I2S_PORT,
           (void *)samples,
           sizeof(samples),
           &bytesRead,     // workaround This is the actual buffer size last half will be empty but why?
           portMAX_DELAY); // no timeout

  if (bytesRead != sizeof(samples))
  {
    Serial.printf("Could only read %u bytes of %u in FillBufferI2S()\n", bytesRead, sizeof(samples));
    // return;
  }

  // ############ Step 2: compensate for Channel number and offset, safe all to vReal Array   ############
  for (uint16_t i = 0; i < ARRAYSIZE(samples); i++)
  {

    vReal[i] = offset - samples[i];
    vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }

  // ############ Step 3: Do FFT on the VReal array  ############
  //  compute FFT
  FFT.DCRemoval();
  FFT.Windowing(vReal, SAMPLEBLOCK, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLEBLOCK, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLEBLOCK);
  FFT.MajorPeak(vReal, SAMPLEBLOCK, samplingFrequency);
  for (int i = 0; i < numBands; i++)
  {
    FreqBins[i] = 0;
  }
  // ############ Step 4: Fill the frequency bins with the FFT Samples ############
  float averageSum = 0.0f;
  for (int i = 2; i < SAMPLEBLOCK / 2; i++)
  {
    averageSum += vReal[i];
    if (vReal[i] > NoiseTresshold)
    {
      int freq = BucketFrequency(i);
      int iBand = 0;
      while (iBand < numBands)
      {
        if (freq < BandCutoffTable[iBand])
          break;
        iBand++;
      }
      if (iBand > numBands)
        iBand = numBands;
      FreqBins[iBand] += vReal[i];
      //  float scaledValue = vReal[i];
      //  if (scaledValue > peak[iBand])
      //    peak[iBand] = scaledValue;
    }
  }

  // bufmd[0]=FreqBins[12];

  // ############ Step 5: Determine the VU value  and mingle in the readout...( cheating the bands ) ############ Step
  float t = averageSum / (SAMPLEBLOCK / 2);
  gVU = max(t, (oldVU * 3 + t) / 4);
  oldVU = gVU;

  if (selectedTheme == PLEX_ALBUM_ART_THEME || selectedTheme == SPOTIFY_ALBUM_ART_THEME)
  {
    DrawVUMeter(0);
    return;
  }

  if (gVU > DemoTreshold)
    LastDoNothingTime = millis(); // if there is signal in any off the bands[>2] then no demo mode

  // Serial.printf("gVu: %d\n",(int) gVU);

  for (int j = 0; j < numBands; j++)
  {
    if (CalibrationType == 1)
      FreqBins[j] *= BandCalibration_Pink[j];
    else if (CalibrationType == 2)
      FreqBins[j] *= BandCalibration_White[j];
    else if (CalibrationType == 3)
      FreqBins[j] *= BandCalibration_Brown[j];
  }

  //*
  // ############ Step 6: Averaging and making it all fit on screen
  // for (int i = 0; i < numBands; i++) {
  // Serial.printf ("Chan[%d]:%d",i,(int)FreqBins[i]);
  // FreqBins[i] = powf(FreqBins[i], gLogScale); // in case we want log scale..i leave it in here as reminder
  //  Serial.printf( " - log: %d \n",(int)FreqBins[i]);
  // }
  static float lastAllBandsPeak = 0.0f;
  float allBandsPeak = 0;
  // bufmd[1]=FreqBins[13];
  // bufmd[2]=FreqBins[1];
  for (int i = 0; i < numBands; i++)
  {
    // allBandsPeak = max (allBandsPeak, FreqBins[i]);
    if (FreqBins[i] > allBandsPeak)
    {
      allBandsPeak = FreqBins[i];
    }
  }
  if (allBandsPeak < 1)
    allBandsPeak = 1;
  //  The followinf picks allBandsPeak if it's gone up.  If it's gone down, it "averages" it by faking a running average of GAIN_DAMPEN past peaks
  allBandsPeak = max(allBandsPeak, ((lastAllBandsPeak * (GAIN_DAMPEN - 1)) + allBandsPeak) / GAIN_DAMPEN); // Dampen rate of change a little bit on way down
  lastAllBandsPeak = allBandsPeak;

  if (allBandsPeak < 80000)
    allBandsPeak = 80000;
  for (int i = 0; i < numBands; i++)
  {
    FreqBins[i] /= (allBandsPeak * 1.0f);
  }

  // Process the FFT data into bar heights
  for (int band = 0; band < numBands; band++)
  {
    int barHeight = FreqBins[band] * kMatrixHeight - 1; //(AMPLITUDE);
    if (barHeight > TOP - 2)
      barHeight = TOP - 2;

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band])
    {
      peak[band] = min(TOP, barHeight);
      PeakFlag[band] = 1;
    }
    bndcounter[band] += barHeight; // ten behoeve calibratie

    // if there hasn't been much of a input signal for a longer time ( see settings ) go to demo mode
    if ((millis() - LastDoNothingTime) > DemoAfterSec && DemoFlag == false)
    {
      DemoFlag = true;
      // first store current mode so we can go back to it after wake up
      DemoModeMem = buttonPushCounter;
      AutoModeMem = autoChangePatterns;
      autoChangePatterns = false;
      buttonPushCounter = 12;
      dma_display->clearScreen();
    }
    // Wait,signal is back? then wakeup!
    else if (DemoFlag == true && (millis() - LastDoNothingTime) < DemoAfterSec)
    { //("In loop 2:  %d", millis() - LastDoNothingTime);
      // while in demo the democounter was reset due to signal on one of the bars.
      // So we need to exit demo mode.
      dma_display->clearScreen();
      buttonPushCounter = DemoModeMem;  // restore settings
      autoChangePatterns = AutoModeMem; // restore settings
      DemoFlag = false;
      isScreenSaverMode = DemoFlag;
    }
    // Now visualize those bar heights
    switch (buttonPushCounter)
    {
    case 0:
      PeakDirection = down;
      BoxedBars(band, barHeight);
      BluePeak(band);
      break;

    case 1:
      PeakDirection = down;
      BoxedBars2(band, barHeight);
      BluePeak(band);
      break;
    case 2:
      PeakDirection = down;
      BoxedBars3(band, barHeight);
      RedPeak(band);
      break;
    case 3:
      PeakDirection = down;
      RedBars(band, barHeight);
      BluePeak(band);
      break;
    case 4:
      PeakDirection = down;
      ColorBars(band, barHeight);
      break;
    case 5:
      PeakDirection = down;
      Twins(band, barHeight);
      WhitePeak(band);
      break;
    case 6:
      PeakDirection = down;
      Twins2(band, barHeight);
      WhitePeak(band);
      break;
    case 7:
      PeakDirection = down;
      TriBars(band, barHeight);
      TriPeak(band);
      break;
    case 8:
      PeakDirection = up;
      TriBars(band, barHeight);
      TriPeak(band);
      break;
    case 9:
      PeakDirection = down;
      centerBars(band, barHeight);
      break;
    case 10:
      PeakDirection = down;
      centerBars2(band, barHeight);
      break;
    case 11:
      PeakDirection = down;
      BlackBars(band, barHeight);
      DoublePeak(band);
      break;
    case 12:
      displayScreenSaver();
      break;
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }
  // for calibration
  // bndcounter[h]+=barHeight;
  if (loopcounter == 256)
  {
    loopcounter = 0;
  }
  loopcounter++;

  if (buttonPushCounter != 12)
    DrawVUMeter(0); // Draw it when not in screensaver mode

  // Decay peak
  EVERY_N_MILLISECONDS(Fallingspeed)
  {
    for (byte band = 0; band < numBands; band++)
    {
      if (PeakFlag[band] == 1)
      {
        PeakTimer[band]++;
        if (PeakTimer[band] > Peakdelay)
        {
          PeakTimer[band] = 0;
          PeakFlag[band] = 0;
        }
      }
      else if ((peak[band] > 0) && (PeakDirection == up))
      {
        peak[band] += 1;
        if (peak[band] > (kMatrixHeight + 10))
          peak[band] = 0;
      } // when to far off screen then reset peak height
      else if ((peak[band] > 0) && (PeakDirection == down))
      {
        peak[band] -= 1;
      }
    }
    colorTimer++;
  }

  EVERY_N_MILLISECONDS(10)
  colorTimer++; // Used in some of the patterns

  EVERY_N_SECONDS(secToChangePattern)
  {
    // if (FastLED.getBrightness() == 0) FastLED.setBrightness(BRIGHTNESSMARK);  //Re-enable if lights are "off"
    if (autoChangePatterns)
    {
      buttonPushCounter = (buttonPushCounter + 1) % 12;
      dma_display->clearScreen();
    }
  }
} // loop end

// Function to update visualizer settings
void updateAudioVisualizerSettings(int pattern)
{
  if (pattern == 12)
  {
    autoChangePatterns = true;
  }
  else
  {
    autoChangePatterns = false;
    buttonPushCounter = pattern;
  }
}

#endif

#pragma endregion
// ******************************************* END AUDIO VISUALIZER *******************************************

// ******************************************* BEGIN WIFI SERVER ***********************************************
#pragma region WEBSERVER

#include "TuneFrameWebPage.h"

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
  else if (method == "POST" && path == "/reset-wifi")
  {
    client.println("HTTP/1.0 204 No Content");
    resetWifi();
    delay(1000);
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
        resetAlbumArtVariables();

        client.println("HTTP/1.0 204 No Content");
        savePreferences();
        currentlyRunningTheme = selectedTheme;
        clearScreen();

        force_restart = true;
        return;
      }
    }

    if (key == PREF_WEATHER_STATION_CREDENTIALS)
    {
      String payload = value;
      Serial.println("Received payload: " + value);

      int firstDelimiterIndex = payload.indexOf(',');
      String cityName = payload.substring(0, firstDelimiterIndex);
      String countryCodeAndApiKeyAndUnit = payload.substring(firstDelimiterIndex + 1);

      int secondDelimiterIndex = countryCodeAndApiKeyAndUnit.indexOf(',');
      String countryCode = countryCodeAndApiKeyAndUnit.substring(0, secondDelimiterIndex);
      String apiKeyAndUnit = countryCodeAndApiKeyAndUnit.substring(secondDelimiterIndex + 1);

      int thirdDelimiterIndex = apiKeyAndUnit.indexOf(',');
      String openweatherApiKey = apiKeyAndUnit.substring(0, thirdDelimiterIndex);
      String unit = apiKeyAndUnit.substring(thirdDelimiterIndex + 1);

      Serial.println("unit === " + unit);

      // Check if parameters are empty and provide default values if necessary
      cityName = (cityName.length() == 0) ? weatherCityName : cityName;
      countryCode = (countryCode.length() == 0) ? weatherCountryCode : countryCode;
      openweatherApiKey = (openweatherApiKey.length() == 0) ? weatherApikey : openweatherApiKey;
      unit = (unit.length() == 0) ? weatherUnit : unit;

      client.println("HTTP/1.0 204 No Content");
      saveWeatherConfig(cityName.c_str(), countryCode.c_str(), openweatherApiKey.c_str(), unit.c_str());

      selectedTheme = WEATHER_STATION_THEME;

      client.println("HTTP/1.0 204 No Content");
      savePreferences();
      currentlyRunningTheme = selectedTheme;
      clearScreen();

      force_restart = true;
      return;
    }

    if (key == PREF_GIF_ART_NAME)
    {
      String payload = value;
      Serial.println("Received payload: " + payload);
      client.println("HTTP/1.0 204 No Content");
      return;
    }

#ifdef AV
    if (key == PREF_AV_PATTERN)
    {
      client.println("HTTP/1.0 204 No Content");
      updateAudioVisualizerSettings(value.toInt());
      return;
    }
#else
    if (key == PREF_PLEX_CREDENTIALS)
    {
      String payload = value;
      Serial.println("Received payload: " + value);

      // Extract server address, server port, and auth token from the payload
      int firstDelimiterIndex = payload.indexOf(',');
      int secondDelimiterIndex = payload.indexOf(',', firstDelimiterIndex + 1);

      String serverAddress = payload.substring(0, firstDelimiterIndex);
      String serverPortAndToken = payload.substring(firstDelimiterIndex + 1);
      int thirdDelimiterIndex = serverPortAndToken.indexOf(',');
      String serverPort = serverPortAndToken.substring(0, thirdDelimiterIndex);
      String authToken = serverPortAndToken.substring(thirdDelimiterIndex + 1);

#ifdef DEBUG
      Serial.println("Server Address: " + serverAddress);
      Serial.println("Server Port: " + serverPort);
      Serial.println("Auth Token: " + authToken);
#endif
      // Check if parameters are empty and provide default values if necessary
      serverAddress = (serverAddress.length() == 0) ? plexServerIp : serverAddress;
      serverPort = (serverPort.length() == 0) ? plexServerPort : serverPort;
      authToken = (authToken.length() == 0) ? plexServerToken : authToken;

      client.println("HTTP/1.0 204 No Content");
      savePlexConfig(serverAddress.c_str(), serverPort.c_str(), authToken.c_str());

      selectedTheme = PLEX_ALBUM_ART_THEME;
      resetAlbumArtVariables();

      client.println("HTTP/1.0 204 No Content");
      savePreferences();
      currentlyRunningTheme = selectedTheme;
      clearScreen();

      force_restart = true;
      return;
    }

    if (key == PREF_SPOTIFY_CREDENTIALS)
    {
      String payload = value;
      Serial.println("Received payload: " + value);

      // Extract server address, server port, and auth token from the payload
      int firstDelimiterIndex = payload.indexOf(',');
      int secondDelimiterIndex = payload.indexOf(',', firstDelimiterIndex + 1);

      String clientId = payload.substring(0, firstDelimiterIndex);
      String clientSecretAndRefreshToken = payload.substring(firstDelimiterIndex + 1);
      int thirdDelimiterIndex = clientSecretAndRefreshToken.indexOf(',');
      String clientSecret = clientSecretAndRefreshToken.substring(0, thirdDelimiterIndex);
      String refreshToken = clientSecretAndRefreshToken.substring(thirdDelimiterIndex + 1);

#ifdef DEBUG
      Serial.println("Spotify Client Id: " + clientId);
      Serial.println("Spotify Client secret: " + clientSecret);
      Serial.println("Spotify Refresh Token: " + refreshToken);
#endif
      // Check if parameters are empty and provide default values if necessary
      clientId = (clientId.length() == 0) ? spotifyClientId : clientId;
      clientSecret = (clientSecret.length() == 0) ? spotifyClientSecret : clientSecret;
      refreshToken = (refreshToken.length() == 0) ? spotifyRefreshToken : refreshToken;

      client.println("HTTP/1.0 204 No Content");
      saveSpotifyConfig(clientId.c_str(), clientSecret.c_str(), refreshToken.c_str());

      selectedTheme = SPOTIFY_ALBUM_ART_THEME;
      resetAlbumArtVariables();

      client.println("HTTP/1.0 204 No Content");
      savePreferences();
      currentlyRunningTheme = selectedTheme;
      clearScreen();

      force_restart = true;
      return;
    }
#endif
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
// ******************************************* END WIFI SERVER *************************************************

// ******************************************* BEGIN MAIN *****************************************************
#pragma region MAIN

#include <HTTPUpdate.h>

int failedConnectionAttempts = 0;
const int MAX_FAILED_ATTEMPTS = 5;
unsigned long lastWeatherUpdateTime = 0;
unsigned long lastClockUpdateTime = 0;
const unsigned long weatherUpdateInterval = 600000; // 10 minutes
const unsigned long clockUpdateInterval = 1000; // 10 minutes
unsigned long lastAlbumArtUpdateTime = 0;
const unsigned long albumArtUpdateInterval = 5000; // 5000 milliseconds

void update_progress(int cur, int total)
{
  // Display progress
  float progress = cur * 100.0 / total;
  char buffer[30];
  sprintf(buffer, "%.0f%%", progress);
  printCenter(buffer, 40, myGREEN, FreeSerifBold9pt7b);
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

  // Cleanup from last session
  deleteAlbumArt();

  preferences.begin("TUNEFRAME", false);
  loadPreferences();
  currentlyRunningTheme = selectedTheme;

  printCenter("TUNEFRAME", 30);

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

  // Display IP address
  IPAddress ipAddress = WiFi.localIP();
  char ipAddressString[16];
  sprintf(ipAddressString, "%s", ipAddress.toString().c_str());
  printCenter(ipAddressString, 10, myBLUE);

  Serial.println("Connected to WiFi");
  printCenter("Connected to WiFi.", 50, myGREEN);
  delay(5000);

  clearScreen();
  Serial.println("\r\nInitialisation done.");

#ifdef AV
  if (selectedTheme == PLEX_ALBUM_ART_THEME || selectedTheme == SPOTIFY_ALBUM_ART_THEME)
  {
    Serial.print("will update firmware to TuneFrameAlbumArt_Firmware");
    // update firmware to canvas
    WiFiClientSecure client;
    client.setInsecure();

    // Reading data over SSL may be slow, use an adequate timeout
    client.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

    printCenter(ipAddressString, 10, myBLUE);
    printCenter("Loading..", 20, myORANGE);
    printCenter("COVER ART", 30, myPURPLE);

    httpUpdate.onProgress(update_progress);

    t_httpUpdate_return ret = httpUpdate.update(client, "https://raw.githubusercontent.com/robegamesios/TUNEFRAME/blob/main/binFiles/TuneFrameAlbumArt_Firmware.bin");

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
    return;
  }
  else
  {
    // Setup audio visualizer
    setupI2S();
  }
#else
  if (selectedTheme == PLEX_ALBUM_ART_THEME)
  {
    fetchPlexConfigFile();
  }
  else if (selectedTheme == SPOTIFY_ALBUM_ART_THEME)
  {
    fetchSpotifyConfigFile();
    getRefreshToken();
  }
  else if (selectedTheme == AUDIO_VISUALIZER_THEME)
  {
    Serial.print("will update firmware to TuneFrameAV_Firmware");
    // update firmware to canvas
    WiFiClientSecure client;
    client.setInsecure();

    // Reading data over SSL may be slow, use an adequate timeout
    client.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

    printCenter(ipAddressString, 10, myBLUE);
    printCenter("Loading..", 20, myORANGE);
    printCenter("AUDIO VISUALIZER", 30, myPURPLE);

    httpUpdate.onProgress(update_progress);

    t_httpUpdate_return ret = httpUpdate.update(client, "https://raw.githubusercontent.com/robegamesios/TUNEFRAME/blob/main/binFiles/TuneFrameAV_Firmware.bin");

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
    return;
  }
#endif

  // get the weather
  fetchWeatherConfigFile();
  if (weatherConfigExist)
  {
    getWeatherInfo();
  }
  else
  {
    displayCheckWeatherCredentials();
  }

  server.begin();
}

void loop()
{
  if (isConnected())
  {
    handleHttpRequest();

#ifdef AV
    // Default to audio visualizer
    loopAudioVisualizer();

    if (isScreenSaverMode && weatherConfigExist)
    {
      printScrolling(scrollingText.c_str(), 5, myBLUE);
      printScrolling2(lowerScrollingText.c_str(), 62, myBLUE);
    }
#else
    // Check album art every 5 seconds
    unsigned long currentMillis = millis();

    if (selectedTheme == WEATHER_STATION_THEME && weatherConfigExist)
    {

      if (currentMillis - lastClockUpdateTime >= clockUpdateInterval)
      {
        lastClockUpdateTime = currentMillis;
        getDateAndTime();
      }

      if (currentMillis - lastWeatherUpdateTime >= weatherUpdateInterval)
      {
        lastWeatherUpdateTime = currentMillis;
        getWeatherInfo();
      }
    }
    else if (selectedTheme == PLEX_ALBUM_ART_THEME)
    {
      if (currentMillis - lastAlbumArtUpdateTime >= albumArtUpdateInterval)
      {
        lastAlbumArtUpdateTime = currentMillis;
        getPlexCurrentTrack();
      }
    }
    else if (selectedTheme == SPOTIFY_ALBUM_ART_THEME)
    {
      if (currentMillis - lastAlbumArtUpdateTime >= albumArtUpdateInterval)
      {
        lastAlbumArtUpdateTime = currentMillis;
        getSpotifyCurrentTrack();
        if (strlen(spotifyImageUrl) > 0 && !isScreenSaverMode)
        {
          downloadSpotifyAlbumArt(String(spotifyImageUrl));
        }
      }
    }

    printScrolling(scrollingText.c_str(), 5, myBLUE);
    printScrolling2(lowerScrollingText.c_str(), 62, myBLUE);
#endif
  }
}

#pragma endregion
// ******************************************* END MAIN *******************************************************
