//
// Created by Deamon on 7/22/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2MESH_H
#define AWEBWOWVIEWERCPP_GM2MESH_H

#include "GMeshGL33.h"
#include "../../../engine/objects/m2/m2Object.h"

class GM2MeshGL33 : public GMeshGL33, public virtual IM2Mesh {
    friend class GDeviceGL33;
protected:
    GM2MeshGL33(IDevice &device, const gMeshTemplate &meshTemplate);

public:
    void setM2Object(void * m2Object) override;
    void setLayer(int layer) override;
    void setPriorityPlane(int priorityPlane) override;
    void setQuery(const HGOcclusionQuery &query) override;
    void setSortDistance(float distance) override;

private:
    float m_sortDistance = 0;
    int m_priorityPlane;
    int m_layer;
    M2Object *m_m2Object = nullptr;
    HGOcclusionQuery m_query = nullptr;

};
#endif //AWEBWOWVIEWERCPP_GM2MESH_H
