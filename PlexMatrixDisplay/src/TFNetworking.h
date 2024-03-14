// ******************************************* BEGIN NETWORKING ***********************************************
#pragma region NETWORKING

#include <Arduino.h>
#include <HTTPClient.h>

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
