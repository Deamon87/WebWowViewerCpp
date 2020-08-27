//
// Created by Deamon on 7/22/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2MESH_H
#define AWEBWOWVIEWERCPP_GM2MESH_H

#include "GMeshVLK.h"

class GM2MeshVLK : public GMeshVLK {
    friend class GDeviceVLK;
protected:
    GM2MeshVLK(IDevice &device, const gMeshTemplate &meshTemplate);

public:
    void setM2Object(void * m2Object) override;
    void setLayer(int layer) override;
    void setPriorityPlane(int priorityPlane) override;
    void setQuery(const HGOcclusionQuery &query) override;
    void setSortDistance(float distance) override;
    float getSortDistance() override;
private:
    HGOcclusionQuery m_query = nullptr;

};
#endif //AWEBWOWVIEWERCPP_GM2MESH_H
