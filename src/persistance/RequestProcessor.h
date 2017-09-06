//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#include <zip.h>
#include <wowScene.h>
#include "httpFile/httpFile.h"


using namespace std::placeholders;

class ZipHttpRequestProcessor : public IFileRequest {
public:
    ZipHttpRequestProcessor (const char *url) {
        //std::string url = "http://deamon87.github.io/WoWFiles/ironforge.zip";


        HttpFile httpFile(url);
        httpFile.setCallback(std::bind(&ZipHttpRequestProcessor::loadingFinished, this, _1));
        httpFile.startDownloading();
    }
private:
//    zipper::Unzipper *m_unzipper;
    zip_t *zipArchive;
    IFileRequester *m_fileRequester;

public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
    void loadingFinished(std::vector<unsigned char> * file);
    void requestFile(const char* fileName) ;
};

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
