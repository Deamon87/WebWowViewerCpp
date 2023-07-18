//
// Created by Deamon on 11/22/2017.
//

#ifndef WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H

#include "RequestProcessor.h"
#include <iostream>

#include "fileBrowser/buildDefinition.h"

const std::string CASC_KEYS_FILE = "KnownCascKeys.txt";

class CascRequestProcessor : public RequestProcessor {
public:
    CascRequestProcessor(std::string &path, BuildDefinition &buildDef);
    ~CascRequestProcessor() override;

    void updateKeys();
private:
    std::string m_cascDir = "";

    void* m_storage = nullptr;
    void* m_storageOnline = nullptr;
protected:
    void processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) override;
private:
    HFileContent tryGetFile(void *cascStorage, void *fileNameToPass, uint32_t openFlags);
};


#endif //WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H
