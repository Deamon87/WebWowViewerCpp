//
// Created by deamon on 21.09.18.
//

#ifndef AWEBWOWVIEWERCPP_WOWFRAMEDATA_H
#define AWEBWOWVIEWERCPP_WOWFRAMEDATA_H

#include "../../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"
#include "wmo/wmoObject.h"
#include <vector>



class WoWFrameData {
public:
    animTime_t deltaTime = 0.0;

    //Frustum culling stage
    mathfu::vec3 m_cameraVec3;
    mathfu::mat4 m_lookAtMat4;
    mathfu::mat4 m_perspectiveMatrixForCulling;
    mathfu::mat4 m_secondLookAtMat;
    mathfu::mat4 m_viewCameraForRender;
    mathfu::mat4 m_perspectiveMatrix;

    mathfu::vec3 m_upVector;
    mathfu::vec3 m_sunDir;
    mathfu::vec4 m_globalAmbientColor;
    mathfu::vec4 m_globalSunColor;



    //Frustum culling results
	std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    int adtAreadId = -1;

    ExteriorView exteriorView = ExteriorView();
    std::vector<InteriorView> interiorViews = {};

    std::vector<std::shared_ptr<ADTObjRenderRes>> adtArray = {};
	std::vector<std::shared_ptr<M2Object>> m2Array = {};
    std::vector<std::shared_ptr<WmoObject>> wmoArray = {};

    //Occlusion culling params
    std::vector<HGOcclusionQuery> renderedThisFrame = {};

    //Occlusion culling results
    int *occlusionResults = 0;

    //Rendering params
    std::vector<HGMesh> meshes = {};
};

#endif //AWEBWOWVIEWERCPP_WOWFRAMEDATA_H
