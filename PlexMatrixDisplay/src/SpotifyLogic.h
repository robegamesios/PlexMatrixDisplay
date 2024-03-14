#include <ArduinoJson.h>
#include <SPIFFS.h>

#define WM_SPOTIFY_CLIENT_ID_LABEL "spotifyClientId"
#define WM_SPOTIFY_CLIENT_SECRET_LABEL "spotifyClientSecret"
#define WM_SPOTIFY_REFRESH_TOKEN_LABEL "spotifyRefreshToken"
#define SPOTIFY_CONFIG_JSON "/spotify_config.json"

char spotifyClientId[64];
char spotifyClientSecret[64];
char spotifyRefreshToken[200];

#ifdef SPOTIFY_MODE

#include <base64.h>
#include "HelperFunctions.h"

char refreshedAccessToken[256]; // Adjust the size as needed

String spotifyAlbumArtUrl = "";

String fetchSpotifyRefreshToken()
{
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

    return response;
  }

  Serial.println("Unable to find the end of refreshed access token");
  return "";

  http.end();
}

boolean fetchSpotifyConfigFile()
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
          return true;
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
  return false;
}

#endif

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
