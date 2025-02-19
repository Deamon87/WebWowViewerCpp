//
// Created by deamon on 24.10.17.
//

#ifndef WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H

#include "httpFile/httpFile.h"
#include "../../wowViewerLib/src/persistence/RequestProcessor.h"

class HttpRequestProcessor : public RequestProcessor {
public:
    HttpRequestProcessor(const char *urlBase, const char *urlBaseFileId) : m_urlBase(urlBase), m_urlBaseFileId(urlBaseFileId){

    }
private:
    std::string m_urlBase;
    std::string m_urlBaseFileId;
protected:
    void processFileRequest(const std::string &fileName, CacheHolderType holderType, const std::weak_ptr<PersistentFile> &s_file) override;
};


#endif //WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
