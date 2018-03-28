//
// Created by Deamon on 3/25/2018.
//

#ifndef WEBWOWVIEWERCPP_LOG_H
#define WEBWOWVIEWERCPP_LOG_H

#include <streambuf>
#ifdef __ANDROID__
# include <android/api-level.h>
#endif

#ifdef __ANDROID_API__
#include <android/log.h>

class androidbuf : public std::streambuf {
public:
    enum { bufsize = 128 }; // ... or some other suitable buffer size
    androidbuf() { this->setp(buffer, buffer + bufsize - 1); }

private:
    int overflow(int c)
    {
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
            char writebuf[bufsize+1];
            memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
            writebuf[this->pptr() - this->pbase()] = '\0';

            rc = __android_log_write(ANDROID_LOG_INFO, "std", writebuf) > 0;
            this->setp(buffer, buffer + bufsize - 1);
        }
        return rc;
    }

    char buffer[bufsize];
};
#endif

#endif //WEBWOWVIEWERCPP_LOG_H
