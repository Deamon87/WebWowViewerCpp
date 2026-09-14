//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENEPLAN_H
#define AWEBWOWVIEWERCPP_MAPSCENEPLAN_H

#include <vector>
#include "FrameDependentData.h"
#include "../../engine/objects/ViewsObjects.h"
#include "../../engine/objects/wmo/wmoObject.h"

struct MapRenderPlan {
    int adtAreadId = -1;
    int areaId = -1;
    int parentAreaId = -1;

    std::string areaName = "";
    std::string wmoAreaName = "";

    animTime_t deltaTime;
    HCameraMatrices renderingMatrices;

    bool renderSky = false;

    //Result of culling test
    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    bool currentWmoGroupIsExtLit = false;
    bool currentWmoGroupShowExtSkybox = false;
    WMOObjId m_currentWMO = emptyWMO;
    int m_currentWmoGroup = -1;

    FrameViewsHolder viewsHolder;

    HFrameDependantData frameDependentData = std::make_shared<FrameDependantData>();

    //Objects for update and rendering
    std::vector<ADTObjRenderRes> adtArray = {};

    M2ObjectListContainer m2Array;
    WMOListContainer wmoArray;
    WMOGroupListContainer wmoGroupArray;

    std::vector<LocalLight> pointLights;
    std::vector<SpotLight> spotLights;
    std::vector<SpotLight> insideSpotLights;

    // Result of the async GPU object-id pick readback (right-click selection), decoded on the CPU.
    bool hasSelectedM2 = false;
    M2ObjId selectedM2 = (M2ObjId)0;

    WMOObjId selectedWMO = emptyWMO;
    int selectedWMOGroupNum = -1;

    bool hasSelectedAABB = false;
    CAaBox selectedAABB;

    // Result of a passive "hover peek" pick (e.g. GameObject nameplates) — same readback
    // mechanism as hasSelectedM2/selectedWMO above, but kept separate so hovering doesn't
    // change the actual selection (and therefore doesn't trigger the selection AABB highlight).
    bool hasHoveredM2 = false;
    M2ObjId hoveredM2 = (M2ObjId)0;

    WMOObjId hoveredWMO = emptyWMO;
    int hoveredWMOGroupNum = -1;
};
typedef std::shared_ptr<MapRenderPlan> HMapRenderPlan;
#endif //AWEBWOWVIEWERCPP_MAPSCENEPLAN_H
