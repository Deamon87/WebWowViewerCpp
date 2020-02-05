//
// Created by Deamon on 11/22/2017.
//

#include <algorithm>
#include <sstream>
#include "CascRequestProcessor.h"

void CascRequestProcessor::requestFile(const char *fileName, CacheHolderType holderType) {
    std::string fileName_s(fileName);
    this->addRequest(fileName_s,holderType);
}

void CascRequestProcessor::processFileRequest(std::string &fileName, CacheHolderType holderType) {
    std::string fileNameFixed = fileName;
    std::replace( fileNameFixed.begin(), fileNameFixed.end(), '/', '\\');

    void *fileNameToPass = (void *) fileNameFixed.c_str();
    DWORD openFlags = CASC_OPEN_BY_NAME;
    if (fileNameFixed.find("File") == 0) {
        std::stringstream ss;
        std::string fileDataIdHex = fileNameFixed.substr(4, fileNameFixed.find(".")-4);
        uint32_t fileDataId;
        ss << std::hex << fileDataIdHex;
        ss >> fileDataId;

        if (fileDataId == 0) {
            return;
        }

        fileNameToPass = reinterpret_cast<void *>(fileDataId);
        openFlags = CASC_OPEN_BY_FILEID;
    }

    HANDLE fileHandle;
    HFileContent fileContent;
    bool fileOpened = false;
    if (CascOpenFile(m_storage, fileNameToPass, 0, openFlags, &fileHandle)) {
        DWORD fileSize1 = CascGetFileSize(fileHandle, nullptr);
        if (fileSize1 != CASC_INVALID_SIZE) {
            fileOpened = true;
            fileContent = std::make_shared<FileContent>(FileContent(fileSize1 + 1));
            auto dataPtr = fileContent->data();

            DWORD totalBytesRead = 0;
            while (totalBytesRead < fileSize1) {
                DWORD dwBytesRead = 0;


                CascReadFile(fileHandle, &dataPtr[totalBytesRead], fileSize1 - totalBytesRead, &dwBytesRead);

                totalBytesRead += dwBytesRead;
            }
        }
    }

    if (fileOpened) {
        this->provideResult(fileName, fileContent, holderType);
    } else {
        std::cout << "Could not open file "<< fileName << std::endl << std::flush;
    }
}
