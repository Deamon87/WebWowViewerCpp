#include "commonFileStructs.h"
#include <string>
//
// Created by deamon on 23.06.17.
//
template<>
std::string M2Array<char>::toString() {
    char * ptr = this->getElement(0);
    return std::string(ptr, ptr+size);
}
