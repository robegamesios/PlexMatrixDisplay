#pragma once

#include <Arduino.h>

// TODO: Move the whole JS to a CDN soon

const char SETTINGS_PAGE[] PROGMEM = R""""(
<!DOCTYPE html>
<html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Card Example</title>
    <style>
        .card {
            display: inline-block;
            vertical-align: top;
            width: 250px;
            border: 1px solid #ccc;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            margin: 10px;
            padding: 20px;
        }

        .card-title {
            font-size: 20px;
            font-weight: bold;
        }

        .card-description {
            margin-top: 10px;
        }

        .card-image {
            width: 100%;
            max-width: 100%;
            height: auto;
            margin-top: 10px;
        }

        .update-button {
            display: block;
            margin-top: 10px;
            background-color: #007bff;
            color: #fff;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            text-align: center;
            text-decoration: none;
            cursor: pointer;
        }
    </style>
</head>

<title>Clockwise Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<link rel="shortcut icon" type="image/x-icon"
  href="https://github.com/jnthas/clockwise/raw/gh-pages/static/images/favicon.png">

<body>

  <div class="w3-container" style="background-image: linear-gradient(120deg, #155799, #159957);">
    <img class="w3-animate-zoom w3-padding w3-image"
      src="data:image/png;base64,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" alt="Logo"
      style="width:40%">
  </div>

  <div class="w3-bar w3-black w3-medium">
    <div id="fw-version" class="w3-bar-item w3-black w3-hover-red"></div>
    <div class="w3-bar-item w3-button w3-hover-yellow w3-right" onclick="resetWifi();"><i class='fa fa-power-off'></i> Reset Wifi</div>
    <div id="ssid" class="w3-bar-item w3-hover-blue w3-right"></div>
    <div class="w3-bar-item w3-button w3-hover-yellow w3-right" onclick="restartDevice();"><i class='fa fa-power-off'></i> Restart Device</div>
    <div id="status" class="w3-bar-item w3-green" style="display:none"><i class='fa fa-floppy-o'></i> Saved! Restart your device</div>
  </div>

<div class="card">
    <h2 class="card-title">Music Visualizer</h2>
    <p class="card-description">Shows audio spectrum that syncs with your music.</p>
    <img class="card-image" src="https://github.com/robegamesios/PlexMatrixDisplay/blob/a21c8d41fbd933d12098baaacfc8a5523ff8d2c5/thumbnails/audio_visualizer.jpg?raw=true" alt="Music Visualizer">
    <br><br>
    <select id="optionSelect" name='option'>
    <option value='13' title='Select Visualizer Pattern'>Select Visualizer Pattern</option>
    <option value='12' title='Auto Change every 1 Min'>Auto Change every 1 Min</option>
    <option value='0' title='Boxed Bars'>Boxed Bars</option>
    <option value='1' title='Boxed Bars 2'>Boxed Bars 2</option>
    <option value='2' title='Boxed Bars 3'>Boxed Bars 3</option>
    <option value='3' title='Red Bars'>Red Bars</option>
    <option value='4' title='Color Bars'>Color Bars</option>
    <option value='5' title='=Twins'>Twins</option>
    <option value='6' title='Twins 2'>Twins 2</option>
    <option value='7' title='Tri Bars'>Tri Bars</option>
    <option value='8' title='Tri Bars 2'>Tri Bars 2</option>
    <option value='9' title='Center Bars'>Center Bars</option>
    <option value='10' title='Center Bars 2'>Center Bars 2</option>
    <option value='11' title='Black Bars'>Black Bars</option>
    </select><br><br>
    <a class="update-button" onclick="updatePreference('selectedTheme', 100)">Select</a>
    
</div>
<div class="card">
    <h2 class="card-title">Plex Music Cover Art</h2>
    <p class="card-description">Displays Plex Music Cover Art.</p>
    <img class="card-image" src="https://github.com/robegamesios/PlexMatrixDisplay/blob/main/thumbnails/plex_cover_art_2.jpg?raw=true" alt="Plex Music Cover Art">
    <label for="serverAddress">Server Address:</label>
    <input type="text" id="serverAddress" name="serverAddress"><br><br>
     <label for="serverPort">Server Port Number:</label>
    <input type="text" id="serverPort" name="serverPort"><br><br>
    <label for="authToken">Auth Token:</label>
    <input type="text" id="authToken" name="authToken"><br><br>
    <button onclick="updatePlexCredentials()">Save Settings</button><br><br>
    <a class="update-button" onclick="updatePreference('selectedTheme', 200)">Select</a>
</div>
<div class="card">
    <h2 class="card-title">GIF Art</h2>
    <p class="card-description">Display your favorite GIF Art.</p>
    <img class="card-image" src="https://github.com/robegamesios/PlexMatrixDisplay/blob/main/thumbnails/gif_art.jpg?raw=true" alt="Gif Art">
    <br><br>
    <select id="gifOptionSelect" name='Select a GIF Art'>
    </select><br><br>
    <button onclick="uploadGifArt()">Load Art</button><br><br>
    <a class="update-button" onclick="updatePreference('selectedTheme', 210)">Select</a>
</div>
<div class="w3-container" style="background-image: linear-gradient(120deg, #155799, #159957);">
  <img class="w3-animate-zoom w3-padding w3-image"
    src="data:image/png;base64,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" alt="Logo"
    style="width:20%">
</div>
<div class="card">
    <h2 class="card-title">Canvas Clock</h2>
    <p class="card-description">Customizable canvas for gif and images.</p>
    <img class="card-image" src="https://github.com/robegamesios/PlexMatrixDisplay/blob/main/thumbnails/canvas_clock.jpg?raw=true" alt="Canvas Clock">
    <a class="update-button" onclick="updatePreference('selectedTheme', 0)">Select</a>
</div>
<div class="card">
    <h2 class="card-title">Mario Bros Clock</h2>
    <p class="card-description">Mario jumps and bumps the block every minute.</p>
    <img class="card-image" src="https://github.com/robegamesios/PlexMatrixDisplay/blob/main/thumbnails/mario_bros_clock.jpg?raw=true" alt="Mario Bros Clock">
    <a class="update-button" onclick="updatePreference('selectedTheme', 1)">Select</a>
</div>
<div class="card">
    <h2 class="card-title">Pacman Clock</h2>
    <p class="card-description">Pacman runs around eating pellets.</p>
    <img class="card-image" src="https://github.com/robegamesios/PlexMatrixDisplay/blob/main/thumbnails/pacman_clock.jpg?raw=true" alt="Pacman Clock">
    <a class="update-button" onclick="updatePreference('selectedTheme', 2)">Select</a>
</div>

  <div class="w3-row-padding w3-padding">
    <div id="base" class="w3-col s3 m3 s12" style="display: none;">
      <div class="w3-card-4 w3-margin-bottom">
        <header class="w3-container w3-blue-gray">
          <h4 id="title">{{TITLE}}</h4>
        </header>
        <div class="w3-container">
          <p style="min-height: 45px;" id="description">{{DESCRIPTION}}</p>
          <hr>
          <div class="w3-row w3-section">
            <div class="w3-col" style="width:50px"><i id="icon" class="w3-xxlarge w3-text-dark-grey fa"></i>
            </div>
            <div class="w3-rest" id="formInput">
              {{FORM_INPUT}}
            </div>
          </div>
        </div>
        <button id="cardButton" class="w3-button w3-block w3-light-blue">Save</button>
      </div>
    </div>
  </div>
  <script>
    function createCards(settings) {
      console.log(settings);
      const cards = [
        {
          title: "Display Bright",
          description: "0 = dark (display off) / 255 = super bright | Value: <strong><output id='rangevalue'>" + settings.displaybright + "</output></strong>",
          formInput: "<input class='w3-input w3-border' type='range' min='0' max='255' value='" + settings.displaybright + "' class='slider' id='bright' oninput='rangevalue.value=value'>",
          icon: "fa-adjust",
          save: "updatePreference('displayBright', bright.value)",
          property: "displayBright"
        },
        {
          title: "Use 24h format?",
          description: "Changes the hour format to show 20:00 instead of 8:00PM",
          formInput: "<input class='w3-check' type='checkbox' id='use24h' " + (settings.use24hformat == '1' ? "checked" : "") + "><label for='use24h'> Yep</label>",
          icon: "fa-clock-o",
          save: "updatePreference('use24hFormat', Number(use24h.checked))",
          property: "use24hFormat"
        },
        {
          title: "Swap Blue/Green pins?",
          description: "Swap Blue and Green pins because the panel is RBG instead of RGB",
          formInput: "<input class='w3-check' type='checkbox' id='swapBG' " + (settings.swapbluegreen == '1' ? "checked" : "") + "><label for='swapBG'> Yep</label>",
          icon: "fa-random",
          save: "updatePreference('swapBlueGreen', Number(swapBG.checked))",
          property: "swapBlueGreen"
        },
        {
          title: "Timezone",
          description: "Consult your TZ identifier <a href='https://en.wikipedia.org/wiki/List_of_tz_database_time_zones'>here.</a> Examples: America/Sao_Paulo, Europe/Lisbon",
          formInput: "<input id='tz' class='w3-input w3-light-grey' name='tz' type='text' placeholder='Timezone' value='" + settings.timezone + "'>",
          icon: "fa-globe",
          save: "updatePreference('timeZone', tz.value)",
          property: "timeZone"
        },
        {
          title: "NTP Server",
          description: "Configure your prefered NTP Server. You can use one of the <a href='https://www.ntppool.org'>NTP Pool Project</a> pools or a local one.",
          formInput: "<input id='ntp' class='w3-input w3-light-grey' name='ntp' type='text' placeholder='NTP Server' value='" + settings.ntpserver + "'>",
          icon: "fa-server",
          save: "updatePreference('ntpServer', ntp.value)",
          property: "ntpServer"
        },
        {
          title: "Automatic Bright",
          description: "Inform the values read by the LDR when the room is dark (min value) and bright (max value). Range 0 - 4095",
          formInput: "<input id='autoBrightMin' class='w3-input w3-light-grey w3-cell w3-margin-right' name='autoBrightMin' style='width:45%;' type='number' min='0' max='4095' placeholder='Min value' value='" + settings.autobrightmin + "'>" + 
                     "<input id='autoBrightMax' class='w3-input w3-light-grey w3-cell' name='autoBrightMax' style='width:45%;' type='number' min='0' max='4095' placeholder='Max value' value='" + settings.autobrightmax + "'>",
          icon: "fa-sun-o",
          save: "updatePreference('autoBright', autoBrightMin.value.padStart(4, '0') + ',' + autoBrightMax.value.padStart(4, '0'))",
          property: "autoBright"
        },
        {
          title: "LDR Pin",
          description: "The GPIO pin where the LDR is connected to. Use just the numeric value (default: 35) | <a href='#' onclick='readPin(ldrPin.value);'>Read Pin: </a><strong id='ldrPinRead'>0</strong>",
          formInput: "<input id='ldrPin' class='w3-input w3-light-grey' name='ldrPin' type='number' min='0' max='39' value='" + settings.ldrpin + "'>",
          icon: "fa-microchip",
          save: "updatePreference('ldrPin', ldrPin.value)",
          property: "ldrPin"
        },
        {
          title: "[Canvas] Description file",
          description: "Name of the description file to be rendered without extension.",
          formInput: "<input id='descFile' class='w3-input w3-light-grey' name='descFile' type='text' placeholder='Description File' value='" + settings.canvasfile + "'>",
          icon: "fa-file-image-o",
          save: "updatePreference('canvasFile', descFile.value)",
          property: "canvasFile",
          exclusive: "cw-cf-0x07"
        },
        {
          title: "[Canvas] Server Address",
          description: "Server address where the description files are located. Change this to test it locally.",
          formInput: "<input id='serverAddress' class='w3-input w3-light-grey' name='serverAddress' type='text' placeholder='Canvas Server' value='" + settings.canvasserver + "'>",
          icon: "fa-server",
          save: "updatePreference('canvasServer', serverAddress.value)",
          property: "canvasServer",
          exclusive: "cw-cf-0x07"
        },
        {
          title: "Posix Timezone String",
          description: "To avoid remote lookups, provide a Posix string that corresponds to your timezone. Leave empty to obtain this automatically from the server. <a href=\"https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv\">Click here for a list.</a>",
          formInput: "<input id='posixString' class='w3-input w3-light-grey' name='posixString' type='text' placeholder='Manual Posix String' value='" + settings.manualposix + "'>",
          icon: "fa-globe",
          save: "updatePreference('manualPosix', posixString.value)",
          property: "manualPosix"
        }
      ];

      var base = document.querySelector('#base');
      cards.forEach(c => {

        if (!c.hasOwnProperty('exclusive') || (c.hasOwnProperty('exclusive') && c.exclusive === settings.clockface_name)) {
          var clone = base.cloneNode(true);
          clone.id = c.property + "-card";
          clone.removeAttribute("style");

          Array.prototype.slice.call(clone.getElementsByTagName('*')).forEach(e => {
            e.id = e.id + "-" + c.property;
          });

          base.before(clone);
          document.getElementById("title-" + c.property).innerHTML = c.title
          document.getElementById("description-" + c.property).innerHTML = c.description
          document.getElementById("formInput-" + c.property).innerHTML = c.formInput
          document.getElementById("icon-" + c.property).classList.add(c.icon);
          document.getElementById("cardButton-" + c.property).setAttribute("onclick", c.save);
        }
      })

      document.getElementById("ssid").innerHTML = "<i class='fa fa-wifi'></i> " + settings.wifissid
      document.getElementById("fw-version").innerHTML = "<i class='fa fa-code-fork'></i> Firmware v" + settings.cw_fw_version
    }

    // Function to fetch options from GitHub JSON file
    function fetchGifOptions() {
        fetch('https://raw.githubusercontent.com/robegamesios/PlexMatrixDisplay/main/shared/gifArtOptions.json')
        .then(response => response.json())
        .then(data => {
            const gifOptionSelect = document.getElementById('gifOptionSelect');
            data.forEach(option => {
                const optionElement = document.createElement('option');
                optionElement.value = option.value;
                optionElement.title = option.title;
                optionElement.textContent = option.label;
                gifOptionSelect.appendChild(optionElement);
            });
        })
        .catch(error => {
            console.error('Error fetching options:', error);
        });
    }

    function uploadGifArt() {
      var selectElement = document.getElementById('gifOptionSelect');
      var selectedOption = selectElement.options[selectElement.selectedIndex];
      var title = selectedOption.title;

      const xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status >= 200 && this.status < 299) {
          document.getElementById('status').style.display = 'block';
        }
      };
      xhr.open('POST', '/set?' + "gifArtName" + '=' + title);
      xhr.send();

      setTimeout(() => {
        document.getElementById('status').style.display = 'none';
      }, 2000);
    }

    document.getElementById('optionSelect').addEventListener('change', function() {
        var selectedValue = this.value;
        var selectedOption = this.options[this.selectedIndex];
        var title = selectedOption.title;

        const xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function () {
          if (this.readyState == 4 && this.status >= 200 && this.status < 299) {
            document.getElementById('status').style.display = 'block';
          }
        };
        xhr.open('POST', '/set?avPattern' + '=' + selectedValue);
        xhr.send();

        setTimeout(() => {
          document.getElementById('status').style.display = 'none';
        }, 2000);        
    });

    function updatePlexCredentials() {
      // Get the values from the input fields
      var serverAddress = document.getElementById("serverAddress").value;
      var serverPort = document.getElementById("serverPort").value;
      var authToken = document.getElementById("authToken").value;

      var payload = serverAddress + "," + serverPort + "," + authToken;

      const xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status >= 200 && this.status < 299) {
          document.getElementById('status').style.display = 'block';
        }
      };
      xhr.open('POST', '/set?' + "plexCredentials" + '=' + payload);
      xhr.send();

      setTimeout(() => {
        document.getElementById('status').style.display = 'none';
      }, 2000);
    }

    function updatePreference(key, value) {
      const xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status >= 200 && this.status < 299) {
          document.getElementById('status').style.display = 'block';
        }
      };
      xhr.open('POST', '/set?' + key + '=' + value);
      xhr.send();

      setTimeout(() => {
        document.getElementById('status').style.display = 'none';
      }, 2000);
    }

    function requestGet(path, cb) {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function () {
        if (this.readyState === 2 && this.status === 204) {
          cb(this);
        }
      };
      xmlhttp.open("GET", path, true);
      xmlhttp.send();
    }
    
    function restartDevice() {
      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/restart');
      xhr.send();
    }

    function resetWifi() {
      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/reset-wifi');
      xhr.send();
    }

    // Call fetchOptions() function when the page loads
    window.onload = function() {
        fetchGifOptions();
    };

  </script>
</body>
</html>
)"""";
