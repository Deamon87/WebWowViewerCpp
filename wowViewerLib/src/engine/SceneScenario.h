//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENESCENARIO_H
#define AWEBWOWVIEWERCPP_SCENESCENARIO_H

class iInnerSceneApi;

#include <vector>
#include "../gapi/interface/meshes/IMesh.h"
#include "objects/ViewsObjects.h"
#include "objects/iInnerSceneApi.h"
#include "camera/CameraInterface.h"

//Holds dependency graph for different scenes
class FrameScenario;

struct CameraMatrices {
    mathfu::mat4 perspectiveMat;
    mathfu::mat4 lookAtMat;
};
typedef std::shared_ptr<CameraMatrices> HCameraMatrices;


struct CullStage {
//Input:
    HCameraMatrices matricesForCulling;
    iInnerSceneApi* scene;

//Output:

    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    ExteriorView exteriorView = ExteriorView();
    std::vector<InteriorView> interiorViews = {};

    std::vector<std::shared_ptr<ADTObjRenderRes>> adtArray = {};
    std::vector<std::shared_ptr<M2Object>> m2Array = {};
    std::vector<std::shared_ptr<WmoObject>> wmoArray = {};
};

typedef std::shared_ptr<CullStage> HCullStage;

struct UpdateStage {
//input
    HCullStage cullResult;
    animTime_t delta;

    HCameraMatrices cameraMatrices;
};

typedef std::shared_ptr<UpdateStage> HUpdateStage;

struct DrawStage;
typedef std::shared_ptr<DrawStage> HDrawStage;

struct DrawStage {
    HCameraMatrices matricesForRendering;
    std::vector<HDrawStage> drawStageDependencies;

    std::vector<HGMesh> meshes;

    HFrameBuffer target;
};


class FrameScenario {
private:
    std::vector<HCullStage> cullStages;
    std::vector<HUpdateStage> updateStages;

    HDrawStage drawStage;
public:
    HCullStage addCullStage(HCameraMatrices matricesForCulling, iInnerSceneApi* scene);
    HUpdateStage addUpdateStage(HCullStage cullStage, animTime_t deltaTime, HCameraMatrices matricesForUpdate);

    HDrawStage getDrawStage();
};

#endif //AWEBWOWVIEWERCPP_SCENESCENARIO_H
