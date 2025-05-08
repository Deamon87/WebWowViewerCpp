//
// Created for managing thread-local frame processing state
//

#ifndef AWEBWOWVIEWERCPP_FRAMECONTEXT_H
#define AWEBWOWVIEWERCPP_FRAMECONTEXT_H

class FrameContext {
public:
    static unsigned int getCurrentProcessingFrameNumber();
    static void setCurrentProcessingFrameNumber(unsigned int frameNumber);
};

#endif //AWEBWOWVIEWERCPP_FRAMECONTEXT_H

