//
// Created by deamon on 26.06.17.
//

#ifndef WEBWOWVIEWERCPP_STRINGTRIM_H
#define WEBWOWVIEWERCPP_STRINGTRIM_H
// trim from start (in place)
int isprint ( int c );
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::iscntrl))));
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
};

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::iscntrl))).base(), s.end());
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
};

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
};

// trim from start (copying)
static inline std::string ltrimmed(std::string s) {
    ltrim(s);
    return s;
};

// trim from end (copying)
static inline std::string rtrimmed(std::string s) {
    rtrim(s);
    return s;
};

// trim from both ends (copying)
static inline std::string trimmed(std::string s) {
    trim(s);
    return s;
};

static inline std::string string_to_hex(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}
#endif //WEBWOWVIEWERCPP_STRINGTRIM_H
