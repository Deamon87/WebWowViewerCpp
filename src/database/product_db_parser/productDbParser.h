//
// Created by Deamon on 9/4/2022.
//

#ifndef AWEBWOWVIEWERCPP_PRODUCTDBPARSER_H
#define AWEBWOWVIEWERCPP_PRODUCTDBPARSER_H

#include <string>
#include <vector>
#include "../buildInfoParser/buildDefinition.h"

class ProductDbParser {
public:
    static SetOfBuildDefs parseFileContent(std::string &data);
};


#endif //AWEBWOWVIEWERCPP_PRODUCTDBPARSER_H
