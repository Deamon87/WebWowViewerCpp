//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENESCENARIO_H
#define AWEBWOWVIEWERCPP_SCENESCENARIO_H

#include <memory>

class IScene;
typedef std::shared_ptr<IScene> HScene;
struct CameraMatrices;
struct CullStage;
struct UpdateStage;

//Holds dependency graph for different scenes
class FrameScenario;


#include <vector>
#include "../gapi/interface/IDevice.h"
#include "../gapi/interface/meshes/IMesh.h"
#include "objects/ViewsObjects.h"
#include "camera/CameraInterface.h"
#include "DrawStage.h"

struct CullStage {
//Input:
    HCameraMatrices matricesForCulling;
    HScene scene;

//Output:
    int adtAreadId = -1;

    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    bool currentWmoGroupIsExtLit = false;
    bool currentWmoGroupShowExtSkybox = false;
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    ExteriorView exteriorView = ExteriorView();
    std::vector<InteriorView> interiorViews = {};

    HFrameDepedantData frameDepedantData = std::make_shared<FrameDepedantData>();

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

//Output
    HMeshesToRender opaqueMeshes;
    HMeshesToRender transparentMeshes;

    std::vector<HGUniformBufferChunk> uniformBufferChunks;
    //
};
typedef std::shared_ptr<UpdateStage> HUpdateStage;


class SceneComposer;

#include "objects/iScene.h"

class FrameScenario {
    friend class SceneComposer;
    struct DrawStageLinkage {
        HScene scene;
        HUpdateStage updateStage;
        HDrawStage drawStage;
    };
private:
    std::vector<HCullStage> cullStages;
    std::vector<HUpdateStage> updateStages;

    std::vector <DrawStageLinkage> drawStageLinks;

    HDrawStage lastDrawStage;
public:
    HCullStage addCullStage(HCameraMatrices matricesForCulling, std::shared_ptr<IScene> scene);
    HUpdateStage addUpdateStage(HCullStage cullStage, animTime_t deltaTime, HCameraMatrices matricesForUpdate);

    HDrawStage addDrawStage(HUpdateStage updateStage,
                            HScene scene,
                            HCameraMatrices matricesForDrawing,
                            std::vector<HDrawStage> const &drawStageDependencies,
                            bool setViewPort,
                            ViewPortDimensions const &viewPortDimensions,
                            bool clearScreen, bool invertedZ,
                            mathfu::vec4 const &clearColor, HFrameBuffer fbTarget);

    HDrawStage getDrawStage();
};
typedef std::shared_ptr<FrameScenario> HFrameScenario;


#endif //AWEBWOWVIEWERCPP_SCENESCENARIO_H
