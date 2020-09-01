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
    httplib::Client cli(m_httpUrl, 80);

    httplib::ContentReceiver contentReceiver = [](const char *data, size_t data_length) -> bool {
        return true;
    };

    auto res = cli.Get("/hi");

//    if (res) {
        std::cout << res->status << std::endl;
        std::cout << res->body << std::endl;
//    }
}

void HttpFile::setFailCallback(HTTPReadyCallback callback) {
    m_failCallback = callback;
}
