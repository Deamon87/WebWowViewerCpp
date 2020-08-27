//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GVERTEXBUFFERDYNAMIC_H
#define WEBWOWVIEWERCPP_GVERTEXBUFFERDYNAMIC_H

#include "../../interface/IDevice.h"
#include "../../interface/buffers/IVertexBuffer.h"
#include "../GDeviceGL33.h"
#include <memory>

class GVertexBufferDynamicGL33 : public IVertexBufferDynamic {
    friend class GDeviceGL33;

    explicit GVertexBufferDynamicGL33(IDevice &device, size_t maxSize);
public:
    ~GVertexBufferDynamicGL33() override;

    void *getPointerForModification() override;

    void save(size_t sizeToSave) override;
    void resize(size_t sizeToSave) override;
private:
    void createBuffer();
    void destroyBuffer();
    void bind() override; //Should be called only by GDevice
    void unbind() override;

public:
    void uploadData(void *, int length) override;

private:
	GDeviceGL33 &m_device;

private:
    std::vector<char> m_buffer;
	std::vector<char> pIdentifierBuffer = {};
    size_t m_size = 0;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

};

#endif //WEBWOWVIEWERCPP_GVERTEXBUFFER_H
