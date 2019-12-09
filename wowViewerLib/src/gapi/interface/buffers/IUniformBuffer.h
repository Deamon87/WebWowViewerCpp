//
// Created by Deamon on 8/27/2018.
//

#ifndef AWEBWOWVIEWERCPP_IUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_IUNIFORMBUFFER_H

class IUniformBuffer {
private:
    void bind(int bindingPoint, int slot, int offset, int length); //Should be called only by GDevice

public:
    virtual ~IUniformBuffer() {};

    template<typename T>
    T &getObject() {
//        assert(sizeof(T) == m_size);
        return *(T *) getPointerForModification();
    }
    virtual void *getPointerForModification() = 0;
    virtual void *getPointerForUpload() = 0;

    virtual void save(bool initialSave = false) = 0;
    virtual void createBuffer() = 0;


};

#endif //AWEBWOWVIEWERCPP_IUNIFORMBUFFER_H
