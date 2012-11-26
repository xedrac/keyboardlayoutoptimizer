#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

namespace util
{

// Return text but without leading and trailing whitespace
std::string trim(const std::string &text)
{
    size_t a = text.find_first_not_of(" \t\n\r");
    size_t b = text.find_last_not_of(" \t\n\r");
    if (a == std::string::npos)
        return "";
    return text.substr(a, b-a+1);
}


std::vector<std::string> split(const std::string &text, const char delimiter=' ', bool keep_empty=false)
{
    std::vector<std::string> tokens;
    size_t prev = 0;
    size_t next = 0;  
    while ((next = text.find_first_of(delimiter, prev)) != std::string::npos) {
        if (next-prev > 0 || keep_empty) {
            tokens.push_back(text.substr(prev, next-prev)); 
        }
        prev = next + 1;
    }

    if (text.length())
        tokens.push_back(text.substr(prev, text.length()-prev));

    return tokens;
}

}

#endif

