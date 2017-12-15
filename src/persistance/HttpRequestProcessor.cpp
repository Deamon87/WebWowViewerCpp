//
// Created by deamon on 24.10.17.
//

#include "HttpRequestProcessor.h"

void HttpRequestProcessor::requestFile(const char *fileName) {
    std::string fileName_s(fileName);
    this->addRequest(fileName_s);
}

void HttpRequestProcessor::processFileRequest(std::string &fileName) {
    std::string fullUrl;
    if (fileName.find("FILE") == 0) {
        fullUrl = m_urlBaseFileId + fileName;
    } else {
        fullUrl = m_urlBase + fileName;
    }

    HttpFile * httpFile = new HttpFile(fullUrl.c_str());
    httpFile->setCallback(
            [=](std::vector<unsigned char> * fileContent) -> void {
                provideResult(fileName, *fileContent);

                //m_fileRequester->provideFile(fileName.c_str(), &((*fileContent)[0]), fileContent->size());

                delete httpFile;
            }
    );
    httpFile->startDownloading();
}
