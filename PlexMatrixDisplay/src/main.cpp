#define WEATHER_STATION_THEME 90
#define AUDIO_VISUALIZER_THEME 100
#define PLEX_ALBUM_ART_THEME 200
#define SPOTIFY_ALBUM_ART_THEME 201
#define ANIMATED_GIF_THEME 210

// #define DEBUG // UnComment to see debug prints

#define ENABLE_MATRIX_DISPLAY // Comment this to disable matrix display (for debugging purposes when no display is connected)

#include "MatrixDisplay.h"
#include "TFWifiManager.h"
#include "TFNetworking.h"
#include "WeatherClock.h"
#include "PlexAmpLogic.h"
#include "SpotifyLogic.h"
#include "AnimatedGifLogic.h"
#include "jRead.h"

// ******************************************* BEGIN GLOBAL VARS AND COMMON FUNC ******************************
#pragma region GLOBAL_VARS_AND_UTILS

uint8_t selectedTheme;
uint8_t currentlyRunningTheme;
uint8_t panelBrightness;

String scrollingText = "";
String lowerScrollingText = "";
String lastAlbumArtURL = ""; // Variable to store the last downloaded album art URL
bool musicPlayerActive = false;

struct ThemeInfo
{
  String title;
  String filename;

  ThemeInfo(String name, String file)
      : title(name), filename(file) {}
};

ThemeInfo getThemeInfo()
{
  if (selectedTheme == WEATHER_STATION_THEME)
  {
    return ThemeInfo("WEATHER CLOCK", "weatherclock_FW");
  }
  else if (selectedTheme == AUDIO_VISUALIZER_THEME)
  {
    return ThemeInfo("MUSIC VISUALIZER", "musicVisualizer_FW");
  }
  else if (selectedTheme == PLEX_ALBUM_ART_THEME)
  {
    return ThemeInfo("PlEXAMP ART", "plexArt_FW");
  }
  else if (selectedTheme == SPOTIFY_ALBUM_ART_THEME)
  {
    return ThemeInfo("SPOTIFY ART", "spotifyArt_FW");
  }
  else if (selectedTheme == ANIMATED_GIF_THEME)
  {
    return ThemeInfo("ANIMATED GIF ART", "animatedGifArt_FW");
  }

  return ThemeInfo("UNKNOWN THEME", "");
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

  musicPlayerActive = false;
  resetAlbumArtVariables();
  clearScreen();
  printCenter("NO TRACK IS", 20, myPURPLE);
  printCenter("CURRENTLY", 30, myPURPLE);
  printCenter("PLAYING", 40, myPURPLE);
}

void displayCheckWeatherCredentials()
{
#ifdef DEBUG
  Serial.println("Check Weather credentials");
#endif

  resetAlbumArtVariables();
  clearScreen();
  printCenter("CHECK WEATHER", 25, myRED);
  printCenter("CREDENTIALS", 35, myRED);
}

void displayCheckPlexAmpCredentials()
{
#ifdef DEBUG
  Serial.println("Check PlexAmp credentials");
#endif

  resetAlbumArtVariables();
  clearScreen();
  printCenter("CHECK PLEXAMP", 25, myRED);
  printCenter("CREDENTIALS", 35, myRED);
}

void displayCheckSpotifyCredentials()
{
#ifdef DEBUG
  Serial.println("Check Spotify credentials");
#endif

  resetAlbumArtVariables();
  clearScreen();
  printCenter("CHECK SPOTIFY", 25, myRED);
  printCenter("CREDENTIALS", 35, myRED);
}

void displayMusicPaused()
{
  String musicPaused = "Music Paused";
  if (lowerScrollingText != musicPaused)
  {
    lowerScrollingText = musicPaused;
  }
}

void displayFailureReloadGifArt()
{
#ifdef DEBUG
  Serial.println("cannot open gif file");
#endif

  printCenter("FAILURE..", 15, myRED);
  printCenter("TRY LOADING", 25, myRED);
  printCenter("GIF ART AGAIN", 35, myRED);
  printCenter("FROM BROWSER", 45, myRED);
}

