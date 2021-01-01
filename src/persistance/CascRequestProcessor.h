//
// Created by Deamon on 11/22/2017.
//

#ifndef WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H

#include "RequestProcessor.h"
#include <iostream>

    #include "../../3rdparty/casclib/src/CascLib.h"


class CascRequestProcessor : public RequestProcessor {
public:
    CascRequestProcessor(const char *path) : m_cascDir(path){
        if (CascOpenStorage(path, 0xFFFFFFFF, &this->m_storage)) {
            std::cout << "Opened CascStorage at "<< path << std::endl;
        } else {
            std::cout << "Could not open CascStorage at "<< path << std::endl;
            throw  "Could not open CascStorage";
        }
    }
private:
    std::string m_cascDir;

    void* m_storage;
protected:
    void processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) override;
};


#endif //WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H
