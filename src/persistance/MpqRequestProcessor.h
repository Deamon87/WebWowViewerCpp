//
// Created by deamon on 23.10.17.
//

#ifndef WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H

#include <vector>
#include "RequestProcessor.h"
#include "../../wowViewerLib/src/include/wowScene.h"
#include "../../3rdparty/stormlib/src/StormLib.h"

static std::string mpqPriorityOrder[] = {
    "patch-3.MPQ",
    "patch-2.MPQ",
    "patch.MPQ",
    "enGB\\patch-enGB-2.MPQ",
    "enGB\\patch-enGB.MPQ",
    "lichking.MPQ",
    "expansion2.MPQ",
    "expansion.MPQ",
    "common-3.MPQ",
    "common-2.MPQ",
    "common.MPQ",
    "enGB\\locale-enGB.MPQ"
};

class MpqRequestProcessor : public IFileRequest{

public:
    void requestFile(const char* fileName, CacheHolderType holderType) override;

    explicit MpqRequestProcessor(const char * wowDistPath) {
        HANDLE hMPQFileCurr ;
        std::string tmpString;
        std::string pathToDist(wowDistPath);

        int acrhiveCnt = sizeof(mpqPriorityOrder)  / sizeof(std::string);
        for (int i = 0; i < acrhiveCnt; i++) {
            hMPQFileCurr = 0;
            tmpString  =  pathToDist +  mpqPriorityOrder[i];

            SFileOpenArchive(tmpString.c_str(), 0, STREAM_FLAG_READ_ONLY, &hMPQFileCurr);
            if (hMPQFileCurr != nullptr) {
                m_MPQArchiveHandles.push_back(hMPQFileCurr);
            }
        }
    }

private:
    IFileRequester *m_fileRequester = nullptr;
    std::vector<HANDLE> m_MPQArchiveHandles;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
};


#endif //WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H
