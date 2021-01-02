//
// Created by Deamon on 11/22/2017.
//

#include <algorithm>
#include <sstream>
#include "CascRequestProcessor.h"

void CascRequestProcessor::processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) {
    auto perstFile = s_file.lock();
    if (perstFile == nullptr) {
        uint32_t fileDataId = 0;
        if (fileName.find("File") == 0) {
            std::stringstream ss;
            std::string fileDataIdHex = fileName.substr(4, fileName.find(".") - 4);

            ss << std::hex << fileDataIdHex;
            ss >> fileDataId;
        }
        std::cout << "perstFile for " << fileName << "(fileDataId = "<<fileDataId<<" ) is expired" << std::endl;
        toBeProcessed--;
        return;
    }

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


                if (!CascReadFile(fileHandle, &dataPtr[totalBytesRead], fileSize1 - totalBytesRead, &dwBytesRead)) {
                    std::cout << "Could read from file "<< fileName << std::endl << std::flush;
                    toBeProcessed--;
                    return;
                }

                totalBytesRead += dwBytesRead;
            }
        }
    }

    if (fileOpened) {
        toBeProcessed--;
        processResult(perstFile, fileContent, fileName);

//        this->provideResult(fileName, fileContent, holderType);
    } else {
        toBeProcessed--;
        std::cout << "Could not open file "<< fileName << std::endl << std::flush;
        this->m_fileRequester->rejectFile(holderType, fileName.c_str());
    }
}
