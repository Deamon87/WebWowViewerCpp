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
#include "objects/wmo/wmoObject.h"


struct FrameInputParams {
    HCameraMatrices matricesForCulling;
    HCameraMatrices cameraMatricesForRendering;
    HCameraMatrices cameraMatricesForDebugCamera;
    HScene scene;

    //Just for proper BoundingBox calculus
    float deltaX = 0.0f;
    float deltaY = 0.0f;
    float deltaZ = 0.0f;

    //Time advance
    animTime_t delta;

    //Parameters for framebuffer
    ViewPortDimensions viewPortDimensions;
    bool invertedZ = false;
    bool clearScreen = false;
};

struct MapRenderPlan {
    int adtAreadId = -1;
    int areaId = -1;
    int parentAreaId = -1;

    //Result of culling test
    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    bool currentWmoGroupIsExtLit = false;
    bool currentWmoGroupShowExtSkybox = false;
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    FrameViewsHolder viewsHolder;

    HFrameDepedantData frameDependentData = std::make_shared<FrameDepedantData>();

    //Objects for update and rendering
    std::vector<std::shared_ptr<ADTObjRenderRes>> adtArray = {};
    M2ObjectListContainer m2Array;
    WMOListContainer wmoArray;
    WMOGroupListContainer wmoGroupArray;

    //Settings for the frame
};

class SceneComposer;

#include "objects/iScene.h"

class FrameScenario {
    friend class SceneComposer;
private:

public:
    HCullStage addCullStage(HCameraMatrices matricesForCulling, std::shared_ptr<IScene> scene, mathfu::vec3 deltas = {0,0,0});
    HUpdateStage addUpdateStage(HCullStage &cullStage, animTime_t deltaTime, HCameraMatrices matricesForUpdate);

    HDrawStage addDrawStage(std::vector<HUpdateStage> &updateStage,
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
