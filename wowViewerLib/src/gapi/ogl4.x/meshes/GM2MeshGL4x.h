//
// Created by Deamon on 7/22/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2MESH_4X_H
#define AWEBWOWVIEWERCPP_GM2MESH_4X_H

#include "GMeshGL4x.h"

class GM2MeshGL4x : public GMeshGL4x {
    friend class GDeviceGL4x;
protected:
    GM2MeshGL4x(IDevice &device, const gMeshTemplate &meshTemplate);

public:
    void setM2Object(void * m2Object) override;
    void setLayer(int layer) override;
    void setPriorityPlane(int priorityPlane) override;
    void setQuery(const HGOcclusionQuery &query) override;
    void setSortDistance(float distance) override;

private:
    HGOcclusionQuery m_query = nullptr;

};
#endif //AWEBWOWVIEWERCPP_GM2MESH_4X_H
