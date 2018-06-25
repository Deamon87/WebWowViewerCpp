//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H


#include "GIndexBuffer.h"

class GMesh {


private:
    GVertexBufferBindings bindings;
    bool m_depthWrite;
    bool m_depthCulling;
    int blendMode;

    int start;
    int end;
    int element;

};


#endif //WEBWOWVIEWERCPP_GMESH_H
