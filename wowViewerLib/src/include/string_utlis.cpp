#include "string_utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

void tokenize(std::string const &str, const std::string delim,
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

static std::string url_char_to_escape( char i )
{
    std::stringstream stream;
    stream << "%"
           << std::setfill ('0') << std::setw(2)
           << std::hex << (int)i;
    return stream.str();
}

std::string toLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

template<typename T>
std::string int_to_hex(T i) {
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}