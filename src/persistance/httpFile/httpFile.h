#ifndef WOWMAPVIEWERREVIVED_HTTPFILE_H
#define WOWMAPVIEWERREVIVED_HTTPFILE_H

#include <curl/curl.h>
#include <string>
#include <vector>

class HttpFile;
typedef void (*HTTPReadyCallback)(HttpFile*);

class HttpFile {

public:
    HttpFile(std::string *httpUrl){
        this->m_httpUrl = httpUrl;
        this->written = 0;
        this->m_callback = nullptr;
        this->m_fileBuffer = new std::vector<unsigned char>();
    }

public:
    void startDownloading();
    void setCallback(HTTPReadyCallback callback);
    std::vector<unsigned char> * getFileBuffer() {
        return m_fileBuffer;
    }
    void writeToBuffer(void *buffer, long int size);

private:
    std::string *m_httpUrl;
    HTTPReadyCallback m_callback;
    CURL *curlInstance;

    long int fileSize;
    long int written;
    std::vector<unsigned char> *m_fileBuffer;


};


#endif //WOWMAPVIEWERREVIVED_HTTPFILE_H
