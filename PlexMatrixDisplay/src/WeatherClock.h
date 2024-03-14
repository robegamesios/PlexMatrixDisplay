
#include <Arduino.h>
#include <SPIFFS.h>

#define WM_WEATHER_CITY_NAME_LABEL "weatherCityName"
#define WM_WEATHER_COUNTRY_CODE_LABEL "weatherCountryCode"
#define WM_WEATHER_API_KEY_LABEL "weatherApiKey"
#define WM_WEATHER_UNIT_LABEL "weatherUnit"
#define WEATHER_CONFIG_JSON "/weather_config.json"

char weatherCityName[32];
char weatherCountryCode[6];
char weatherApikey[64];
char weatherUnit[2] = "0";

#ifdef WEATHERCLOCK_MODE

#include <cstring>
#include <string>
#include <time.h>
#include <ArduinoJson.h>
#include "HelperFunctions.h"
#include <jRead.h>

// ******************************************* BEGIN CLOCK ****************************************************
#pragma region CLOCK

String getLocalTimeFromUnix(float unixTime)
{
    time_t unixTime_t = static_cast<time_t>(unixTime); // Convert int to time_t
    struct tm *timeinfo = localtime(&unixTime_t);      // Convert time_t to struct tm
    if (timeinfo == nullptr)
    {
        return ""; // Error handling if conversion fails
    }

    char buffer[20];

    if (strcmp(weatherUnit, "0") == 0) // imperial
    {
        strftime(buffer, sizeof(buffer), "%l:%M%P", timeinfo);
    }
    else
    {
        strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
    }
    String localTime = String(buffer);
    return localTime;
}

String getLocalTimeAndDate()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
#ifdef DEBUG
        Serial.println("Failed to obtain time");
#endif

        return "";
    }
    char buffer[20];

    if (strcmp(weatherUnit, "0") == 0) // imperial
    {
        strftime(buffer, sizeof(buffer), "%l:%M%P  %b %d", &timeinfo);
    }
    else
    {
        strftime(buffer, sizeof(buffer), "%H:%M  %b %d", &timeinfo);
    }

    String localTime = String(buffer);
    return localTime;
}

#pragma endregion
// ******************************************* END CLOCK ******************************************************

// ******************************************* BEGIN WEATHER **************************************************
#pragma region WEATHER

struct WeatherData
{
    std::string weatherIcon;
    std::string cityName;
    std::string temperature;
    std::string feelsLike;
    std::string humidity;
    std::string tempMin;
    std::string tempMax;
    std::string extraInfo;

    WeatherData(std::string icon, std::string city, std::string temp, std::string feels, std::string hum, std::string min, std::string max, std::string extra)
        : weatherIcon(icon), cityName(city), temperature(temp), feelsLike(feels), humidity(hum), tempMin(min), tempMax(max), extraInfo(extra) {}
};

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

WeatherData processWeatherJson(const char *pJson)
{
    char str[128]; // general use string buffer
    jRead_string(pJson, "{'timezone'", str, 128);
    String timezone = str;
    configTime(timezone.toFloat(), 0, "pool.ntp.org"); // DST offset is set to zero, no need since timezone accounts for it.

    std::string tempUnit = "";
    std::string pressureUnit = "hPa";
    std::string windUnit = "";

    if (strcmp(weatherUnit, "1") == 0)
    {
        // metric
        tempUnit = "C";
        windUnit = "m/s";
    }
    else if (strcmp(weatherUnit, "2") == 0)
    {
        // standard
        tempUnit = "K";
        windUnit = "m/s";
    }
    else
    {
        // imperial
        tempUnit = "F";
        windUnit = "MPH";
    }

    jRead_string(pJson, "{'weather'[{'description'", str, 128);
    std::string uppercasedDescription = HelperFunctions::toUpperCase(str);

    jRead_string(pJson, "{'weather'[{'icon'", str, 128);
    std::string icon = str;
    
    float temp = jRead_float(pJson, "{'main'{'temp'");
    int tempInt = (int)temp;
    std::string tempString = std::to_string(tempInt);

    float feelsLike = jRead_float(pJson, "{'main'{'feels_like'");
    int feelsLikeInt = (int)feelsLike;
    std::string feelsLikeString = "FL: " + std::to_string(feelsLikeInt) + tempUnit;

    float temp_min = jRead_float(pJson, "{'main'{'temp_min'");
    // printf("Temperature min: %.2f\n", temp_min);
    int tempMinInt = (int)temp_min;
    std::string tempMinString = "L: " + std::to_string(tempMinInt) + tempUnit;

    float temp_max = jRead_float(pJson, "{'main'{'temp_max'");
    // printf("Temperature max: %.2f\n", temp_max);
    int tempMaxInt = (int)temp_max;
    std::string tempMaxString = "H: " + std::to_string(tempMaxInt) + tempUnit;

    float pressure = jRead_float(pJson, "{'main'{'pressure'");
    // printf("Pressure: %.2f\n", pressure);
    int pressureInt = (int)pressure;
    std::string pressureString = "P: " + std::to_string(pressureInt) + pressureUnit;

    float humidity = jRead_float(pJson, "{'main'{'humidity'");
    // printf("Humidity: %.2f\n", humidity);
    int humidityInt = (int)humidity;
    std::string humidityString = "RH: " + std::to_string(humidityInt) + "%";

    float wind_direction = jRead_float(pJson, "{'wind'{'deg'");
    // printf("Wind direction: %.2f\n", wind_direction);
    int windDirectionInt = (int)wind_direction;

    float wind_speed = jRead_float(pJson, "{'wind'{'speed'");
    // printf("Wind speed: %.2f\n", wind_speed);
    int windSpeedInt = (int)wind_speed;
    std::string windString = "Wind: " + degreesToDirection(windDirectionInt) + " " + std::to_string(windSpeedInt) + windUnit;

    float sunrise = jRead_float(pJson, "{'sys'{'sunrise'");
    // printf("sunrise: %.2f\n", sunrise);
    std::string sunriseValue = getLocalTimeFromUnix(sunrise).c_str();
    std::string sunriseString = "Sunrise: " + sunriseValue;

    float sunset = jRead_float(pJson, "{'sys'{'sunset'");
    // printf("sunset: %.2f\n", sunset);
    std::string sunsetValue = getLocalTimeFromUnix(sunset).c_str();
    std::string sunsetString = "Sunset: " + sunsetValue;

    std::string extraInfo = uppercasedDescription + "    " + sunriseString + "    " + sunsetString + "    " + pressureString + "    " + windString;

    // display the info to LED Matrix
    String cleanCityName = String(weatherCityName);
    cleanCityName.replace("%20", " ");
    std::string uppercasedCityName = HelperFunctions::toUpperCase(cleanCityName.c_str());

    WeatherData weatherData(icon, uppercasedCityName, tempString, feelsLikeString, humidityString, tempMinString, tempMaxString, extraInfo);

    return weatherData;
}

boolean fetchWeatherConfigFile()
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
                    return true;
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
    return false;
}

#endif

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

#pragma endregion
// ******************************************* END WEATHER ****************************************************
