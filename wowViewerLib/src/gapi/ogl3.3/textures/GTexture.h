//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURE_H
#define AWEBWOWVIEWERCPP_GTEXTURE_H

#include "../GDevice.h"
#include "../../interface/textures/ITexture.h"

class GTexture : public virtual ITexture {
    friend class GDevice;
protected:
    explicit GTexture(IDevice &device);
public:
    ~GTexture() override;

    void loadData(int width, int height, void *data) override;
    bool getIsLoaded() override;
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();
protected:
    void * pIdentifierBuffer;

    IDevice &m_device;

    bool m_loaded = false;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_H
