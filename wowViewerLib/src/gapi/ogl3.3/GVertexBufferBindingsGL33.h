//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GVertexBufferGL33;
class GIndexBufferGL33;
class GVertexBufferBindingsGL33;

#include <vector>
#include <cstdint>
#include "buffers/GVertexBufferGL33.h"
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"
#include "GDeviceGL33.h"

class GVertexBufferBindingsGL33 : public IVertexBufferBindings {
    friend class GDeviceGL33;
private:
    std::vector<GVertexBufferBinding> m_bindings;
    HGIndexBuffer m_indexBuffer = HGIndexBuffer(nullptr);


private:
    HGDevice m_device;
private:
    std::vector<char> m_buffer;

public:
    explicit GVertexBufferBindingsGL33(const HGDevice &m_device);
    ~GVertexBufferBindingsGL33() override;

private:
    void createBuffer();
    void destroyBuffer();
    void bind(); //Should be called only by GDevice
    void unbind();
public:
    void save() override;

    void setIndexBuffer(HGIndexBuffer indexBuffer) override;
    void addVertexBufferBinding(GVertexBufferBinding binding) override;

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
