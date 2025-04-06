//
// Created by Deamon on 7/7/2023.
//

#ifndef AWEBWOWVIEWERCPP_COMMANDBUFFERTHREAD_H
#define AWEBWOWVIEWERCPP_COMMANDBUFFERTHREAD_H

#include <thread>

class CommandBufferThread {
public:
    
private:
    bool m_isTerminating;
    std::thread m_commandBufferThread;
};


#endif //AWEBWOWVIEWERCPP_COMMANDBUFFERTHREAD_H
