//
// Created by deamon on 24.10.17.
//
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "HttpRequestProcessor.h"
#include <emscripten/fetch.h>

struct UserDataForRequest {
    std::string fileName;
    CacheHolderType holderType;
    HttpRequestProcessor *processor;
};

void downloadSucceeded(emscripten_fetch_t *fetch) {
//    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
//
//    try {
//        std::cout << "Creating FileContent with " << fetch->numBytes << " bytes" << std::endl;
        HFileContent fileContent = std::make_shared<FileContent>(fetch->numBytes);
//        std::cout << "FileContent was created" << std::endl;
        std::copy(fetch->data, fetch->data + fetch->numBytes, fileContent->begin());


//    HFileContent fileContent(&fetch->data[0], &fetch->data[fetch->numBytes]);
        UserDataForRequest *userDataForRequest = (UserDataForRequest *) fetch->userData;
        fetch->userData = nullptr;

        if (userDataForRequest->holderType != CacheHolderType::CACHE_ANIM && fileContent->size() > 4 &&
            (*(uint32_t *) fileContent->data() == 0)) {
            std::cout << "Encountered encrypted file " << std::string(fetch->url) << std::endl;
            return;
        }


        userDataForRequest->processor->provideResult(userDataForRequest->fileName, fileContent,
                                                     userDataForRequest->holderType);
        userDataForRequest->processor->currentlyProcessing--;



        // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
        delete userDataForRequest;
        emscripten_fetch_close(fetch); // Free data associated with the fetch.
//    } catch (...) {
//        printf("Exception on Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
//    }

}

void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    UserDataForRequest * userDataForRequest = (UserDataForRequest *)fetch->userData;
    userDataForRequest->processor->currentlyProcessing--;

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
std::string char_to_escape( char i )
{
    std::stringstream stream;
    stream << "%"
           << std::setfill ('0') << std::setw(2)
           << std::hex << (int)i;
    return stream.str();
}

void HttpRequestProcessor::requestFile(const char *fileName, CacheHolderType holderType) {
    std::string fileName_s(fileName);
    this->addRequest(fileName_s, holderType);
}


std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void HttpRequestProcessor::processFileRequest(std::string &fileName, CacheHolderType holderType) {
//    std::cout << "processFileRequest : filename = " << fileName << std::endl;

    const std::string charsToEscape = " !*'();:@&=+$,/?#[]";

    std::string escapedFileName = fileName;
    for (int i = 0; i < charsToEscape.size(); i++) {
        char c = charsToEscape[i];
        escapedFileName = ReplaceAll(escapedFileName, std::string(1, c), char_to_escape(c));
    }

    std::string fullUrl;
    if (escapedFileName.find("File") == 0) {
        std::stringstream ss;
        std::string fileDataIdHex = escapedFileName.substr(4, escapedFileName.find(".")-4);
        uint32_t fileDataId;
        ss << std::hex << fileDataIdHex;
        ss >> fileDataId;

        if (fileDataId == 0) {
            currentlyProcessing--;
            return;
        }

        fullUrl = m_urlBaseFileId + std::to_string(fileDataId);
    } else {
        fullUrl = m_urlBase + escapedFileName;
    }

//    fullUrl = urlEncode(fullUrl);

    UserDataForRequest * userDataForRequest = new UserDataForRequest();
    userDataForRequest->fileName = fileName;
    userDataForRequest->holderType = holderType;
    userDataForRequest->processor = this;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    attr.userData = userDataForRequest;
    emscripten_fetch(&attr, fullUrl.c_str());

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
