//
// Created by Deamon on 10/9/2017.
//

#ifndef WEBWOWVIEWERCPP_HTTPZIPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPZIPREQUESTPROCESSOR_H

#include "httpFile/httpFile.h"
#include "ZipRequestProcessor.h"

class HttpZipRequestProcessor : public ZipRequestProcessor {
public:
    HttpZipRequestProcessor (const char *url) {
        m_httpFile = new HttpFile(url);
        m_httpFile->setCallback(std::bind(&ZipRequestProcessor::loadingFinished, this, _1));
        m_httpFile->startDownloading();
    }

    ~HttpZipRequestProcessor() {
      if (m_httpFile != nullptr) {
          delete m_httpFile;
          m_httpFile = nullptr;
      }
    };

private:
    HttpFile *m_httpFile = nullptr;
};


#endif //WEBWOWVIEWERCPP_HTTPZIPREQUESTPROCESSOR_H