void restartDevice()
{
  clearScreen();
  printCenter("RESTARTING..", 30);
  delay(1000);
  ESP.restart();
}

void resetWifi()
{
  printCenter("RESETTING WiFi..", 30);
  wifiManager.resetSettings();
}

#pragma endregion
// ******************************************* END GLOBAL VARS AND COMMON FUNC ********************************

// ******************************************* BEGIN PREFERENCES **********************************************
#pragma region PREFERENCES

#include <Preferences.h>

Preferences preferences;
const char *PREF_SELECTED_THEME = "selectedTheme";
const char *PREF_AV_PATTERN = "avPattern";
const char *PREF_GIF_ART_URL = "gifArtUrl";
const char *PREF_WEATHER_STATION_CREDENTIALS = "weatherStationCredentials";
const char *PREF_PLEX_CREDENTIALS = "plexCredentials";
const char *PREF_SPOTIFY_CREDENTIALS = "spotifyCredentials";
const char *PREF_DISPLAY_GIF_TIME_AND_WEATHER = "displayGifTimeAndWeather";
const char *PREF_PANEL_BRIGHTNESS = "panelBrightness";

void savePreferences()
{
  preferences.putUInt(PREF_SELECTED_THEME, selectedTheme);
  preferences.putInt(PREF_PANEL_BRIGHTNESS, panelBrightness);
}

void loadPreferences()
{
  selectedTheme = preferences.getUInt(PREF_SELECTED_THEME, AUDIO_VISUALIZER_THEME);
  panelBrightness = preferences.getUInt(PREF_PANEL_BRIGHTNESS, 128);
}

#pragma endregion
// ******************************************* END PREFERENCES ************************************************

// ******************************************* BEGIN CLOCK ****************************************************
#pragma region CLOCK

#if defined(WEATHERCLOCK_MODE) || defined(ANIMATEDGIF_MODE)

unsigned long lastWeatherUpdateTime = 0;
unsigned long lastClockUpdateTime = 0;
const unsigned long weatherUpdateInterval = 600000; // 10 minutes
const unsigned long clockUpdateInterval = 1000;     // 1 second

bool displayGifTimeAndWeather = false;

void getDateAndTime()
{
  String localTime = getLocalTimeAndDate();
#ifdef DEBUG
  Serial.println("current time = " + localTime);
#endif
  scrollingText = localTime;
}

#endif

#pragma endregion
// ******************************************* END CLOCK ******************************************************

// ******************************************* BEGIN WEATHER **************************************************
#pragma region WEATHER

#ifdef WEATHERCLOCK_MODE

#include "weatherIcons.h"

// Draw one of the available weather icons in the specified space
void drawWeatherIcon(int startx, int starty, int width, int height, std::string icon, bool enlarged)
{
  // Perform switch-case based on the entire string
  if (icon == "01d")
  {
    drawBitmap(startx, starty, width, height, sun_24x24, enlarged);
  }
  else if (icon == "01n")
  {
    drawBitmap(startx, starty, width, height, moon_24x24, enlarged);
  }
  else if (icon == "02d")
  {
    drawBitmap(startx, starty, width, height, fewClouds_24x24, enlarged);
  }
  else if (icon == "02n")
  {
    drawBitmap(startx, starty, width, height, fewCloudsNight_24x24, enlarged);
  }
  else if (icon == "03d" || icon == "03n")
  {
    drawBitmap(startx, starty, 24, 24, scatteredClouds_24x24, enlarged);
  }
  else if (icon == "04d" || icon == "04n")
  {
    drawBitmap(startx, starty, 24, 24, brokenClouds_24x24, enlarged);
  }
  else if (icon == "09d")
  {
    drawBitmap(startx, starty, width, height, showers_24x24, enlarged);
  }
  else if (icon == "09n")
  {
    drawBitmap(startx, starty, width, height, showersNight_24x24, enlarged);
  }
  else if (icon == "10d" || icon == "10n")
  {
    drawBitmap(startx, starty, width, height, rain_24x24, enlarged);
  }
  else if (icon == "11d" || icon == "11n")
  {
    drawBitmap(startx, starty, width, height, thunderstorm_24x24, enlarged);
  }
  else if (icon == "13d" || icon == "13n")
  {
    drawBitmap(startx, starty, width, height, snow_24x24, enlarged);
  }
  else if (icon == "50d")
  {
    drawBitmap(startx, starty, width, height, mist_24x24, enlarged);
  }
}

