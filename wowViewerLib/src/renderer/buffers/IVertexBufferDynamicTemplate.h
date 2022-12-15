//
// Created by Deamon on 14.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICTEMPLATE_H
#define AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICTEMPLATE_H

#include <vector>

template<typename T>
class IVertexBufferDynamicTemplate {
public:
    virtual ~IVertexBufferDynamicTemplate() = 0;

    virtual const std::vector<T> &getBuffer() = 0;
    virtual void save(size_t sizeToSave) = 0;
};


#endif //AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICTEMPLATE_H
