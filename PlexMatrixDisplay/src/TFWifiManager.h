// ******************************************* BEGIN WIFI *****************************************************
#pragma region WIFI

#include <WiFi.h>
#include <WiFiManager.h>

WiFiManager wifiManager;

std::vector<const char *> _menu = {"wifi", "exit"};

boolean wifiConnect()
{
  bool resp;

  wifiManager.setTitle("TUNEFRAME Wifi Setup");
  wifiManager.setMenu(_menu);

  resp = wifiManager.autoConnect("TUNEFRAME Wifi Setup");

  if (!resp)
  {
    Serial.println("Failed to connect");
    return false;
  }
  Serial.println("connected!");
  return true;
}

boolean isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

#pragma endregion
// ******************************************* END WIFI *******************************************************
