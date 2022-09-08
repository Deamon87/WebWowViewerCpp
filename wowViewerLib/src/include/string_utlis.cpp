#include "string_utils.h"
#include <sstream>
#include <iomanip>








std::string url_char_to_escape( char i )
{
    std::stringstream stream;
    stream << "%"
           << std::setfill ('0') << std::setw(2)
           << std::hex << (int)i;
    return stream.str();
}



template<typename T>
inline std::string int_to_hex(T i) {
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

