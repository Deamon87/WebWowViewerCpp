//
// Created by Deamon on 8/2/2021.
//

#ifndef AWEBWOWVIEWERCPP_STRING_UTILS_H
#define AWEBWOWVIEWERCPP_STRING_UTILS_H

#include <string>
#include <vector>

bool endsWith(std::string_view str, std::string_view suffix);

bool startsWith(std::string_view str, std::string_view prefix);

void tokenize(std::string const &str, const std::string delim,
              std::vector<std::string> &out);


template< typename T >
static inline std::string int_to_hex( T i );
static std::string url_char_to_escape( char i );
static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);
std::string toLowerCase(std::string s);
#endif //AWEBWOWVIEWERCPP_STRING_UTILS_H
