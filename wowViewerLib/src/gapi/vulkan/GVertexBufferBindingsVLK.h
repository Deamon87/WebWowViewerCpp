//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H

class GVertexBufferGL33;
class GIndexBufferGL33;
class GVertexBufferBindingsVLK;

#include <vector>
#include <cstdint>
#include "../interface/IVertexBufferBindings.h"
#include "../interface/IDevice.h"


struct vkBufferFormatHolder {
    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescription;
};

class GVertexBufferBindingsVLK : public IVertexBufferBindings {
private:
    std::vector<vkBufferFormatHolder> m_BufferBindingsVLK;
public:
    explicit GVertexBufferBindingsVLK();
    ~GVertexBufferBindingsVLK() override;

private:

public:
    void save() override;

    void setIndexBuffer(HGIndexBuffer indexBuffer) override;
    void addVertexBufferBinding(const HGVertexBuffer &vertexBuffer, const std::vector<GBufferBinding> &bindings) override;
    std::vector<vkBufferFormatHolder> &getVLKFormat();

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFERBINDINGS_H
