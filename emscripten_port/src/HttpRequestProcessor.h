//
// Created by deamon on 24.10.17.
//

#ifndef WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H

#include <emscripten/fetch.h>
#include "RequestProcessor.h"

class HttpRequestProcessor : public RequestProcessor {
public:
    HttpRequestProcessor(const char *urlBase, const char *urlBaseFileId) : m_urlBase(urlBase), m_urlBaseFileId(urlBaseFileId){

    }
private:
    std::string m_urlBase;
    std::string m_urlBaseFileId;
protected:
    void processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) override;

public:
    friend void downloadFailed(emscripten_fetch_t *fetch);
    friend void downloadSucceeded(emscripten_fetch_t *fetch);

    void setUrls(const char *urlBase, const char *urlBaseFileId) {
        m_urlBase = std::string(urlBase);
        m_urlBaseFileId  = std::string(urlBaseFileId);
    }
};


#endif //WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
