//
// Created by deamon on 13.09.18.
//

#include "GPUFenceGL44.h"

void GPUFenceGL44::setGpuFence() {
    if( m_sync )
    {
        glDeleteSync( m_sync );
    }
    m_sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
}

void GPUFenceGL44::wait() {
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
}
