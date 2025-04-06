//
// Created by Deamon on 9/4/2022.
//

#ifndef AWEBWOWVIEWERCPP_BUILDINFOPARSER_H
#define AWEBWOWVIEWERCPP_BUILDINFOPARSER_H

#include <string>
#include "buildDefinition.h"

class BuildInfoParser {
public:
    static SetOfBuildDefs parseFileContent(std::string &data);
};

#endif //AWEBWOWVIEWERCPP_BUILDINFOPARSER_H
