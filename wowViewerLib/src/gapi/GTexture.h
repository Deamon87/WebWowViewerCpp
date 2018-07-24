//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTURE_H
#define AWEBWOWVIEWERCPP_GTEXTURE_H


class GTexture {
    friend class GDevice;
protected:
    explicit GTexture(GDevice &device);
public:
    virtual ~GTexture();

    void loadData(int width, int height, void *data);
    virtual bool getIsLoaded();
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();
protected:
    void * pIdentifierBuffer;

    GDevice &m_device;

    bool m_loaded = false;
};


#endif //AWEBWOWVIEWERCPP_GTEXTURE_H
