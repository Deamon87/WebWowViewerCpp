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
    "patch-2.MPQ",
    "patch.MPQ",
    "enGB\\patch-enGB-2.MPQ"
    "enGB\\patch-enGB.MPQ",
    "expansion3.MPQ",
    "expansion2.MPQ",
    "expansion.MPQ",
    "common.MPQ",
    "common-2.MPQ",
    "common-3.MPQ",
    "enGB\\locale-enGB.MPQ"
};

class MpqRequestProcessor : public IFileRequest{

public:
    void requestFile(const char* fileName) override;

    MpqRequestProcessor(const char * wowDistPath) {

        HANDLE hmpq;
        SFileOpenArchive("", 0, 0, &hmpq);
    }

    void GetMPQHandles(std::string &pathToDist) {
        int CurrentArchive;
        HANDLE hMPQFileCurr ;
        std::string tmpString;

        std::vector<HANDLE> MPQArchiveHandle;
        for (int i = 0; i < sizeof(mpqPriorityOrder); i++) {
            hMPQFileCurr = 0;
            tmpString  =  pathToDist +  mpqPriorityOrder[CurrentArchive];

            SFileOpenArchive(tmpString.c_str(),0,0, &hMPQFileCurr);
            if (hMPQFileCurr > 0) {
                MPQArchiveHandle.push_back(hMPQFileCurr);
            }
        }
    }

private:
    IFileRequester *m_fileRequester = nullptr;

public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
};


#endif //WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H
