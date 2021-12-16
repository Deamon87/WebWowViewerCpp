//
// Created by deamon on 22.10.19.
//

#ifndef AWEBWOWVIEWERCPP_VIEWSOBJECTS_H
#define AWEBWOWVIEWERCPP_VIEWSOBJECTS_H

class InteriorView;
class ExteriorView;
class ADTObjRenderRes;

#include <vector>
#include "wmo/wmoGroupObject.h"
#include "m2/m2Object.h"

class ADTObjRenderRes {
public:
    std::shared_ptr<AdtObject> adtObject;
    bool wasLoaded = false;
    bool drawChunk[256] = {false};
    bool drawWaterChunk[256] = {false};
    bool checkRefs[256] = {false};
};

class GeneralView {
public:
    bool viewCreated = false;
    std::vector<std::shared_ptr<WmoGroupObject>> drawnWmos = {}; //Wmos which draw their meshes
    std::vector<std::shared_ptr<WmoGroupObject>> wmosForM2 = {}; //Wmo which contribute M2s to the scene
    std::vector<std::shared_ptr<M2Object>> drawnM2s = {};

    //Support several frustum planes because of how portal culling works
    std::vector<std::vector<mathfu::vec3>> worldPortalVertices = {};
    std::vector<std::vector<mathfu::vec4>> frustumPlanes = {};

    int level = -1;
    int renderOrder = -1;
    struct PortalPointsFrame {
        HGIndexBuffer m_indexVBO;
        HGVertexBuffer m_bufferVBO;

        HGVertexBufferBindings m_bindings;
        std::vector<HGMesh> m_meshes = {};
    } portalPointsFrame;



    virtual void collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes);
    virtual void setM2Lights(std::shared_ptr<M2Object> m2Object);

    void produceTransformedPortalMeshes(HApiContainer apiContainer,std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes);
    void addM2FromGroups(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos);
};

class InteriorView : public GeneralView {
public:
    int portalIndex;
    void setM2Lights(std::shared_ptr<M2Object> m2Object) override;
};

class ExteriorView : public GeneralView {
public:
	std::vector<std::shared_ptr<ADTObjRenderRes>> drawnADTs = {};
    std::vector<HGMesh> m_opaqueMeshes = {};
    std::vector<HGMesh> m_transparentMeshes = {};

public:
    void collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes) override;
};


#endif //AWEBWOWVIEWERCPP_VIEWSOBJECTS_H
