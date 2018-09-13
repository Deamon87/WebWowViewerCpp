//
// Created by deamon on 13.09.18.
//

#ifndef AWEBWOWVIEWERCPP_GPUFENCEGL44_H
#define AWEBWOWVIEWERCPP_GPUFENCEGL44_H

#include "../../../engine/opengl/header.h"
#include "../../interface/syncronization/IGPUFence.h"

class GPUFenceGL44 : public IGPUFence {
public:
    void setGpuFence() override {
        if( m_sync )
        {
            glDeleteSync( m_sync );
        }
        m_sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
    };
    void wait() override {
        if( m_sync )
        {
            while( 1 )
            {
                GLenum waitReturn = glClientWaitSync( m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1 );
                if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) {
                    glDeleteSync( m_sync );
                    m_sync = 0;
                    return;
                }
            }
        }
    };
private:
    GLsync m_sync = 0;

};


#endif //AWEBWOWVIEWERCPP_GPUFENCEGL44_H
