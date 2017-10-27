//
// Created by deamon on 24.10.17.
//

#ifndef WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H

#include "httpFile/httpFile.h"
#include "../../wowViewerLib/src/include/wowScene.h"
#include "RequestProcessor.h"

class HttpRequestProcessor : public RequestProcessor {
public:
    HttpRequestProcessor(const char *urlBase) : m_urlBase(urlBase){

    }
private:
    std::string m_urlBase;
protected:
    void processFileRequest(std::string &fileName) override;

public:
    void requestFile(const char* fileName) override;
};


#endif //WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
