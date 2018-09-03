//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IINDEXBUFFER_H
#define AWEBWOWVIEWERCPP_IINDEXBUFFER_H
class IIndexBuffer {
public:
    virtual ~IIndexBuffer() {};
    virtual void uploadData(void *, int length) = 0;

};
#endif //AWEBWOWVIEWERCPP_IINDEXBUFFER_H
