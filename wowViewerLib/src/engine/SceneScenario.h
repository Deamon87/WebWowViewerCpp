//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENESCENARIO_H
#define AWEBWOWVIEWERCPP_SCENESCENARIO_H

#include "../gapi/interface/meshes/IMesh.h"
#include "objects/ViewsObjects.h"

//Holds dependency graph for different scenes

struct FrameBufferScenario {
    
};

struct CullResult {
    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    ExteriorView exteriorView = ExteriorView();
    std::vector<InteriorView> interiorViews = {};

    std::vector<std::shared_ptr<ADTObjRenderRes>> adtArray = {};
    std::vector<std::shared_ptr<M2Object>> m2Array = {};
    std::vector<std::shared_ptr<WmoObject>> wmoArray = {};
};

struct FrameBufferStages {
    //Stages
    std::vector<IMesh> adtOpaqueMeshes;
    std::vector<IMesh> wmoOpaqueMeshes;

    std::vector<IMesh> projectionMeshes;

    std::vector<IMesh> m2OpaqueMeshes;
    std::vector<IMesh> skyBoxMeshes;
    std::vector<IMesh> areaMeshes;

    std::vector<IMesh> transparentMeshes;
    HFrameBuffer target;
};

struct SceneScenario {



};

#endif //AWEBWOWVIEWERCPP_SCENESCENARIO_H
