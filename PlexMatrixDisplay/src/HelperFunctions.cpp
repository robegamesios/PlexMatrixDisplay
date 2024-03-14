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
