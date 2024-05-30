# TUNEFRAME

![tuneframe](https://github.com/robegamesios/TUNEFRAME/assets/10041871/6f5bf6e1-6c4b-49f1-a3ac-ad2c2bdf3d92)

![sddefault](https://github.com/robegamesios/TUNEFRAME/assets/10041871/2d4f611e-7fed-4663-86e8-12363abf33bd)

![Screenshot 2024-05-30 091634](https://github.com/robegamesios/TUNEFRAME/assets/10041871/0b28931d-e569-4500-8c21-d548cfde7373)

TuneFrame is an ESP32-powered LED matrix panel that serves as a dynamic music visualizer. It syncs captivating LED light patterns with your favorite tunes and displays album art from Spotify and PlexAmp, enhancing your listening experience with vibrant visuals. It also serves as a weather station providing current weather information, and as an Animated GIF display to play your favorite GIF art.

# Video tutorials: 

https://youtu.be/m5aLhCCSwWE?si=qLfko10hiLxpDJDM

https://youtu.be/RzFt6x_XNsM?si=SkvgdLdjnOrtg5Ka

# Hardware

The project will cost less than $30 to build. You can get any of these items somewhere else, but I found Aliexpress to be the cheapest vendor.

1. P3 64 x 64 LED Matrix Panel ($15) - https://www.aliexpress.us/item/3256803863694770.html?spm=a2g0o.productlist.main.3.4ff27a74nqpHbg&algo_pvid=bf682a3e-05b4-4958-92d1-a9cfc81a1af0&aem_p4p_detail=202402282054061388946146196280000774142&algo_exp_id=bf682a3e-05b4-4958-92d1-a9cfc81a1af0-1&pdp_npi=4%40dis%21USD%2133.12%2111.56%21%21%2133.12%2111.56%21%402101fb1117091824460494444e1dfb%2112000027861763733%21sea%21US%210%21AB&curPageLogUid=r7uBKgI8R3wV&utparam-url=scene%3Asearch%7Cquery_from%3A&search_p4p_id=202402282054061388946146196280000774142_2
2. INMP441 Omnidirectional Microphone module ($1) - https://www.aliexpress.us/item/3256805923157007.html?spm=a2g0o.productlist.main.1.19185e2fcMhZ5D&algo_pvid=af6c30c3-4e38-4dc2-aad6-335a03bf63e0&algo_exp_id=af6c30c3-4e38-4dc2-aad6-335a03bf63e0-0&pdp_npi=4%40dis%21USD%214.95%210.99%21%21%2135.51%217.13%21%402103080717091453651281542e63cb%2112000035788707400%21sea%21US%210%21AB&curPageLogUid=60rGypPtLQcJ&utparam-url=scene%3Asearch%7Cquery_from%3A
3. ESP32 Wroom 32 Development Board ($3) - https://www.aliexpress.us/item/3256806034074322.html?spm=a2g0o.productlist.main.3.7e07faf9NOwugQ&algo_pvid=7095655f-7c51-45af-bfd9-53728ed615f4&algo_exp_id=7095655f-7c51-45af-bfd9-53728ed615f4-1&pdp_npi=4%40dis%21USD%2110.52%210.99%21%21%2175.44%217.09%21%402101e58b17091454129601513e5d07%2112000036337209451%21sea%21US%210%21AB&curPageLogUid=wHW6Ua4rmBmQ&utparam-url=scene%3Asearch%7Cquery_from%3A
4. Dupont cables ($2) - https://www.aliexpress.us/item/3256805941164824.html?spm=a2g0o.productlist.main.3.40e0e4fceP8JKN&algo_pvid=0921ba51-af56-45c3-827a-596400023cb8&algo_exp_id=0921ba51-af56-45c3-827a-596400023cb8-1&pdp_npi=4%40dis%21USD%213.13%210.99%21%21%2122.45%217.12%21%402103205217091454542188529e4d68%2112000035962714816%21sea%21US%210%21AB&curPageLogUid=82wePhoUlfkk&utparam-url=scene%3Asearch%7Cquery_from%3A
5. 5V 2A power adapter (higher current the better) ($5) - https://www.aliexpress.us/item/3256802434250774.html?spm=a2g0o.productlist.main.17.61c928160Z0yhE&algo_pvid=d4b5595b-a54e-4474-b64a-14c91990539c&aem_p4p_detail=2024022820511014725752996342080000001570&algo_exp_id=d4b5595b-a54e-4474-b64a-14c91990539c-8&pdp_npi=4%40dis%21USD%216.84%210.99%21%21%216.84%210.99%21%402101fb0c17091822709174611e0ed6%2112000021491724526%21sea%21US%210%21AB&curPageLogUid=KhTauQnzDska&utparam-url=scene%3Asearch%7Cquery_from%3A&search_p4p_id=2024022820511014725752996342080000001570_9
   
# Pinouts

Pinout information can be found here: https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA. Below is the connection between the panel and the ESP32. Use female-to-female dupont cables to connect them together.

![tuneframePinout](https://github.com/robegamesios/TUNEFRAME/assets/10041871/ff1a1cfb-e1f9-40e2-96bc-e4a88dcd2414)

# LED Panel => ESP32

R1 => PIN 25

G1 => PIN 26

B1 => PIN 27

N = GND => Connect to common ground (no. 14 on ESP32 diagram)

R2 => PIN 14

G2 => PIN 12

B2 => PIN 13

E => PIN 18

A => PIN 23

B => PIN 19

C => PIN 5

D => PIN 17

CK => PIN 16

LS (also known as LAT) => 4

OE => PIN 15

N = GND => Connect to common ground (no. 14 on ESP32 diagram)

# INMP441 => ESP32

SD => PIN 32

VDD => 3.3V (no. 1 on ESP32 diagram)

GND => Connect to common ground (no. 14 on ESP32 diagram)

L/R => Connect to common ground (no. 14 on ESP32 diagram)

WS => PIN 2

SCK => PIN 33

# Using a single Power Supply

CAUTION: DO NOT CONNECT THE Vin 5V Pin to the power supply and also connect a micro-usb cable to your computer at the same time, this will fry your ESP32. If you're uploading code using the micro-usb cable, make sure the Vin 5V Pin is not connected to anything.

If you want to use a single power supply for both the LED Panel and the ESP32, you can daisy chain from the 5V power supply and connect the positive (+) terminal to the Vin 5V Pin (no. 19 in the diagram) of the ESP32.

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

# Get your PlexAmp Auth token:

1. Login to your Plex Media Server using Chrome.
2. Go to your music library, then right click on the page and click "Inspect".
3. Go to the "elements" tab.
4. Search for X-Plex-Token key. Your Auth token will be the characters squeezed between 2 X-Plex-Token keys.
   ```
   X-Plex-Token%3DThisIsYourAuthToken&X-Plex-Token

# Fetching Spotify Refresh Token via Curl command:

You can also get your spotify refresh token using this website: https://robegamesios.github.io/spotify-refresh-token/

1. Download `get-spotify-auth-code.js` from here: https://github.com/robegamesios/TUNEFRAME/blob/926691ebbc9ba7eea4be319e6fc88895b16115f2/misc/get-spotify-auth-code.js
2. Open `get-spotify-auth-code.js` and change lines 8 and 9 to use your Spotify Client ID and Client Secret. Then run it to get the authorization code.
3. Next run this CURL command to get the access token and refresh token:
   ```
   curl -d client_id=[client_id] -d client_secret=[client_secret] -d grant_type=authorization_code -d code=[auth code from step 2] -d redirect_uri=http%3A%2F%2F[esp32 ip address]%2Fcallback/ https://accounts.spotify.com/api/token


