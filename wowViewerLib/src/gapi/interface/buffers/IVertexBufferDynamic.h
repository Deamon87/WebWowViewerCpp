//
// Created by deamon on 31.10.19.
//

#ifndef AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMIC_H
#define AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMIC_H

#include <cstddef>
#include "IVertexBuffer.h"

class IVertexBufferDynamic : public IVertexBuffer {
public:
    ~IVertexBufferDynamic() override = default;

    template<typename T>
    T &getObject() {
//        assert(sizeof(T) == m_size);
        return *(T *) getPointerForModification();
    }
    virtual void *getPointerForModification() = 0;

    virtual void save(size_t sizeToSave) = 0;
    virtual void resize(size_t size) = 0;
};
#endif //AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMIC_H
