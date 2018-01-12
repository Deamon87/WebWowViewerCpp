//
// Created by deamon on 24.10.17.
//

#include <iostream>
#include <sstream>
#include "HttpRequestProcessor.h"

void HttpRequestProcessor::requestFile(const char *fileName) {
    std::string fileName_s(fileName);
    this->addRequest(fileName_s);
}

void HttpRequestProcessor::processFileRequest(std::string &fileName) {
    std::string fullUrl;
    if (fileName.find("File") == 0) {
        std::stringstream ss;
        std::string fileDataIdHex = fileName.substr(4, fileName.find(".")-4);
        uint32_t fileDataId;
        ss << std::hex << fileDataIdHex;
        ss >> fileDataId;

        fullUrl = m_urlBaseFileId + std::to_string(fileDataId);
    } else {
        fullUrl = m_urlBase + fileName;
    }

//    std::cout << "fullUrl = " << fullUrl << std::endl;

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
