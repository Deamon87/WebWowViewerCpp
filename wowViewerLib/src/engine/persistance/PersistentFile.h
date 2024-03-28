#ifndef AWEBWOWVIEWERCPP_PERSISTENTFILE_H
#define AWEBWOWVIEWERCPP_PERSISTENTFILE_H

#include <string>
#include "../../include/sharedFile.h"
enum class FileStatus {
    FSNotLoaded,
    FSLoaded,
    FSRejected
};

class PersistentFile {
public:
    virtual void process(HFileContent fileContent, const std::string &fileName) = 0;
    FileStatus getStatus() {
        return fsStatus;
    };
    void setRejected() {
        fsStatus = FileStatus::FSRejected;
    };
protected:
    FileStatus fsStatus = FileStatus::FSNotLoaded;
};

#endif //AWEBWOWVIEWERCPP_PERSISTENTFILE_H