void printTemperature(std::string icon, const char *buf, int x, int y, uint16_t textColor = myWHITE, GFXfont font = Picopixel)
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

void displayWeatherData(WeatherData data)
{
  printCenter(data.cityName.c_str(), 13, myORANGE);

  printTemperature(data.weatherIcon, data.temperature.c_str(), 22, 35, myGREEN, FreeSerifBold9pt7b);

  printLeft(data.feelsLike.c_str(), 5, 47, myPURPLE);
  printLeft(data.humidity.c_str(), 5, 54, myGOLD);

  printLeft(data.tempMin.c_str(), 40, 47, myCYAN);
  printLeft(data.tempMax.c_str(), 40, 54, myRED);

  lowerScrollingText = data.extraInfo.c_str();

  delay(500);
  getDateAndTime();
}

#endif

#if defined(WEATHERCLOCK_MODE) || defined(ANIMATEDGIF_MODE)

void getWeatherInfo()
{
  String city = String(weatherCityName);
  city.replace("%20", "+"); // change space to + for cities with more than 1 word.
  String countryCode = String(weatherCountryCode);
  String apiKey = String(weatherApikey);
  String unit = getWeatherUnit();

  String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + apiKey + "&units=" + unit;

#ifdef DEBUG
  Serial.println("weather endpoint = " + endpoint);
#endif

  HTTPClient http;
  http.begin(endpoint);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String httpResponse = http.getString();
#ifdef DEBUG
    Serial.println("http response: " + httpResponse);
#endif

    const char *jsonCString = httpResponse.c_str();
    WeatherData data = processWeatherJson(jsonCString);

#ifdef WEATHERCLOCK_MODE
#ifdef ENABLE_MATRIX_DISPLAY
    displayWeatherData(data);
#else
    String fullInfo = data.fullInfo.c_str();
    Serial.println("Weather Data = " + fullInfo);
#endif

#elif ANIMATEDGIF_MODE
    lowerScrollingText = data.fullInfo.c_str();
#endif
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpCode);
  }

  http.end();
}

void fetchWeatherAndTime()
{
  unsigned long currentMillis = millis();

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
  printScrolling(scrollingText.c_str(), 5, myBLUE);
  printScrolling2(lowerScrollingText.c_str(), 62, myBLUE);
}

#endif

#pragma endregion
// ******************************************* END WEATHER ****************************************************

// ******************************************* BEGIN PLEX ALBUM ART *******************************************
#pragma region PLEX_ALBUM_ART

#ifdef PLEXAMP_MODE

void downloadPlexAlbumArt(String relativeUrl, String trackTitle, String artistName)
{
  String imageUrl = "http://" + String(plexServerIp) + ":" + String(plexServerPort) + "/photo/:/transcode?width=48&height=48&url=" + relativeUrl;
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
      PlexData data = processPlexResponse(response);
            // Check if the current album art URL is different from the last one
            String coverArtUrl = data.artUrl;
            String trackTitle = data.trackTitle;
            String artistName = data.artistName;

            if (artistName == "")
            {
              displayNoTrackPlaying();
            }
            if (artistName == "paused")
            {
                displayMusicPaused();
            }
            else if (coverArtUrl != lastAlbumArtURL)
            {
              // Delete old cover art
              deleteAlbumArt();

              // Download and save the thumbnail image
              downloadPlexAlbumArt(coverArtUrl, trackTitle, artistName);
              lastAlbumArtURL = coverArtUrl; // Update the last downloaded album art URL
            }
            else
            {
#ifdef DEBUG
              Serial.println("Album art hasn't changed. Skipping download.");
#endif

              // Trigger scrolling song title
              scrollingText = trackTitle;
              lowerScrollingText = artistName;
            }

    } else {
      displayNoTrackPlaying();
    } });
}

