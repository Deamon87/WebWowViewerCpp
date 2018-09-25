//
// Created by deamon on 24.10.17.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "HttpRequestProcessor.h"
#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);


    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
}

template< typename T >
std::string int_to_hex( T i )
{
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

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

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    emscripten_fetch(&attr, "myfile.dat");

//    //
//    HttpFile * httpFile = new HttpFile(fullUrl.c_str());
//    httpFile->setCallback(
//            [=](std::vector<unsigned char> * fileContent) -> void {
//
//                delete httpFile;
//            }
//    );
//    httpFile->startDownloading();
}
