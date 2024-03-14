#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <Arduino.h> // Include necessary libraries if needed

class HelperFunctions {
public:
    static std::string capitalizeString(const char *str);
    static std::string toUpperCase(const char *str);
    static const char *strrstr(const char *haystack, const char *needle);
    static const char *extractStringValue(const char *jsonString, const char *fieldName, bool findLast = false, const char *defaultValue = nullptr);
    static float extractFloatValue(const char *jsonString, const char *fieldName, float defaultValue = 0.0f);
    static void floatToChar(float floatValue, char *charArray, int bufferSize);
    static String decodeHtmlEntities(String text);
    static String urlEncode(const char *msg);
};

#endif
