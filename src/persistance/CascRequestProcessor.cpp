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

    CascLib::HANDLE fileHandle;
    std::vector<unsigned char> fileContent;
    if (CascLib::CascOpenFile(m_storage, fileNameFixed.c_str(), 0,  0, &fileHandle)) {
        CascLib::DWORD fileSize1 = CascLib::CascGetFileSize(fileHandle, 0);

        fileContent = std::vector<unsigned char> (fileSize1+1);

        CascLib::DWORD totalBytesRead = 0;
        while (true) {
            CascLib::DWORD dwBytesRead;

            CascLib::CascReadFile(fileHandle, &fileContent[totalBytesRead], fileSize1-totalBytesRead, &dwBytesRead);

            if(dwBytesRead == 0) {
                break;
            }

            totalBytesRead += dwBytesRead;
        }


    }

    this->provideResult(fileName, fileContent);
}
