//
// Created by Deamon on 11/22/2017.
//

#ifndef WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H

#include "../../wowViewerLib/src/persistence/RequestProcessor.h"
#include "StorageError.h"

struct BuildDefinition;
#include <iostream>

const std::string CASC_KEYS_FILE = "WoW.txt";

class CascRequestProcessor : public RequestProcessor {
public:
    CascRequestProcessor(const std::string &path, const BuildDefinition &buildDef, StorageErrorCallback errorCallback = {});
    ~CascRequestProcessor() override;

    void updateKeys();
    bool isOpen() const { return m_isOpen; }
private:
    StorageErrorCallback m_errorCallback;
    bool m_isOpen = false;
    std::string m_cascDir = "";

    std::function<HFileContent(const std::string &fileName, uint32_t fileDataId)> readFileContentLambda;
    std::function<void(
        std::function<bool (int fileDataId, const std::string &fileName)> &process,
        std::function<void (int fileDataId, const HFileContent &fileData)> &callback
    )> iterateFilesLambda;

protected:
    void processFileRequest(CacheHolderType holderType, const std::weak_ptr<PersistentFile> &s_file) override;
    void iterateFilesInternal(
        std::function<bool (int fileDataId, const std::string &fileName)> &process,
        std::function<void (int fileDataId, const HFileContent &fileData)> &callback) override;
private:
    HFileContent tryGetFileFromOverrides(int fileDataId);

    HFileContent readFileContent(const std::string &fileName, uint32_t fileDataId);
};


#endif //WEBWOWVIEWERCPP_CASCREQUESTPROCESSOR_H
