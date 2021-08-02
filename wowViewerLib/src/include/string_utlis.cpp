#include "string_utils.h"

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
