//
// Created by deamon on 24.10.17.
//

#ifndef WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H

#include <../../../../src/persistance/RequestProcessor.h>

class HttpRequestProcessorAndroid : public RequestProcessor {
public:
    HttpRequestProcessorAndroid(const char *urlBase, const char *urlBaseFileId, android_app* app ) {

        m_urlBase = urlBase;
        m_urlBaseFileId = urlBaseFileId;
        m_app = app;
    }
private:
    std::string m_urlBase;
    std::string m_urlBaseFileId;
    android_app* m_app = nullptr;
protected:
    void processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) override;

public:
    friend void downloadFailed(JNIEnv *, jobject, jlong lp);
    friend void downloadSucceeded(JNIEnv *env, jobject, jlong userDataPtr, jbyteArray array);

    void setUrls(const char *urlBase, const char *urlBaseFileId) {
        m_urlBase = std::string(urlBase);
        m_urlBaseFileId  = std::string(urlBaseFileId);
    }
};


#endif //WEBWOWVIEWERCPP_HTTPREQUESTPROCESSOR_H
