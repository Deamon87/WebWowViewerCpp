//
// Created by Deamon on 11/22/2017.
//

#include <algorithm>
#include "CascRequestProcessor.h"

void CascRequestProcessor::requestFile(const char *fileName) {
    std::string fileName_s(fileName);
    this->addRequest(fileName_s);
}

void CascRequestProcessor::processFileRequest(std::string &fileName) {
    std::string fileNameFixed = fileName;
    std::replace( fileNameFixed.begin(), fileNameFixed.end(), '/', '\\');

    HANDLE fileHandle;
    std::vector<unsigned char> fileContent;
    bool fileOpened = false;
    if (CascOpenFile(m_storage, fileNameFixed.c_str(), 0,  0, &fileHandle)) {
        DWORD fileSize1 = CascGetFileSize(fileHandle, 0);
        fileOpened = true;
        fileContent = std::vector<unsigned char> (fileSize1+1);

        DWORD totalBytesRead = 0;
        while (true) {
            DWORD dwBytesRead;

            CascReadFile(fileHandle, &fileContent[totalBytesRead], fileSize1-totalBytesRead, &dwBytesRead);

            if(dwBytesRead == 0) {
                break;
            }

            totalBytesRead += dwBytesRead;
        }


    }

    if (fileOpened) {
        this->provideResult(fileName, fileContent);
    } else {
        std::cout << "Could not open file "<< fileName << std::endl << std::flush;
    }
}
