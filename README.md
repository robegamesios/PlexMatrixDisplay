# TUNEFRAME

"TuneFrame" is an ESP32-powered LED matrix panel that serves as a dynamic music visualizer. It syncs captivating LED light patterns with your favorite tunes and displays album art from Spotify and PlexAmp, enhancing your listening experience with vibrant visuals.

# Video tutorial : https://youtu.be/m5aLhCCSwWE

# Hardware

TBD

# Software:

# Using Over the Air Update:

1. Download otaDefaultUpdate.ino from here: https://github.com/robegamesios/TUNEFRAME/blob/f9670607f996004907c58c464b867fd6b849f5fa/otaDefaultUpdate/otaDefaultUpdate.ino
2. Download the TuneFrameFirmware.bin from here: https://github.com/robegamesios/TUNEFRAME/blob/f9670607f996004907c58c464b867fd6b849f5fa/binFiles/TuneFrameFirmware.bin
3. Open otaDefaultUpdate.ino, then edit lines 8 and 9 to use your 2.4Ghz wifi credentials.
4. Upload it to your ESP32.
5. Open the Arduino IDE Serial Monitor (top right icon that looks like a Magnifying glass). Notice the wifi IP address.
6. Open your web browser and access the IP address.
7. Leave the User ID and Password blank, and click Login.
8. Choose the file you downloaded from step 2, then click Update.
9. Wait for the upload to finish, it should restart the ESP32 and you see TUNEFRAME at startup.

# Using Visual Studio Code:

1. Fork the TUNEFRAME repo or download the zip file.
2. Import the project in Visual Studio Code using Platform.io. (I'm assuming you are familiar on how to import a platform.io project).

# Fetching Spotify Refresh Token via Curl commands:

1. Download `get-spotify-auth-code.js` from here: https://github.com/robegamesios/TUNEFRAME/blob/926691ebbc9ba7eea4be319e6fc88895b16115f2/misc/get-spotify-auth-code.js
2. Open `get-spotify-auth-code.js` and change lines 8 and 9 to use your Spotify Client ID and Client Secret. Then run it to get the authorization code.
3. Next run this CURL command to get the access token and refresh token:
   ```
   curl -d client_id=[client_id] -d client_secret=[client_secret] -d grant_type=authorization_code -d code=[auth code from step 2] -d redirect_uri=http%3A%2F%2F[esp32 ip address]%2Fcallback/ https://accounts.spotify.com/api/token


