#include "httpFile.h"
#include <iostream>
#include <iomanip>


std::string url_encode(const std::string &value) {
    std::ostringstream escaped;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/' || c == ':') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << std::hex << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

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
//    CURL * curl = curl_easy_init();
//    this->curlInstance = curl;
//
//
//    if (curl) {
//        curl_easy_cleanup(curl);
//        curl = curl_easy_init();
//        std::string escaped_url = url_encode(m_httpUrl);
//
//        curl_easy_setopt(curl, CURLOPT_URL, escaped_url.c_str());
//
//        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
//        curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
//        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5000);
//        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
//        CURLcode res = curl_easy_perform(curl);
//
//        long httpCode;
//        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpCode);
//
//        if (httpCode == 200 && res == CURLE_OK) {
//            if (this->m_fileBuffer->size() == 0) {
//                std::cout << "File "<< this->m_httpUrl.c_str() << " is empty" << std::endl <<
//                    escaped_url << std::endl << std::flush;
//            } else if (this->m_callback != nullptr) {
//                m_callback(this->m_fileBuffer);
//            }
//        } else {
//            std::cout << "Could not download file "<<this->m_httpUrl.c_str() << std::endl <<
//                escaped_url << std::endl << std::flush;
//        }
//        /* always cleanup */
//        curl_easy_cleanup(curl);
//    }

//    std::string escaped_url = url_encode(m_httpUrl);
    std::string escaped_url = m_httpUrl;

    auto r = cpr::Get(cpr::Url{escaped_url}, cpr::VerifySsl{false});
    if (r.status_code == 200) {
        if (this->m_fileBuffer != nullptr) delete(this->m_fileBuffer);
        this->m_fileBuffer = new std::vector<unsigned char>(r.text.begin(), r.text.end());
        if (this->m_fileBuffer->size() == 0) {
            std::cout << "File " << this->m_httpUrl.c_str() << " is empty" << std::endl <<
                      escaped_url << std::endl << std::flush;
        } else if (this->m_callback != nullptr) {
            m_callback(this->m_fileBuffer);
        }
    } else {
        std::cout << "Could not download file " << this->m_httpUrl.c_str() << std::endl <<
                  escaped_url << std::endl << std::flush;
    }
}