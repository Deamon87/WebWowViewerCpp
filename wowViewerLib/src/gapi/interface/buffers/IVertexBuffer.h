//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IVERTEXBUFFER_H
#define AWEBWOWVIEWERCPP_IVERTEXBUFFER_H
class IVertexBuffer {
public:
    virtual ~IVertexBuffer() {};
    virtual void uploadData(void *, int length) = 0;
};
#endif //AWEBWOWVIEWERCPP_IVERTEXBUFFER_H