#endif

#pragma endregion
// ******************************************* END PLEX ALBUM ART *********************************************

// ******************************************* BEGIN SPOTIFY ALBUM ART ****************************************
#pragma region SPOTIFY_ALBUM_ART

#ifdef SPOTIFY_MODE

void getSpotifyRefreshToken()
{
  Serial.println("***********Fetching Refresh Token*********");
  printCenter("REFRESHING", 25, myCYAN);
  printCenter("ACCESS TOKEN", 35, myCYAN);

  String response = fetchSpotifyRefreshToken();

  if (response.length() > 0)
  {
    // parse the refreshed access token
    const char *pJson = response.c_str();
    jRead_string(pJson, "{'access_token'", refreshedAccessToken, 256);
    clearScreen();
  }
  else
  {
    Serial.println("Unable to find the end of refreshed access token");
    displayCheckSpotifyCredentials();
  }
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

void processSpotifyJson(const char *pJson)
{
  char str[128]; // general use string buffer
  jRead_string(pJson, "{'item'{'name'", str, 128);
  String trackTitle = str;

  jRead_string(pJson, "{'item'{'album'{'artists'[{'name'", str, 128);
  String artistName = str;

  jRead_string(pJson, "{'item'{'album'{'images'[2{'url'", str, 128);
  String albumArtUrl = str;

  jRead_string(pJson, "{'is_playing'", str, 128);
  String isPlaying = str;

#ifdef DEBUG
  Serial.println("Spotify track title: " + trackTitle);
  Serial.println("Spotify artist name: " + artistName);
  Serial.println("Spotify album Art Url: " + albumArtUrl);
  Serial.println("Spotify is playing: " + isPlaying);
#endif

  if (isPlaying != "true")
  {
    displayMusicPaused();
    return;
  }

  scrollingText = trackTitle;
  lowerScrollingText = artistName;

  if (albumArtUrl != lastAlbumArtURL)
  {
    spotifyAlbumArtUrl = albumArtUrl;
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
      displayNoTrackPlaying();
    } else if (httpCode == HTTP_CODE_OK) {
      musicPlayerActive = true;
      // Convert to const char*
      const char *jsonCString = response.c_str();
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

#ifdef AV_MODE

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

// ******************************************* BEGIN GIF PLAYER ***********************************************
#pragma region GIF_PLAYER

#ifdef ANIMATEDGIF_MODE

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
  y = pDraw->iY + pDraw->y + y_offset; // current line

  if (displayGifTimeAndWeather && (y < 7 || y > 56))
  {
    return;
  }
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
      } // while looking for opaque pixels
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
      dma_display->drawPixel(x + x_offset, y, usPalette[*s++]); // color 565
    }
  }
}

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
#ifdef DEBUG
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d; offset x = %d y = %d\n", gif.getCanvasWidth(), gif.getCanvasHeight(), x_offset, y_offset);
#endif
      gifState = STATE_PLAY_GIF;
    }
    else
    {
      displayFailureReloadGifArt();
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

boolean downloadSuccess = false; // Global variable to track download success

void handleGifDownload(int httpCode, const String &response)
{
  if (httpCode == HTTP_CODE_OK)
  {
    deleteGifArt();

    // Open a file to save the downloaded GIF
    File file = SPIFFS.open(GIF_ART, FILE_WRITE);
    if (file)
    {
      file.print(response);
      file.close();

      // Check if the file exists
      if (SPIFFS.exists(GIF_ART))
      {
        Serial.println("Image downloaded and saved successfully");
        downloadSuccess = true;
      }
      else
      {
        Serial.println("Failed to save image to SPIFFS");
      }
    }
    else
    {
      Serial.println("Failed to create file");
    }
  }
  else
  {
    Serial.print("Failed to download image. HTTP error code: ");
    Serial.println(httpCode);
  }
}

boolean downloadGifArt(String filename)
{
  // Construct the full URL for the GIF file
  String gifURL = filename + ".gif";

#ifdef DEBUG
  Serial.println("*************GIF url = " + gifURL);
#endif

  downloadSuccess = false; // Reset download success flag

  httpGet(gifURL, "", "", handleGifDownload);

  return downloadSuccess;
}

void httpsDownloadGifArt(String filename)
{
  // Initialize HTTP client
  HTTPClient http;

  // Specify the base URL of the GIF files on raw.githubusercontent.com
  String gifURL = filename + ".gif";

  // #ifdef DEBUG
  Serial.println("*************GIF url = " + gifURL);
  // #endif

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
          clearScreen();
          gif.close();
          gifState = STATE_IDLE;
          showGIF();
        }
        else
        {
          Serial.println("Failed to save image to SPIFFS");
          displayFailureReloadGifArt();
        }
      }
      else
      {
        Serial.println("Failed to create file");
        displayFailureReloadGifArt();
      }
    }
    else
    {
      Serial.print("Failed to download image. HTTP error code: ");
      displayFailureReloadGifArt();
    }
    // End the HTTP client
    http.end();
  }
  else
  {
    Serial.println("Failed to connect to image URL");
    clearScreen();
    printCenter("FAILED TO", 20);
    printCenter("CONNECT TO", 30);
    printCenter("IMAGE URL", 40);
  }
}

