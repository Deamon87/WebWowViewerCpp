//
// Created by deamon on 23.10.17.
//

#ifndef WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H

#include <vector>
#include "RequestProcessor.h"
#include "../../wowViewerLib/src/include/wowScene.h"
#include "../../3rdparty/stormlib/src/StormLib.h"

class MpqRequestProcessor : public IFileRequest{

public:
    void loadingFinished(std::vector<unsigned char> * file);
    void requestFile(const char* fileName) override;

    MpqRequestProcessor(const char * wowDistPath) {

        HANDLE hmpq;
        SFileOpenArchive("", 0, 0, &hmpq);
    }

private:
    IFileRequester *m_fileRequester = nullptr;

public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
};


#endif //WEBWOWVIEWERCPP_MPQREQUESTPROCESSOR_H
