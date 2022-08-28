//
// Created by Deamon on 8/24/2022.
//

#include "db2File.h"

void Db2File::process(HFileContent db2File, const std::string &fileName) {
    db2Content = db2File;
    fsStatus = FileStatus::FSLoaded;
}
