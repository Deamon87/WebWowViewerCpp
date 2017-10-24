//
// Created by deamon on 24.10.17.
//

#ifndef WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H

#include "httpFile/httpFile.h"
#include "../../wowViewerLib/src/include/wowScene.h"

class HttpRequestProcessor : public IFileRequest{
public:
    HttpRequestProcessor(const char *urlBase) : m_urlBase(urlBase){

    }
private:
    IFileRequester *m_fileRequester = nullptr;
    std::string m_urlBase;
public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
public:
    void requestFile(const char* fileName) override;
};


#endif //WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
