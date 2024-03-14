#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <Arduino.h> // Include necessary libraries if needed

class HelperFunctions {
public:
    static std::string capitalizeString(const char *str);
    static std::string toUpperCase(const char *str);
    static String decodeHtmlEntities(String text);
};

#endif
