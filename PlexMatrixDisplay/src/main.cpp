// ******************************************* BEGIN Matrix display *******************************************
#pragma region MATRIXDISPLAY

#include <FS.h>
#include <SPIFFS.h>
#include <JPEGDEC.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "picopixel.h"

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
      64, // module width
      64, // module height
      1   // Chain length
  );
  mxconfig.gpio.e = 18;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
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
  int16_t x1, y1;
  uint16_t w, h;
  dma_display->setFont(&Picopixel);
  dma_display->getTextBounds(buf, 0, y, &x1, &y1, &w, &h);
  dma_display->setCursor(32 - (w / 2), y);
  dma_display->setTextColor(0xffff);
  dma_display->print(buf);
}

int drawImagefromFile(const char *imageFileUri)
{
  unsigned long lTime = millis();
  lTime = millis();
  jpeg.open((const char *)imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
  int decodeStatus = jpeg.decode(0, 0, 0);
  jpeg.close();
  Serial.print("Time taken to decode and display Image (ms): ");
  Serial.println(millis() - lTime);
  return decodeStatus;
}

#pragma endregion
// ******************************************* END Matrix display *********************************************

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

void restartDevice() {
  delay(3000);
  ESP.restart();
}

void resetWifi()
{
  wifiManager.resetSettings();
}

void fetchConfigFile()
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
  json[WM_PLEX_SERVER_PORT_LABEL] = plexServerPort; // Assigning C-style strings directly
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

String lastAlbumArtURL = ""; // Variable to store the last downloaded album art URL
bool albumArtChanged = false;

void downloadCoverArt(const char *relativeUrl)
{
  HTTPClient http;
  // Construct the full URL by appending the relative URL to the base URL
  String imageUrl = "http://" + String(plexServerIp) + ":" + String(plexServerPort) + "/photo/:/transcode?width=64&height=64&url=" + String(relativeUrl);
  // Send GET request to the image URL
  if (http.begin(imageUrl))
  {
    // Set the authentication token in the request headers
    http.addHeader("X-Plex-Token", plexServerToken);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      // Successfully downloaded the image, now save it to SPIFFS
      File file = SPIFFS.open(ALBUM_ART, FILE_WRITE);
      if (!file)
      {
        Serial.println("Failed to create file");
        return;
      }
      http.writeToStream(&file);
      file.close();
      // Check if the file exists
      if (SPIFFS.exists(ALBUM_ART))
      {
        Serial.println("Image downloaded and saved successfully");
        drawImagefromFile(ALBUM_ART);
      }
      else
      {
        Serial.println("Failed to save image to SPIFFS");
      }
    }
    else
    {
      Serial.print("Failed to download image. HTTP error code: ");
      Serial.println(httpCode);
    }
    http.end();
  }
  else
  {
    Serial.println("Failed to connect to image URL");
  }
}

void getAlbumArt()
{
  HTTPClient http;
  // Construct the Plex API URL to get the currently playing item
  String apiUrl = "http://" + String(plexServerIp) + ":" + String(plexServerPort) + "/status/sessions";

  if (http.begin(apiUrl))
  {
    // Set the authentication token in the request headers
    http.addHeader("X-Plex-Token", plexServerToken);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      String payload = http.getString();
      // Serial.println("*****************BEGIN PAYLOAD********************");
      // Serial.println(payload);
      // Serial.println("*****************END PAYLOAD********************");
      // Parse the XML response to get the last track title, artist name, and thumbnail URL
      int lastTrackIndex = payload.lastIndexOf("<Track ");
      if (lastTrackIndex != -1)
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
                Serial.println("Last Track Title: " + trackTitle);
                Serial.println("Artist Name: " + artistName);
                Serial.println("Last Thumbnail URL: " + coverArtURL);
                // Check if the current album art URL is different from the last one
                if (coverArtURL != lastAlbumArtURL)
                {
                  albumArtChanged = true;
                  // Allocate a character array with extra space for null-terminator
                  char charArray[coverArtURL.length() + 1];
                  // Copy the content of the String to the char array
                  coverArtURL.toCharArray(charArray, coverArtURL.length() + 1);
                  // Download and save the thumbnail image
                  downloadCoverArt(charArray);
                  lastAlbumArtURL = coverArtURL; // Update the last downloaded album art URL
                }
                else
                {
                  albumArtChanged = false;
                  Serial.println("Album art hasn't changed. Skipping download.");
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
      else
      {
        Serial.println("No track is currently playing.");
      }
    }
    else
    {
      Serial.println("HTTP request failed");
    }
    http.end();
  }
  else
  {
    Serial.println("Unable to connect to Plex server");
  }
}

int failedConnectionAttempts = 0;
const int MAX_FAILED_ATTEMPTS = 5;

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

  printCenter(" PLEX ", 10);
  printCenter("MATRIX", 20);
  printCenter("DISPLAY", 30);
  delay(3000);

  fetchConfigFile();

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
  delay(2000);

  Serial.println("\r\nInitialisation done.");
}

void loop()
{
  if (isConnected())
  {
    getAlbumArt();
  }
  delay(5000); // Check every 5 seconds
}