#include "httpFile.h"
#include <iostream>




size_t write_data(void *ptr, size_t size, size_t nmemb, HttpFile *httpFile) {
    httpFile->writeToBuffer(ptr, size*nmemb);
    return nmemb*size;
}

void HttpFile::writeToBuffer(void *buffer, long int size){
    for (int i = 0; i < size; i++) {
        unsigned char byte = ((unsigned char *) buffer)[i];
        this->m_fileBuffer->push_back(byte);
    }

}

void HttpFile::setCallback(HTTPReadyCallback callback) {
    this->m_callback = callback;
}

void HttpFile::startDownloading() {
    CURL * curl = curl_easy_init();
    this->curlInstance = curl;


    if (curl) {
        curl_easy_cleanup(curl);
        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, this->m_httpUrl->c_str());

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5000);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            if (this->m_callback != nullptr) {
                m_callback(this->m_fileBuffer);
            }
        } else {
            std::cout << "Could not download file "<<this->m_httpUrl->c_str() << std::flush;
        }
        /* always cleanup */
       curl_easy_cleanup(curl);
    }
}