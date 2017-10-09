//
// Created by Deamon on 10/9/2017.
//

#ifndef WEBWOWVIEWERCPP_HTTPZIPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_HTTPZIPREQUESTPROCESSOR_H


#include "ZipRequestProcessor.h"

class HttpZipRequestProcessor : public ZipRequestProcessor {
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
