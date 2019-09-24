//
// Created by deamon on 30.07.18.
//

#ifndef AWEBWOWVIEWERCPP_GLOBALTHREADS_H
#define AWEBWOWVIEWERCPP_GLOBALTHREADS_H

//Strictly debug class
#include <thread>

//#ifdef DODEBUGTHREADS
class GlobalThreads {
public:
    std::thread cullingThread;
    std::thread updateThread;
    std::thread renderThread;
    std::thread loadingResourcesThread;
};

extern GlobalThreads g_globalThreadsSingleton;
//#endif

#endif //AWEBWOWVIEWERCPP_GLOBALTHREADS_H
