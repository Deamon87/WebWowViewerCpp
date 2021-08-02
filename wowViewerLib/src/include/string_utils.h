//
// Created by Deamon on 8/2/2021.
//

#ifndef AWEBWOWVIEWERCPP_STRING_UTILS_H
#define AWEBWOWVIEWERCPP_STRING_UTILS_H

#include <string>
#include <vector>

bool endsWith(std::string_view str, std::string_view suffix);
bool endsWith2(std::string_view str, std::string_view suffix);

bool startsWith(std::string_view str, std::string_view prefix);

void tokenize(std::string const &str, const std::string delim,
              std::vector<std::string> &out);


#endif //AWEBWOWVIEWERCPP_STRING_UTILS_H
