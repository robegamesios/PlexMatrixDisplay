#include "HelperFunctions.h"

std::string HelperFunctions::capitalizeString(const char *str) {
    std::string capitalizedStr;
    if (str != nullptr && strlen(str) > 0) {
        capitalizedStr += std::toupper(str[0]);
        capitalizedStr += &str[1]; // Append the rest of the string
    }
    return capitalizedStr;
}

std::string HelperFunctions::toUpperCase(const char *str) {
    std::string upperStr;
    if (str != nullptr) {
        size_t length = strlen(str);
        for (size_t i = 0; i < length; ++i) {
            upperStr += std::toupper(str[i]);
        }
    }
    return upperStr;
}

const char *HelperFunctions::strrstr(const char *haystack, const char *needle) {
    if (!*needle)
        return haystack;
    const char *result = nullptr;
    const char *p;
    const char *q;
    for (; *haystack; ++haystack) {
        if (*haystack == needle[0]) {
            p = haystack;
            q = needle;
            while (*p && *q && *p == *q) {
                ++p;
                ++q;
            }
            if (!*q) {
                result = haystack;
            }
        }
    }
    return result;
}

const char *HelperFunctions::extractStringValue(const char *jsonString, const char *fieldName, bool findLast, const char *defaultValue) {
    const char *fieldStart = nullptr;

    // Find the start index of the field
    if (findLast) {
        fieldStart = strrstr(jsonString, fieldName);
    } else {
        fieldStart = strstr(jsonString, fieldName);
    }

    if (fieldStart == nullptr) {
        printf("Error: Unable to find field '%s'.\n", fieldName);
        return defaultValue;
    }

    // Move to the actual value
    fieldStart += strlen(fieldName) + 2; // Account for the field name and 1 colon and 1 quote: "\"fieldName\":"

    // Find the end index of the field value
    const char *fieldValueEnd = strchr(fieldStart, '"');
    if (fieldValueEnd == nullptr) {
        printf("Error: Invalid JSON format for field '%s'.\n", fieldName);
        return defaultValue;
    }

    // Calculate the length of the field value
    int fieldValueLength = fieldValueEnd - fieldStart;

    // Allocate memory for the field value string
    char *fieldValue = (char *)malloc(fieldValueLength + 1);
    if (fieldValue == nullptr) {
        // Handle memory allocation failure
        printf("Error: Failed to allocate memory.\n");
        return defaultValue;
    }

    // Copy the value
    strncpy(fieldValue, fieldStart, fieldValueLength);
    fieldValue[fieldValueLength] = '\0';

    return fieldValue;
}

float HelperFunctions::extractFloatValue(const char *jsonString, const char *fieldName, float defaultValue) {
    // Find the start index of the field
    const char *fieldStart = strstr(jsonString, fieldName);
    if (fieldStart == nullptr) {
        printf("Error: Unable to find field '%s'.\n", fieldName);
        return defaultValue;
    }

    // Move to the actual value
    fieldStart += strlen(fieldName) + 1; // Account for the field name and colon: "\"fieldName\":"

    // Find the end index of the field value
    float extractedValue;
    if (sscanf(fieldStart, "%f", &extractedValue) != 1) {
        printf("Error: Unable to extract float value for field '%s'.\n", fieldName);
        return defaultValue;
    }

    return extractedValue;
}

void HelperFunctions::floatToChar(float floatValue, char *charArray, int bufferSize) {
    snprintf(charArray, bufferSize, "%.0f", floatValue);
}

String HelperFunctions::decodeHtmlEntities(String text) {
    text.replace("&quot;", "\"");
    text.replace("&amp;", "&");
    text.replace("&apos;", "'");
    text.replace("&lt;", "<");
    text.replace("&gt;", ">");
    text.replace("&#8217;", "'");
    text.replace("&#8216;", "'");
    text.replace("&#39;", "'");
    // Add more replacements as needed

    return text;
}

String HelperFunctions::urlEncode(const char *msg) {
    const char *hex = "0123456789ABCDEF";
    String encodedMsg = "";

    while (*msg != '\0') {
        if (('a' <= *msg && *msg <= 'z') ||
            ('A' <= *msg && *msg <= 'Z') ||
            ('0' <= *msg && *msg <= '9') ||
            *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }

    return encodedMsg;
}
