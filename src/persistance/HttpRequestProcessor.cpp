//
// Created by deamon on 24.10.17.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include "HttpRequestProcessor.h"
#include "../../wowViewerLib/src/include/sharedFile.h"

static const std::string cacheDirectory = "./../cache/";

template< typename T >
std::string int_to_hex( T i )
{
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

void HttpRequestProcessor::requestFile(const char *fileName, CacheHolderType holderType) {
    std::string fileName_s(fileName);
    this->addRequest(fileName_s, holderType);
}
std::string char_to_escape( char i )
{
    std::stringstream stream;
    stream << "%"
           << std::setfill ('0') << std::setw(2)
           << std::hex << (int)i;
    return stream.str();
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

    const std::string charsToEscape = " !*'();:@&=+$,/?#[]";

    std::string escapedFileName = fileName;
    for (int i = 0; i < charsToEscape.size(); i++) {
        char c = charsToEscape[i];
        escapedFileName = ReplaceAll(escapedFileName, std::string(1, c), char_to_escape(c));
    }


    std::string fullUrl;
    if (fileName.find("File") == 0) {
        std::stringstream ss;
        std::string fileDataIdHex = fileName.substr(4, fileName.find(".")-4);
        uint32_t fileDataId;
        ss << std::hex << fileDataIdHex;
        ss >> fileDataId;

        fullUrl = m_urlBaseFileId + std::to_string(fileDataId);
    } else {
        fullUrl = m_urlBase + escapedFileName;
    }

    size_t hash = std::hash<std::string>{}(fileName);
    std::string inputFileName = cacheDirectory + int_to_hex(hash);

    std::ifstream cache_file(inputFileName, std::ios::in |std::ios::binary);
    if (cache_file.good()) {
        cache_file.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;

        cache_file.seekg(0, std::ios::end);
        fileSize = cache_file.tellg();
        cache_file.seekg(0, std::ios::beg);


        HFileContent vec = std::make_shared<FileContent>(fileSize);
		cache_file.read((char *)&(*vec.get())[0], fileSize);

        if (holderType != CacheHolderType::CACHE_ANIM && vec->size() > 4 && (*(uint32_t *)vec->data() == 0)) {
            std::cout << "Encountered encrypted file in cache " << inputFileName << " initial name " << fileName << std::endl;
            return;
        }

		//vec->reserve(fileSize);

		// read the data:
		//vec->insert(vec->begin(),
		//	std::istream_iterator<unsigned char>(cache_file),
		//	std::istream_iterator<unsigned char>());
//        HFileContent vec = HFileContent(new FileContent());
//        vec->reserve(fileSize);
//
//        // read the data:
//        std::copy(std::istream_iterator<unsigned char>(cache_file),
//                  std::istream_iterator<unsigned char>(),
//                  std::back_inserter(*vec.get()));

//        vec.reserve(fileSize);

        // read the data:
//        vec.insert(vec.begin(),
//                   std::istream_iterator<unsigned char>(cache_file),
//                   std::istream_iterator<unsigned char>());

//        std::copy(std::istream_iterator<unsigned char>(cache_file),
//                  std::istream_iterator<unsigned char>(),
//                  std::back_inserter(vec));

        provideResult(fileName, vec, holderType);

        return;
    }
//
    HttpFile * httpFile = new HttpFile(fullUrl);
    httpFile->setCallback(
            [fileName, this, holderType, httpFile, fullUrl](HFileContent fileContent) -> void {
                std::string newFileName = fileName;

                if (holderType != CacheHolderType::CACHE_ANIM && fileContent->size() > 4 && (*(uint32_t *)fileContent->data() == 0)) {
                    std::cout << "Encountered encrypted file " << fullUrl << std::endl;
                    return;
                }

                //Write to cache
                size_t hash = std::hash<std::string>{}(newFileName);
                std::string outputFileName = cacheDirectory + int_to_hex(hash);

                std::ofstream FILE(outputFileName, std::ios::out | std::ios::binary);
                std::copy(fileContent->begin(), fileContent->end(), std::ostreambuf_iterator<char>(FILE));
                FILE.flush();
                FILE.close();


                //Provide file!
                provideResult(newFileName, fileContent, holderType);
                //std::ofstream output_file(outputFileName, std::ios::out | std::ofstream::binary);
				//std::ostreambuf_iterator<unsigned char> output_iterator(output_file);


				//std::copy(fileContent->begin(), fileContent->end(), output_iterator);
            }
    );
    httpFile->setFailCallback([fileName, this, holderType, httpFile](HFileContent fileContent) -> void {
        this->m_fileRequester->rejectFile(holderType, fileName.c_str());
    });
    httpFile->startDownloading();

    delete httpFile;
}
