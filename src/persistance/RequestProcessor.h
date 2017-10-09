//
// Created by deamon on 06.09.17.
//

#ifndef WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
#define WEBWOWVIEWERCPP_REQUESTPROCESSOR_H

#include <wowScene.h>
#include <thread>

class RequestProcessor : public IFileRequest {

    RequestProcessor() {
//        auto receiver = [](int count, PolyM::Queue& q) {
//            while (true) {
//                auto msg = q.get();
//                auto &dm = dynamic_cast<PolyM::DataMsg<int> &>(*msg);
//
//            }
//        };
//
//        loaderThread = new std::thread()
    }

private:
    std::thread *loaderThread;
};

#endif //WEBWOWVIEWERCPP_REQUESTPROCESSOR_H