#endif

#pragma endregion
// ******************************************* END GIF PLAYER **************************************************

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

    if (key == PREF_PANEL_BRIGHTNESS)
    {
      panelBrightness = value.toInt();
      dma_display->setPanelBrightness(panelBrightness);
      client.println("HTTP/1.0 204 No Content");
      savePreferences();
    }

#ifdef AV_MODE
    if (key == PREF_AV_PATTERN)
    {
      client.println("HTTP/1.0 204 No Content");
      updateAudioVisualizerSettings(value.toInt());
      return;
    }
#endif

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

#ifdef ANIMATEDGIF_MODE
    if (key == PREF_GIF_ART_URL)
    {
      String payload = value;

#ifdef DEBUG
      Serial.println("Received payload: " + payload);
#endif

      client.println("HTTP/1.0 204 No Content");

      if (payload.startsWith("https://"))
      {
        httpsDownloadGifArt(payload);
        return;
      }

      bool downloadSuccess = downloadGifArt(payload);
      if (downloadSuccess)
      {
        clearScreen();
        gif.close();
        gifState = STATE_IDLE;
        showGIF();
      }
      else
      {
#ifdef DEBUG
        Serial.println("******** Failed to show GIF");
#endif

        clearScreen();
        printCenter("FAILED TO", 20, myRED);
        printCenter("SHOW GIF ART", 30, myRED);
        gif.close();
        gifState = STATE_IDLE;
      }
      return;
    }

    if (key == PREF_DISPLAY_GIF_TIME_AND_WEATHER)
    {
      displayGifTimeAndWeather = value == "true" && fetchWeatherConfigFile();

      client.println("HTTP/1.0 204 No Content");
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
unsigned long lastAlbumArtUpdateTime = 0;
const unsigned long albumArtUpdateInterval = 5000; // 5 seconds

void update_progress(int cur, int total)
{
  // Display progress
  float progress = cur * 100.0 / total;
  char buffer[30];
  sprintf(buffer, "%.0f%%", progress);
  printCenter(buffer, 50, myGREEN, FreeSerifBold9pt7b);
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
  dma_display->setPanelBrightness(panelBrightness);
  currentlyRunningTheme = selectedTheme;

  printCenter("TUNEFRAME", 30);

  if (!wifiConnect())
  {
    printCenter("FAILED to Connect", 30);
    restartDevice();
    return;
  }

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
      return;
    }
  }

  // Display IP address
  IPAddress ipAddress = WiFi.localIP();
  char ipAddressString[16];
  sprintf(ipAddressString, "%s", ipAddress.toString().c_str());
  printCenter(ipAddressString, 10, myBLUE);

  Serial.println("Connected to WiFi");
  printCenter("Connected to WiFi.", 50, myGREEN);
  delay(1000);

  clearScreen();
  Serial.println("\r\nInitialisation done.");

