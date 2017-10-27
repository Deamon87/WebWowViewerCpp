//
// Created by deamon on 23.10.17.
//

#include <iostream>
#include <algorithm>
#include "MpqRequestProcessor.h"

void MpqRequestProcessor::requestFile(const char *fileName) {
    std::string stringFile (fileName);
    std::replace( stringFile.begin(), stringFile.end(), '/', '\\');

    for (int i = 0; i < m_MPQArchiveHandles.size(); i++) {
        HANDLE fileHandle;
        if (SFileOpenFileEx(m_MPQArchiveHandles[i], stringFile.c_str(), 0, &fileHandle)) {

            DWORD fileSize = SFileGetFileSize(fileHandle,0);
            std::vector<unsigned char> m_buffer(fileSize);

            DWORD actualRead = 0;
            SFileReadFile(fileHandle, &m_buffer[0], fileSize, &actualRead, nullptr);

            m_fileRequester->provideFile(fileName, &m_buffer[0], actualRead);

            return;
        }
        //std::cout << "LastError = " << GetLastError() << std::endl;
    }

    std::cout << "Could not load file " << std::string(fileName) << std::endl << std::flush;
    m_fileRequester->rejectFile(fileName);

}

