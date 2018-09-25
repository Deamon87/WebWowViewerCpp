//
// Created by deamon on 24.10.17.
//

#ifndef WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H

#include "../../wowViewerLib/src/include/wowScene.h"
#include "RequestProcessor.h"

class HttpRequestProcessor : public RequestProcessor {
public:
    HttpRequestProcessor(const char *urlBase, const char *urlBaseFileId) : m_urlBase(urlBase), m_urlBaseFileId(urlBaseFileId){

    }
private:
    std::string m_urlBase;
    std::string m_urlBaseFileId;
protected:
    void processFileRequest(std::string &fileName) override;

public:
    void requestFile(const char* fileName) override;
};


#endif //WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
