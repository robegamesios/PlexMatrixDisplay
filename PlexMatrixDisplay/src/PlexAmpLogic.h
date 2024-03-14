#include <ArduinoJson.h>
#include <SPIFFS.h>

#define WM_PLEX_SERVER_IP_LABEL "plexServerIp"
#define WM_PLEX_SERVER_PORT_LABEL "plexServerPort"
#define WM_PLEX_SERVER_TOKEN_LABEL "plexServerToken"
#define PLEX_CONFIG_JSON "/plex_config.json"

char plexServerIp[20];
char plexServerPort[10];
char plexServerToken[100];

#ifdef PLEXAMP_MODE

#include "HelperFunctions.h"

struct PlexData
{
    String artUrl;
    String trackTitle;
    String artistName;

    PlexData(String art, String title, String artist)
        : artUrl(art), trackTitle(title), artistName(artist) {}
};

PlexData processPlexResponse(const String &payload)
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

#ifdef DEBUG
    Serial.println("****player state:" + playerState);
#endif

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

                        String cleanTrackTitle = HelperFunctions::decodeHtmlEntities(trackTitle);
                        char trackTitleCharArray[cleanTrackTitle.length() + 1];
                        cleanTrackTitle.toCharArray(trackTitleCharArray, cleanTrackTitle.length() + 1);

                        String cleanArtistName = HelperFunctions::decodeHtmlEntities(artistName);
                        char artistNameCharArray[cleanArtistName.length() + 1];
                        cleanArtistName.toCharArray(artistNameCharArray, artistName.length() + 1);

                        PlexData data(coverArtURL, cleanTrackTitle, cleanArtistName);
                        return data;
                    }
                }
                else
                {
                    Serial.println("Incomplete information for the last played song.");
                }
            }
        }
    }
    PlexData data("", "", "paused");
    return data;
}

boolean fetchPlexConfigFile()
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
                    return true;
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
    return false;
}

#endif

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
