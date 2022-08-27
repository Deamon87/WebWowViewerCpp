//
// Created by Deamon on 8/24/2022.
//

#ifndef AWEBWOWVIEWERCPP_DB2FILE_H
#define AWEBWOWVIEWERCPP_DB2FILE_H

#include <string>
#include "PersistentFile.h"

class Db2File : public PersistentFile {
public:
    Db2File(std::string fileName){};
    Db2File(int fileDataId){};

    void process(HFileContent db2File, const std::string &fileName) override;
private:
    HFileContent db2Content;
};

#endif //AWEBWOWVIEWERCPP_DB2FILE_H
