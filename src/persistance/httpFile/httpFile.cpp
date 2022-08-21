#include "httpFile.h"
#ifndef __ANDROID__
#include "cpr/cpr.h"
#endif
#include <iostream>
#include <iomanip>
#include <sstream>


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
    std::string escaped_url = m_httpUrl;
#ifndef __ANDROID__
    auto verSSL = cpr::VerifySsl{false};

    auto r = cpr::Get(cpr::Url{escaped_url}, verSSL );
    if (r.status_code == 200) {
        this->m_fileBuffer = std::make_shared<FileContent>(FileContent(r.text.begin(), r.text.end()));



        if (this->m_fileBuffer->size() == 0) {
            std::cout << "File " << this->m_httpUrl.c_str() << " is empty" << std::endl <<
                      escaped_url << std::endl << std::flush;
            m_failCallback({});
        } else if (this->m_callback != nullptr) {
            m_callback(this->m_fileBuffer);
        }
    } else {
        std::cout << "Could not download file " << this->m_httpUrl.c_str() << std::endl <<
                  escaped_url << std::endl << std::flush;
        m_failCallback({});
    }
#else
#endif
}

void HttpFile::setFailCallback(HTTPReadyCallback callback) {
    m_failCallback = callback;
}
