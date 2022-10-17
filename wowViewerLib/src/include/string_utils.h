//
// Created by Deamon on 8/2/2021.
//

#ifndef AWEBWOWVIEWERCPP_STRING_UTILS_H
#define AWEBWOWVIEWERCPP_STRING_UTILS_H

#include <string>
#include <vector>
#include <algorithm>

static inline bool startsWith(const std::string_view &str, const std::string_view &prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}
static inline bool endsWith(const std::string_view &str, const std::string_view &suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}
static inline bool contains(const std::string_view &str, const std::string_view &subString) {
    return str.find(subString) != std::string::npos;
}

static inline void tokenize(std::string const &str, const std::string delim,
              std::vector<std::string> &out)
{
    size_t prev_start = 0;
    size_t start = 0;

    while ((start = str.find(delim, start)) != std::string::npos)
    {
        out.push_back(str.substr(prev_start, start-prev_start));
        prev_start = ++start;
    }
    out.push_back(str.substr(prev_start, start-prev_start));
}


template< typename T >
static inline std::string int_to_hex( T i );
static std::string url_char_to_escape( char i );

static inline std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

static inline std::string toLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}
#endif //AWEBWOWVIEWERCPP_STRING_UTILS_H
