//
// Created by deamon on 24.10.17.
//

#include "HttpRequestProcessor.h"

void HttpRequestProcessor::requestFile(const char *fileName) {
    std::string fullUrl = m_urlBase + fileName;

    HttpFile * httpFile = new HttpFile(fullUrl.c_str());
    httpFile->setCallback(
        [&](std::vector<unsigned char> * fileContent) -> void {
            m_fileRequester->provideFile(fileName, &((*fileContent)[0]), fileContent->size());

            delete httpFile;
        }
    );
    httpFile->startDownloading();
}
