#ifndef WOWMAPVIEWERREVIVED_HTTPFILE_H
#define WOWMAPVIEWERREVIVED_HTTPFILE_H

#include <string>
#include <vector>
#include <functional>
//curl
#include "cpr/cpr.h"

class HttpFile;

typedef std::function<void (std::vector<unsigned char>*)> HTTPReadyCallback ;

class HttpFile {

public:
    HttpFile(std::string httpUrl){
        this->m_httpUrl = httpUrl;
        this->written = 0;
//        this->m_callback = nullptr;
        this->m_fileBuffer = new std::vector<unsigned char>();
    }

    ~HttpFile(){
        if (this->m_fileBuffer != nullptr) {
            delete this->m_fileBuffer;
        }
    }

public:
    void startDownloading();
    void setCallback(HTTPReadyCallback callback);
    std::vector<unsigned char> * getFileBuffer() {
        return m_fileBuffer;
    }
    void writeToBuffer(void *buffer, long int size);

private:
    std::string m_httpUrl;
    HTTPReadyCallback m_callback;

    long int fileSize;
    long int written;
    std::vector<unsigned char> *m_fileBuffer = nullptr;


};


#endif //WOWMAPVIEWERREVIVED_HTTPFILE_H
