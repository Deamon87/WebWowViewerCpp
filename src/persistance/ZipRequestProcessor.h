//
// Created by Deamon on 10/9/2017.
//

#ifndef WEBWOWVIEWERCPP_ZIPHTTPREQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_ZIPHTTPREQUESTPROCESSOR_H


//#include <zip.h>
#include <ios>
#include <fstream>
#include <regex>
#include <functional>
#include <vector>
#include "../../wowViewerLib/src/include/iostuff.h"


using namespace std::placeholders;

class ZipRequestProcessor : public IFileRequest {
public:
    ZipRequestProcessor(){};

    explicit ZipRequestProcessor (const char *archivePath) {
        std::ifstream input( archivePath, std::ios::binary );
        m_file = std::vector<unsigned char>(
            (std::istreambuf_iterator<char>(input)),
            (std::istreambuf_iterator<char>())
        );

        this->loadingFinished(&m_file);
    }
private:
    //zip_t *zipArchive = nullptr;
    std::vector<unsigned char> m_file;
    IFileRequester *m_fileRequester = nullptr;

public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
    void loadingFinished(std::vector<unsigned char> * file);
    void requestFile(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) override;
};


#endif //WEBWOWVIEWERCPP_ZIPHTTPREQUESTPROCESSOR_H
