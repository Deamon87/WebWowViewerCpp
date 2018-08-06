//
// Created by Deamon on 3/25/2018.
//

#ifndef WEBWOWVIEWERCPP_LOG_H
#define WEBWOWVIEWERCPP_LOG_H

#include <streambuf>

#ifdef __ANDROID_API__
#include <android/log.h>
#include <android/api-level.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

extern AAssetManager *g_assetMgr;

class androidbuf : public std::streambuf {
public:
    enum { bufsize = 1024 }; // ... or some other suitable buffer size
    androidbuf() { this->setp(buffer, buffer + bufsize - 1); }

private:
    void printBufferContent() {
        if (this->pbase() != this->pptr()) {
            char writebuf[bufsize+1];
            memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
            writebuf[this->pptr() - this->pbase()] = '\0';
            std::string temp(writebuf, writebuf+(this->pptr() - this->pbase()));
            temp = "WoWRenderLog: " + temp;
            __android_log_write(ANDROID_LOG_INFO, "std", temp.c_str()) > 0;
            this->setp(buffer, buffer + bufsize - 1);
        }

    };

    int overflow(int c)
    {
        printBufferContent();
        //print s
        if (c == traits_type::eof()) {
            *this->pptr() = traits_type::to_char_type(c);
            this->sbumpc();
        }
        return this->sync()? traits_type::eof(): traits_type::not_eof(c);
    }

    int sync()
    {
        int rc = 0;
        if (this->pbase() != this->pptr()) {
            printBufferContent();
            this->setp(buffer, buffer + bufsize - 1);
        }
        return rc;
    }

    char buffer[bufsize];
};
#endif



#endif //WEBWOWVIEWERCPP_LOG_H
