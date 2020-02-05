//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENESCENARIO_H
#define AWEBWOWVIEWERCPP_SCENESCENARIO_H

#include <memory>

class IScene;
struct CameraMatrices;
struct CullStage;
struct UpdateStage;

typedef std::shared_ptr<CullStage> HCullStage;
typedef std::shared_ptr<UpdateStage> HUpdateStage;




#include <vector>
#include "../gapi/interface/IDevice.h"
#include "../gapi/interface/meshes/IMesh.h"
#include "objects/ViewsObjects.h"
#include "objects/iScene.h"
#include "camera/CameraInterface.h"

//Holds dependency graph for different scenes
class FrameScenario;



struct CullStage {
//Input:
    HCameraMatrices matricesForCulling;
    std::shared_ptr<IScene> scene;

//Output:
    int adtAreadId = -1;

    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    ExteriorView exteriorView = ExteriorView();
    std::vector<InteriorView> interiorViews = {};

    std::vector<std::shared_ptr<ADTObjRenderRes>> adtArray = {};
    std::vector<std::shared_ptr<M2Object>> m2Array = {};
    std::vector<std::shared_ptr<WmoObject>> wmoArray = {};
};


struct UpdateStage {
//input
    HCullStage cullResult;
    animTime_t delta;
    HCameraMatrices cameraMatrices;

    //Result
    std::vector<HGMesh> meshes;
};





class SceneComposer;

class FrameScenario {
    friend class SceneComposer;
private:
    std::vector<HCullStage> cullStages;
    std::vector<HUpdateStage> updateStages;

    HDrawStage lastDrawStage;
public:
    HCullStage addCullStage(HCameraMatrices matricesForCulling, std::shared_ptr<IScene> scene);
    HUpdateStage addUpdateStage(HCullStage cullStage, animTime_t deltaTime, HCameraMatrices matricesForUpdate);

    HDrawStage addDrawStage(HUpdateStage updateStage,
                            HCameraMatrices matricesForDrawing,
                            std::vector<HDrawStage> drawStageDependencies,
                            bool setViewPort,
                            ViewPortDimensions viewPortDimensions,
                            bool clearScreen);

    HDrawStage getDrawStage();
};

typedef std::shared_ptr<FrameScenario> HFrameScenario;

#endif //AWEBWOWVIEWERCPP_SCENESCENARIO_H
