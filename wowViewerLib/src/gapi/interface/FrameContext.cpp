//
// Created for managing thread-local frame processing state
//

#include "FrameContext.h"

thread_local unsigned int processingFrame = 0;

unsigned int FrameContext::getCurrentProcessingFrameNumber() {
    return processingFrame;
}

void FrameContext::setCurrentProcessingFrameNumber(unsigned int frameNumber) {
    processingFrame = frameNumber;
}