#ifdef WEATHERCLOCK_MODE
#define CURRENT_ENV WEATHER_STATION_THEME
#elif defined(AV_MODE)
#define CURRENT_ENV AUDIO_VISUALIZER_THEME

#elif defined(PLEXAMP_MODE)
#define CURRENT_ENV PLEX_ALBUM_ART_THEME

#elif defined(SPOTIFY_MODE)
#define CURRENT_ENV SPOTIFY_ALBUM_ART_THEME

#elif defined(ANIMATEDGIF_MODE)
#define CURRENT_ENV ANIMATED_GIF_THEME
#endif

  if (selectedTheme != CURRENT_ENV)
  {
    Serial.print("starting update of TuneFrame Firmware");
    // update firmware to canvas
    WiFiClientSecure client;
    client.setInsecure();

    // Reading data over SSL may be slow, use an adequate timeout
    client.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

    printCenter(ipAddressString, 10, myBLUE);
    printCenter("Loading..", 20, myORANGE);
    String title = getThemeInfo().title;
    String filename = getThemeInfo().filename;
    printCenter(title.c_str(), 30, myPURPLE);

    httpUpdate.onProgress(update_progress);

    String baseUrl = "https://raw.githubusercontent.com/robegamesios/TUNEFRAME/main/binFiles/";

    String firmwareUrl = baseUrl + filename + ".bin";

    t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl);

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

#if defined(WEATHERCLOCK_MODE) || defined(ANIMATEDGIF_MODE)
  // get the weather
  if (fetchWeatherConfigFile())
  {
    getWeatherInfo();
    delay(1000);
  }
  else
  {
    displayCheckWeatherCredentials();
  }
#endif

#ifdef AV_MODE
  // Setup audio visualizer
  setupI2S();
#endif

#ifdef PLEXAMP_MODE
  if (!fetchPlexConfigFile())
  {
    displayCheckPlexAmpCredentials();
  }
#endif

#ifdef SPOTIFY_MODE
  if (fetchSpotifyConfigFile())
  {
    getSpotifyRefreshToken();
  }
  else
  {
    displayCheckSpotifyCredentials();
  }
#endif

#ifdef ANIMATEDGIF_MODE
// setup for Animated GIF
#endif

  server.begin();
}

void loop()
{
  if (isConnected())
  {
    handleHttpRequest();

#if defined(WEATHERCLOCK_MODE)
    fetchWeatherAndTime();
#endif

#ifdef AV_MODE
    loopAudioVisualizer();
#endif

#ifdef PLEXAMP_MODE
    unsigned long currentMillis = millis();

    if (currentMillis - lastAlbumArtUpdateTime >= albumArtUpdateInterval && !isTextScrolling && !isTextScrolling2) //! isTextScrolling && !isTextScrolling2 is just a work around for scrolling text to not pause when polling is done
    {
      lastAlbumArtUpdateTime = currentMillis;
      getPlexCurrentTrack();
    }
    printScrolling(scrollingText.c_str(), 5, myBLUE);
    printScrolling2(lowerScrollingText.c_str(), 62, myBLUE);
#endif

#ifdef SPOTIFY_MODE
    unsigned long currentMillis = millis();

    if (currentMillis - lastAlbumArtUpdateTime >= albumArtUpdateInterval && !isTextScrolling && !isTextScrolling2) //! isTextScrolling && !isTextScrolling2 is just a work around for scrolling text to not pause when polling is done
    {
      lastAlbumArtUpdateTime = currentMillis;
      getSpotifyCurrentTrack();

      if (spotifyAlbumArtUrl.length() > 0 && musicPlayerActive)
      {
        downloadSpotifyAlbumArt(spotifyAlbumArtUrl);
      }
    }
    printScrolling(scrollingText.c_str(), 5, myBLUE);
    printScrolling2(lowerScrollingText.c_str(), 62, myBLUE);
#endif

#ifdef ANIMATEDGIF_MODE
    if (displayGifTimeAndWeather)
    {
      fetchWeatherAndTime();
    }
    showGIF();
#endif
  }
}

#pragma endregion
// ******************************************* END MAIN *******************************************************
