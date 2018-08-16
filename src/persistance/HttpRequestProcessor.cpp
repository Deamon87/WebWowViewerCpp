//
// Created by deamon on 24.10.17.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "HttpRequestProcessor.h"

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

    size_t hash = std::hash<std::string>{}(fileName);
    std::string inputFileName = "./cache/" + int_to_hex(hash);

    std::ifstream cache_file(inputFileName, std::ios::binary);
    if (cache_file.good()) {
        cache_file.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;

        cache_file.seekg(0, std::ios::end);
        fileSize = cache_file.tellg();
        cache_file.seekg(0, std::ios::beg);

        std::vector<unsigned char> vec;
        vec.reserve(fileSize);

        // read the data:
        vec.insert(vec.begin(),
                   std::istream_iterator<unsigned char>(cache_file),
                   std::istream_iterator<unsigned char>());

        provideResult(fileName, vec);

        return;
    }
//
    HttpFile * httpFile = new HttpFile(fullUrl.c_str());
    httpFile->setCallback(
            [=](std::vector<unsigned char> * fileContent) -> void {
                std::string newFileName = fileName;
                provideResult(newFileName, *fileContent);

                //Write to cache
                size_t hash = std::hash<std::string>{}(newFileName);
                std::string outputFileName = "./cache/" + int_to_hex(hash);
                std::ofstream output_file(outputFileName, std::ios::out | std::ios::binary);
                std::ostream_iterator<unsigned char> output_iterator(output_file);
                std::copy(fileContent->begin(), fileContent->end(), output_iterator);

                delete httpFile;
            }
    );
    httpFile->startDownloading();
}
