//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/mathHelper_culling.h"
#include "../../algorithms/mathHelper_culling_sse.h"
#include "../../algorithms/grahamScan.h"
#include "../../persistance/header/commonFileStructs.h"
#include "./../../../gapi/interface/IDevice.h"
#include "../../../renderer/frame/FrameProfile.h"
#include <algorithm>
#include <limits>

std::vector<mathfu::vec3> createAntiPortal(const HWmoGroupGeom& groupGeom, const mathfu::mat4 &placementMat)
{
    std::vector<mathfu::vec3> points(0);

    if (groupGeom == nullptr || groupGeom->getStatus() != FileStatus::FSLoaded) return points;
    for ( unsigned int mopy_index (0), movi_index (0)
            ; mopy_index < groupGeom->mopyLen
            ; ++mopy_index, ++movi_index
            )
    {
        points.emplace_back() = (placementMat * mathfu::vec4(mathfu::vec3(groupGeom->verticles[groupGeom->indicies[3*movi_index+0]]), 1.0f)).xyz();
        points.emplace_back() = (placementMat * mathfu::vec4(mathfu::vec3(groupGeom->verticles[groupGeom->indicies[3*movi_index+1]]), 1.0f)).xyz();
        points.emplace_back() = (placementMat * mathfu::vec4(mathfu::vec3(groupGeom->verticles[groupGeom->indicies[3*movi_index+2]]), 1.0f)).xyz();
    }
    return points;
}


void WmoObject::startLoading() {
    if (!m_loading) {
        m_loading = true;


        Cache<WmoMainGeom> *wmoGeomCache = m_api->cacheStorage->getWmoMainCache();
        if (!useFileId) {
            mainGeom = wmoGeomCache->get(m_modelName);
        } else {
            mainGeom = wmoGeomCache->getFileId(m_modelFileId);
        }

    }
}

std::shared_ptr<M2Object> WmoObject::getDoodad(int index, int fromGroupIndex) {
    auto iterator = this->m_doodadsUnorderedMap.find(index);
    if (iterator != this->m_doodadsUnorderedMap.end()) {
        auto s_doodad = iterator->second.lock();
        if (s_doodad) {
            return s_doodad;
        }
        else {
            this->m_doodadsUnorderedMap.erase(iterator);
        }
    }

    bool existsInActiveDoodadSets = false;
    for (int i = 0; i < this->mainGeom->doodadSetsLen; i++) {
        if (!m_activeDoodadSets[i]) continue;;

        if (
            index >= this->mainGeom->doodadSets[i].firstinstanceindex &&
            index <= this->mainGeom->doodadSets[i].firstinstanceindex + this->mainGeom->doodadSets[i].numDoodads
            ) {
            existsInActiveDoodadSets = true;
            break;
        }
    }

    if (!existsInActiveDoodadSets)
        return nullptr;

    const SMODoodadDef *doodadDef = &this->mainGeom->doodadDefs[index];

    bool fileIdMode = false;
    int doodadfileDataId = 0;
    std::string fileName;
    if (this->mainGeom->doodadFileDataIds == nullptr && this->mainGeom->doodadFileDataIdsLen == 0) {
        if (this->mainGeom->doodadNamesFieldLen <= 0) {
            return nullptr;
        }
        assert(doodadDef->name_offset < this->mainGeom->doodadNamesFieldLen);
        fileName = std::string (&this->mainGeom->doodadNamesField[doodadDef->name_offset]);
    } else {
        doodadfileDataId = this->mainGeom->doodadFileDataIds[doodadDef->name_offset];
        fileIdMode = true;
    }

    auto m2Object = m2Factory->createObject(m_api);

    m2Object->setLoadParams(0, {},{});

    //Initial state for doodads in WMO is interior lighting. Unless there is exterior group WMO that references it
    m2Object->setInteriorExteriorBlend(0.0f);
    if (fileIdMode) {
        m2Object->setModelFileId(doodadfileDataId);
    } else {
        m2Object->setModelFileName(fileName);
    }
    m2Object->createPlacementMatrix(*doodadDef, m_placementMatrix);
    m2Object->calcWorldPosition();

    //Set interior lighting params
    float mddiVal = 1.0f;
    if (doodadDef->flag_0x10 && index < mainGeom->m_doodadAdditionalInfoLen) {
        mddiVal = mainGeom->m_doodadAdditionalInfo[index];
    }

    auto l_wmoId = this->getObjectId();
    m2Object->addPostLoadEvent([l_wmoId, l_doodadDef = *doodadDef, mddiVal, fromGroupIndex](M2Object *m2Object) {
        auto wmoObj = wmoFactory->getObjectById<0>(l_wmoId);
        if (!wmoObj) return;

        auto wmoFlag = wmoObj->mainGeom->groups[fromGroupIndex].flags;
        if (wmoFlag.EXTERIOR_LIT || wmoFlag.EXTERIOR || !wmoFlag.INTERIOR) return;

        wmoObj->applyLightingParamsToDoodad(&l_doodadDef, m2Object, mddiVal, fromGroupIndex);
    });


    // if (doodadDef->flag_AcceptProjTex) std::cout << "doodadDef->flag_AcceptProjTex" << std::endl;
    // if (doodadDef->flag_0x2) std::cout << "doodadDef->flag_0x2" << std::endl;
    // if (doodadDef->flag_0x4) std::cout << "doodadDef->flag_0x4" << std::endl;
    // if (doodadDef->flag_0x8) std::cout << "doodadDef->flag_0x8" << std::endl;
    // if (doodadDef->flag_0x10) std::cout << "doodadDef->flag_0x10" << std::endl;
    // if (doodadDef->flag_0x20) std::cout << "doodadDef->flag_0x20" << std::endl;
    // if (doodadDef->flag_0x40) std::cout << "doodadDef->flag_0x40" << std::endl;
    // if (doodadDef->flag_0x80) std::cout << "doodadDef->flag_0x80" << std::endl;

    this->m_doodadsUnorderedMap[index] = m2Object;

    return m2Object;
}

mathfu::vec3 addDirectColorAndAmbient(mathfu::vec3 directColor, mathfu::vec3 ambient) {
    auto sum = ambient + directColor;

    float biggestComponent = std::max(sum.x, std::max(sum.y, sum.z));
    if (biggestComponent > 1.0f) {
        sum = sum * (1.0f / biggestComponent);
    }
    sum = mathfu::vec3(
        fminf(sum.x, 1.0f),
        fminf(sum.y, 1.0f),
        fminf(sum.z, 1.0f)
    );
    return sum;
}

inline mathfu::vec3 fixDirectColor(const mathfu::vec3 &directColor, uint8_t limit) {
    float biggestComponent = std::max(directColor.x, std::max(directColor.y, directColor.z));

    if (biggestComponent <= 0.00001f) {
        biggestComponent = 1.0f/255.0f;
    }

    const float limitInFloat = limit * (1.0f/255.0f);
    if (biggestComponent < limitInFloat) {
        auto hsv = MathHelper::rgb2hsv(directColor);
        hsv.v = hsv.v * (limitInFloat / biggestComponent);
        auto rgb = MathHelper::hsv2rgb(hsv);
        return rgb;
    }

    return directColor;
}
inline mathfu::vec3 fixAmbient1(const mathfu::vec3 &ambient, uint8_t limit) {
    float biggestComponent = std::max(ambient.x, std::max(ambient.y, ambient.z));

    if (biggestComponent <= 0.00001f) {
        biggestComponent = 1.0f/255.0f;
    }

    const float limitInFloat = limit * (1.0f/255.0f);
    if (biggestComponent > limitInFloat) {
        return ambient * (limitInFloat / biggestComponent);
    }

    return ambient;
}

void WmoObject::applyColorFromMOLT(
        const SMODoodadDef *doodadDef,
        M2Object* doodad,
        std::array<mathfu::vec3, 3> &interiorAmbients,
        mathfu::vec3 &color,
        bool &hasDoodad0x4Flag,
        int fromGroupIndex)
{

    hasDoodad0x4Flag = doodadDef->flag_0x4;
    if (doodadDef->flag_0x40)
        hasDoodad0x4Flag = true;

    if (doodadDef->flag_0x8) {
        interiorAmbients[0] = color;
        interiorAmbients[1] = color;
        interiorAmbients[2] = color;

        doodad->setAmbientColorOverride(
            interiorAmbients[0],
            interiorAmbients[1],
            interiorAmbients[2]
        );
    }

    if (hasDoodad0x4Flag) {
        if (doodadDef->flag_0x40) {
            color = mathfu::vec3(0,0,0);
            doodad->setInteriorDirectColor(color);
        } else if (doodadDef->color.a != 255 && doodadDef->color.a < this->mainGeom->lightsLen) {
            auto &light = this->mainGeom->lights[doodadDef->color.a];

            auto MOLTWorldPos = this->m_placementMatrix * mathfu::vec4(mathfu::vec3(light.position), 1.0f);

            const auto &doodadAAbb = doodad->getAABB();

            auto sunDirVec = (mathfu::vec3(doodadAAbb.max) + mathfu::vec3(doodadAAbb.min)) * 0.5f - MOLTWorldPos.xyz();
            if (sunDirVec.LengthSquared() > 0) {
                doodad->setSunDirOverride(sunDirVec.Normalized());
            }

            auto lightColor = ImVectorToVec4(light.color).xyz() * light.intensity;
            float biggestComponent = std::max(lightColor.x, std::max(lightColor.y, lightColor.z));
            if (biggestComponent > 1.0f) {
                float inv = 1.0f / biggestComponent;
                lightColor = lightColor * inv;
                color = mathfu::vec3(
                    fminf(lightColor.x, 1.0f),
                    fminf(lightColor.y, 1.0f),
                    fminf(lightColor.z, 1.0f)
                );
            }

            if (doodadDef->flag_0x2 == 0) {
                auto directColor = color;
                directColor = fixDirectColor(directColor, 0x70);
                doodad->setInteriorDirectColor(directColor);
            }
        }
    }

    if (!doodadDef->flag_0x2)
        return;

    mathfu::vec3 interiorLightStart;
    if (doodadDef->color.a == 255) {
        auto const aabb = this->groupObjects[fromGroupIndex]->getWorldAABB();
        interiorLightStart = (mathfu::vec3(aabb.max) + mathfu::vec3(aabb.min)) * 0.5f;
    } else {
        auto &light = this->mainGeom->lights[doodadDef->color.a];

        interiorLightStart = this->m_placementMatrix * mathfu::vec4(mathfu::vec3(light.position), 1.0f).xyz();
    }

    const auto &doodadAAbb = doodad->getAABB();

    auto sunDirVec = (mathfu::vec3(doodadAAbb.max) + mathfu::vec3(doodadAAbb.min)) * 0.5f - interiorLightStart;

    if (sunDirVec.LengthSquared() > 0) {
        doodad->setSunDirOverride(sunDirVec.Normalized());
    }

}

void WmoObject::applyLightingParamsToDoodad(const SMODoodadDef *doodadDef, M2Object *doodad, float mddiVal, int fromGroupIndex) {
    std::array<mathfu::vec3, 3> interiorAmbients;
    mathfu::Vector<float, 3> color;
    bool hasDoodad0x4Flag;

    interiorAmbients = getAmbientColors();
    color = ImVectorToVec4(doodadDef->color).xyz();

    applyColorFromMOLT(doodadDef, doodad, interiorAmbients, color, hasDoodad0x4Flag, fromGroupIndex);

    //If both flag_0x4 and flag_0x8 are set - nothing is applied
    if (hasDoodad0x4Flag && doodadDef->flag_0x8)
        return;

    if (doodadDef->flag_0x2) {
        if (doodadDef->flag_0x10 && !hasDoodad0x4Flag) {
            mathfu::vec3 directColor = mathfu::vec3(0,0,0);
            if (!doodadDef->flag_0x80) {
                mddiVal = std::max<float>(mddiVal, 1.0f);
                directColor = color * mddiVal;
            }
            color = addDirectColorAndAmbient(directColor,  interiorAmbients[0]);
        }
    } else {
        if (doodadDef->flag_0x10) {
            mathfu::vec3 directColor = mathfu::vec3(0,0,0);
            if (!doodadDef->flag_0x80) {
                mddiVal = std::max<float>(mddiVal, 1.0f);
                directColor = color * mddiVal;
            }

            interiorAmbients[0] = addDirectColorAndAmbient(directColor,  interiorAmbients[0]);
            interiorAmbients[1] = addDirectColorAndAmbient(directColor,  interiorAmbients[1]);
            interiorAmbients[2] = addDirectColorAndAmbient(directColor,  interiorAmbients[2]);
            color = interiorAmbients[0];
        } else {
            interiorAmbients[0] = color;
            interiorAmbients[1] = color;
            interiorAmbients[2] = color;
        }

        color = fixDirectColor(color, 0x70u);
        interiorAmbients[0] = fixAmbient1(interiorAmbients[0], 0x60u);
        interiorAmbients[1] = fixAmbient1(interiorAmbients[1], 0x60u);
        interiorAmbients[2] = fixAmbient1(interiorAmbients[2], 0x60u);
    }

    if (!hasDoodad0x4Flag) {
        doodad->setInteriorDirectColor(color);
    }
    if (!doodadDef->flag_0x8) {
        doodad->setAmbientColorOverride(
            interiorAmbients[0],
            interiorAmbients[1],
            interiorAmbients[2]
        );
    }
}

void WmoObject::createPlacementMatrix(const SMMapObjDef &mapObjDef){
    mathfu::mat4 adtToWorldMat4 = MathHelper::getAdtToWorldMat4();

    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix *= adtToWorldMat4;
    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(mapObjDef.position));
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(-1, 1, -1));

    placementMatrix *= MathHelper::RotationY(toRadian(mapObjDef.rotation.y-270));
    placementMatrix *= MathHelper::RotationZ(toRadian(-mapObjDef.rotation.x));
    placementMatrix *= MathHelper::RotationX(toRadian(mapObjDef.rotation.z-90));
    if (mapObjDef.unk != 0) {
        placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(mapObjDef.unk / 1024.0f));
    }
    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;
    m_placementMatChanged = true;

    //BBox is in ADT coordinates. We need to transform it first
    const C3Vector &bb1 = mapObjDef.extents.min;
    const C3Vector &bb2 = mapObjDef.extents.max;
    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(
            adtToWorldMat4, bb1vec, bb2vec);

    createBB(worldAABB);
}
void WmoObject::createPlacementMatrix(const SMMapObjDefObj1 &mapObjDef){
    mathfu::mat4 adtToWorldMat4 = MathHelper::getAdtToWorldMat4();

    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix *= adtToWorldMat4;
    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(mapObjDef.position));
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(-1, 1, -1));

    placementMatrix *= MathHelper::RotationY(toRadian(mapObjDef.rotation.y-270));
    placementMatrix *= MathHelper::RotationZ(toRadian(-mapObjDef.rotation.x));
    placementMatrix *= MathHelper::RotationX(toRadian(mapObjDef.rotation.z-90));

    if (mapObjDef.unk != 0) {
        placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(mapObjDef.unk / 1024.0f));
    }

    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;
    m_placementMatChanged = true;

    //BBox is in ADT coordinates. We need to transform it first
//    C3Vector &bb1 = mapObjDef.extents.min;
//    C3Vector &bb2 = mapObjDef.extents.max;
//    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
//    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);
//
    mathfu::vec4 bb1vec = mathfu::vec4(-1000,-1000,-1000, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(1000, 1000,1000, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(
            adtToWorldMat4, bb1vec, bb2vec);

    createBB(worldAABB);
}

void WmoObject::createGroupObjects(){
    groupObjects = std::vector<std::shared_ptr<WmoGroupObject>>(mainGeom->groupsLen, nullptr);
    groupObjectsLod1 = std::vector<std::shared_ptr<WmoGroupObject>>(mainGeom->groupsLen, nullptr);
    groupObjectsLod2 = std::vector<std::shared_ptr<WmoGroupObject>>(mainGeom->groupsLen, nullptr);
    drawGroupWMO = std::vector<bool>(mainGeom->groupsLen, false);
    lodGroupLevelWMO = std::vector<int>(mainGeom->groupsLen, 0);

    //Initial per-group world boxes from the group table bounds. They are refined from the
    //group file bounds and expanded with doodads by recalcGroupBorders once geometry loads.
    m_groupWorldBorders = std::vector<CAaBox>(mainGeom->groupsLen);
    m_groupVolumeWorldBorders = std::vector<CAaBox>(mainGeom->groupsLen);
    for (int i = 0; i < mainGeom->groupsLen; i++) {
        const CAaBox &localBox = mainGeom->groups[i].bounding_box;
        mathfu::vec4 bbMin(localBox.min.x, localBox.min.y, localBox.min.z, 1.0f);
        mathfu::vec4 bbMax(localBox.max.x, localBox.max.y, localBox.max.z, 1.0f);
        CAaBox worldBox = MathHelper::transformAABBWithMat4(m_placementMatrix, bbMin, bbMax);
        m_groupWorldBorders[i] = worldBox;
        m_groupVolumeWorldBorders[i] = worldBox;
    }

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {

        groupObjects[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
        groupObjects[i]->setWmoApi(this);

        if (mainGeom->gfids.size() > 0) {
            groupObjects[i]->setModelFileId(mainGeom->gfids[0][i]);
            if (mainGeom->gfids.size() > 1 && i < mainGeom->gfids[1].size() && mainGeom->gfids[1][i] > 0) {
                groupObjectsLod1[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
                groupObjectsLod1[i]->setWmoApi(this);
                groupObjectsLod1[i]->setModelFileId(mainGeom->gfids[1][i]);
            }
            if (mainGeom->gfids.size() > 2 && i < mainGeom->gfids[2].size() && mainGeom->gfids[2][i] > 0) {
                groupObjectsLod2[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
                groupObjectsLod2[i]->setWmoApi(this);
                groupObjectsLod2[i]->setModelFileId(mainGeom->gfids[2][i]);
            }
        } else if (!useFileId) {
            std::string numStr = std::to_string(i);
            for (int j = numStr.size(); j < 3; j++) numStr = '0' + numStr;

            std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";
            std::string groupFilenameLod1 = nameTemplate + "_" + numStr + "_lod1.wmo";
            std::string groupFilenameLod2 = nameTemplate + "_" + numStr + "_lod2.wmo";
            groupObjects[i]->setModelFileName(groupFilename);

            groupObjectsLod1[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
            groupObjectsLod1[i]->setWmoApi(this);
            groupObjectsLod1[i]->setModelFileName(groupFilenameLod1);

            groupObjectsLod2[i] = std::make_shared<WmoGroupObject>(this->m_placementMatrix, m_api, mainGeom->groups[i], i);
            groupObjectsLod2[i]->setWmoApi(this);
            groupObjectsLod2[i]->setModelFileName(groupFilenameLod2);
        }
    }
}

void WmoObject::createWorldPortals() {

    int portalCnt = mainGeom->portalsLen;
    auto &portals = mainGeom->portals;
    auto &portalVerticles = mainGeom->portal_vertices;

    if (portalCnt <= 0) return;
    geometryPerPortal = std::vector<PortalInfo_t>(portalCnt);

    for (int j = 0; j < portalCnt; j++) {
        const SMOPortal *portalInfo = &portals[j];


        int base_index = portalInfo->base_index;
        std::vector <mathfu::vec3> portalVecs;
        for (int k = 0; k < portalInfo->index_count; k++) {
            mathfu::vec3 verticle = mathfu::vec3(
                portalVerticles[base_index + k].x,
                portalVerticles[base_index + k].y,
                portalVerticles[base_index + k].z);

            portalVecs.emplace_back() = verticle;
        }

        //Calculate center of the portal
        mathfu::vec3 center(0,0,0);
        for (int k = 0; k < portalVecs.size(); k++) {
            center += portalVecs[k];
        }
        center *= 1.0f / (float)portalVecs.size();

        //Calculate create projection and calc simplified, sorted polygon
        mathfu::vec3 lookAt = center + mathfu::vec3(portalInfo->plane.planeGeneral.normal);
        mathfu::vec3 upVector = portalVecs[0] - center;

        mathfu::mat4 viewMat = mathfu::mat4::LookAt(
                lookAt,
                center,
                upVector
//                1.0f
        );
        mathfu::mat4 viewMatInv = viewMat.Inverse();

        framebased::vector <mathfu::vec3> portalTransformed(portalVecs.size());
        for (int k = 0; k < portalVecs.size(); k++) {
            portalTransformed[k] = (viewMat * mathfu::vec4(portalVecs[k], 1.0)).xyz();
        }

        framebased::vector<mathfu::vec3> hulled = MathHelper::getHullPoints(portalTransformed);

        portalVecs.clear();
        for (int k = 0; k < hulled.size(); k++) {
            portalVecs.emplace_back() = (viewMatInv * mathfu::vec4(hulled[k], 1.0)).xyz();
        }
        geometryPerPortal[j].sortedVericles = portalVecs;

        //Calc CAAbox
        mathfu::vec3 min(99999,99999,99999), max(-99999,-99999,-99999);

        for (const auto & portalVec : portalVecs) {
            min = mathfu::vec3::Min(portalVec, min);
            max = mathfu::vec3::Max(portalVec, max);
        }

        CAaBox &aaBoxCopyTo = geometryPerPortal[j].aaBox;

        aaBoxCopyTo = CAaBox(C3Vector(min), C3Vector(max));;
    }
}

bool WmoObject::doPostLoad(const HMapSceneBufferCreate &sceneRenderer) {
    if (!m_loaded) {
        if (mainGeom != nullptr && mainGeom->getStatus() == FileStatus::FSLoaded){

            this->createMaterialCache();
            this->createNewLights();
            this->createGroupObjects();
            this->calculateAmbient();
            this->createWorldPortals();
            this->createBB(mainGeom->header->bounding_box);

            m_hasAlwaysDrawGroups = false;
            m_hasAntiportalGroups = false;
            for (int i = 0; i < mainGeom->groupsLen; i++) {
                m_hasAlwaysDrawGroups = m_hasAlwaysDrawGroups || (mainGeom->groups[i].flags.ALWAYSDRAW > 0);
                m_hasAntiportalGroups = m_hasAntiportalGroups || (mainGeom->groups[i].flags.ANTIPORTAL > 0);
            }

            m_wmoModelChunk = sceneRenderer->createWMOWideChunk(mainGeom->groupsLen);

            if ((mainGeom->skyBoxM2FileName != nullptr && mainGeom->skyBoxM2FileNameLen > 0) || mainGeom->skyboxM2FileId != 0) {
                skyBox = m2Factory->createObject(m_api, true);
                skyBox->setLoadParams(0, {},{});

                if ( mainGeom->skyboxM2FileId != 0) {
                    skyBox->setModelFileId(mainGeom->skyboxM2FileId);
                } else {
                    skyBox->setModelFileName(&mainGeom->skyBoxM2FileName[0]);
                }
                skyBox->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
                skyBox->calcWorldPosition();
            }

            m_loaded = true;
            m_loading = false;
            return true;
        } else {
            this->startLoading();
        }

        return false;
    }

    return false;
}

void WmoObject::update() {
    if (!m_loaded) return;

    if (m_placementMatChanged) {
        auto &placementMat = m_wmoModelChunk->m_placementMatrix->getObject();
        placementMat.uPlacementMat = m_placementMatrix;
        m_wmoModelChunk->m_placementMatrix->save();

        m_placementMatChanged = false;
    }
    if (m_interiorAmbientsChanged) {
        auto &groupInteriorData = m_wmoModelChunk->m_groupInteriorData->getObject();
        int interCount = (int) std::min(m_groupInteriorData.size(), (size_t) MAX_WMO_GROUPS);
        std::copy(m_groupInteriorData.data(), m_groupInteriorData.data() + interCount, groupInteriorData.interiorData);

        m_wmoModelChunk->m_groupInteriorData->save();
        m_interiorAmbientsChanged = false;
    }

    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr) {
            groupObjects[i]->update();
        }
    }
    for (int i= 0; i < groupObjectsLod1.size(); i++) {
        if(groupObjectsLod1[i] != nullptr) {
            groupObjectsLod1[i]->update();
        }
    }
    for (int i= 0; i < groupObjectsLod2.size(); i++) {
        if(groupObjectsLod2[i] != nullptr) {
            groupObjectsLod2[i]->update();
        }
    }

}
void WmoObject::uploadGeneratorBuffers() {
    if (!m_loaded) return;

//    for (int i= 0; i < groupObjects.size(); i++) {
//        if(groupObjects[i] != nullptr) {
//            groupObjects[i]->uploadGeneratorBuffers();
//        }
//    }
}

void WmoObject::collectMeshes(std::vector<HGMesh> &renderedThisFrame){
    if (!m_loaded) return;
    //Draw debug portals/Lights here?
}

void setActiveDoodadFromMWDR(ActiveDoodadSets &doodadSets, uint16_t doodadSet, const PointerChecker<MWDR> &MWDR, const PointerChecker<uint16_t> &MWDS) {
    doodadSets.reset();

    for (int i = MWDR[doodadSet].begin; i < MWDR[doodadSet].end; i++) {
        auto doodadSetToSet = MWDS[i];
        doodadSets.set(doodadSetToSet);

        // std::cout << "Setting doodadSet " << doodadSetToSet << " to active" << std::endl;
    }
}

void WmoObject::setLoadingParam(const SMMapObjDef &mapObjDef, const PointerChecker<MWDR> &MWDR, const PointerChecker<uint16_t> &MWDS) {
    createPlacementMatrix(mapObjDef);

    if (mapObjDef.flags.modf_use_sets_from_mwds && MWDR != nullptr && MWDS != nullptr) {
        setActiveDoodadFromMWDR(m_activeDoodadSets, mapObjDef.doodadSet, MWDR, MWDS);
    } else {
        m_activeDoodadSets.set(mapObjDef.doodadSet);
        // std::cout << "Setting doodadSet " << mapObjDef.doodadSet << " to active" << std::endl;
    }
    this->m_nameSet = mapObjDef.nameSet;
}
void WmoObject::setLoadingParam(const SMMapObjDefObj1 &mapObjDef, const PointerChecker<MWDR> &MWDR, const PointerChecker<uint16_t> &MWDS) {
    createPlacementMatrix(mapObjDef);

    if (mapObjDef.flags.modf_use_sets_from_mwds && MWDR != nullptr && MWDS != nullptr) {
        setActiveDoodadFromMWDR(m_activeDoodadSets, mapObjDef.doodadSet, MWDR, MWDS);
    } else {
        m_activeDoodadSets.set(mapObjDef.doodadSet);
        // std::cout << "Setting doodadSet " << mapObjDef.doodadSet << " to active" << std::endl;
    }
    this->m_nameSet = mapObjDef.nameSet;
}

void WmoObject::setLoadingParam(mathfu::vec3 pos, mathfu::vec3 scaleVec, mathfu::mat4 *rotationMatrix,
                                       const mathfu::vec4 &localAABBMin, const mathfu::vec4 &localAABBMax) {
    mathfu::mat4 placementMatrix = mathfu::mat4::FromTranslationVector(pos);
    if (rotationMatrix != nullptr) {
        placementMatrix *= *rotationMatrix;
    }
    placementMatrix *= mathfu::mat4::FromScaleVector(scaleVec);

    m_placementInvertMatrix = placementMatrix.Inverse();
    m_placementMatrix = placementMatrix;
    m_placementMatChanged = true;

    m_bbox = MathHelper::transformAABBWithMat4(placementMatrix, localAABBMin, localAABBMax);
    syncBBoxComponent();

    //DoodadSet 0 is always active; no MWDR/MWDS override data exists for generically-placed WMOs.
    m_activeDoodadSets.set(0);
    m_nameSet = 0;
}

HGSamplableTexture WmoObject::getTexture(int textureId, bool isSpec) {
    if (textureId == 0) return nullptr; //Usual case

    //Non-usual case
    if (textureId < 0 || (mainGeom->textureNamesField != nullptr && textureId >= mainGeom->textureNamesFieldLen)) {
        debuglog("Non valid textureindex for WMO")
        return nullptr;
    };

    robin_hood::unordered_flat_map<int, HGSamplableTexture> &textureCache =
        !isSpec ? diffuseTextures : specularTextures;

    auto i = textureCache.find(textureId);
    if (i != textureCache.end()) {
        return i->second;
    }
    HBlpTexture texture;
    if (mainGeom->textureNamesField != nullptr) {
        std::string materialTexture(&mainGeom->textureNamesField[textureId]);
        if (materialTexture == "") return nullptr;

        if (isSpec) {
            materialTexture = materialTexture.substr(0, materialTexture.length() - 4) + "_s.blp";
        }

        texture = m_api->cacheStorage->getTextureCache()->get(materialTexture);
    } else {
        texture = m_api->cacheStorage->getTextureCache()->getFileId(textureId);
    }

    auto hgTexture = m_api->hDevice->createBlpTexture(texture, true, true);
    textureCache[textureId] = hgTexture;

    return hgTexture;
}

void WmoObject::createBB(CAaBox bbox) {
//            groupInfo = this.groupInfo;
//            bb1 = groupInfo.bb1;
//            bb2 = groupInfo.bb2;
//        } else {
//            groupInfo = this.wmoGeom.wmoGroupFile.mogp;
//            bb1 = groupInfo.BoundBoxCorner1;
//            bb2 = groupInfo.BoundBoxCorner2;
//        }
    C3Vector &bb1 = bbox.min;
    C3Vector &bb2 = bbox.max;

    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, bb1vec, bb2vec);

    this->m_bbox = worldAABB;
    syncBBoxComponent();
}

void WmoObject::syncBBoxComponent() {
    if (auto *slot = wmoFactory->getObjectById<1>(this->getObjectId()))
        *slot = this->m_bbox;
}

void WmoObject::updateBB() {
    CAaBox &AABB = this->m_bbox;

    for (int j = 0; j < this->m_groupWorldBorders.size(); j++) {
        CAaBox &groupAAbb = m_groupWorldBorders[j];

        //2. Update the world group BB
        AABB.min = mathfu::vec3_packed(mathfu::vec3(std::min(groupAAbb.min.x,AABB.min.x),
                                                         std::min(groupAAbb.min.y,AABB.min.y),
                                                         std::min(groupAAbb.min.z,AABB.min.z)));

        AABB.max = mathfu::vec3_packed(mathfu::vec3(std::max(groupAAbb.max.x,AABB.max.x),
                                                         std::max(groupAAbb.max.y,AABB.max.y),
                                                         std::max(groupAAbb.max.z,AABB.max.z)));
    }

    syncBBoxComponent();
}

void WmoObject::recalcGroupBorders(int groupId) {
    if (mainGeom == nullptr || groupId < 0 || groupId >= mainGeom->groupsLen) return;
    if (m_groupWorldBorders.size() < mainGeom->groupsLen) return;

    auto &groupObject = this->groupObjects[groupId];

    //Volume box: use the group file bounds once the group geometry is loaded,
    //the (coarser) group table bounds otherwise
    CAaBox localBox = mainGeom->groups[groupId].bounding_box;
    HWmoGroupGeom groupGeom = groupObject != nullptr ? groupObject->getWmoGroupGeom() : nullptr;
    if (groupGeom != nullptr && groupGeom->getStatus() == FileStatus::FSLoaded) {
        localBox = groupGeom->mogp->boundingBox;
    }

    mathfu::vec4 bbMin(localBox.min.x, localBox.min.y, localBox.min.z, 1.0f);
    mathfu::vec4 bbMax(localBox.max.x, localBox.max.y, localBox.max.z, 1.0f);
    CAaBox volumeWorld = MathHelper::transformAABBWithMat4(m_placementMatrix, bbMin, bbMax);

    //The world border additionally includes all loaded doodad M2s of the group
    //(mirrors the old WmoGroupObject::updateWorldGroupBBWithM2)
    CAaBox world = volumeWorld;
    if (groupObject != nullptr && groupObject->getIsLoaded()) {
        for (auto &m2Object : groupObject->getDoodads()) {
            if (m2Object == nullptr || !m2Object->isMainDataLoaded()) continue;

            CAaBox m2AAbb = m2Object->getAABB();
            world.min = mathfu::vec3_packed(mathfu::vec3(std::min(m2AAbb.min.x, world.min.x),
                                                         std::min(m2AAbb.min.y, world.min.y),
                                                         std::min(m2AAbb.min.z, world.min.z)));
            world.max = mathfu::vec3_packed(mathfu::vec3(std::max(m2AAbb.max.x, world.max.x),
                                                         std::max(m2AAbb.max.y, world.max.y),
                                                         std::max(m2AAbb.max.z, world.max.z)));
        }
    }

    m_groupVolumeWorldBorders[groupId] = volumeWorld;
    m_groupWorldBorders[groupId] = world;

    //Refresh the whole-WMO bbox (unions the per-group world borders)
    updateBB();
}

CAaBox WmoObject::getAABB() {
    return this->m_bbox;
}

void WmoObject::createMaterialCache() {
    m_materialCache = decltype(m_materialCache)(mainGeom->materialsLen);
}

void WmoObject::postWmoGroupObjectLoad(int groupId, int lod) {
    //1. Create portal verticles from geometry
}

void WmoObject::checkGroupDoodads(int groupId, mathfu::vec4 &cameraVec4,
                                  std::vector<mathfu::vec4> &frustumPlane,
                                  M2ObjectListContainer &m2Candidates) {
    std::shared_ptr<WmoGroupObject> groupWmoObject = groupObjects[groupId];
    if (groupWmoObject != nullptr && groupWmoObject->getIsLoaded()) {

        for (auto &m2Object : groupWmoObject->getDoodads()) {
            if (!m2Object) continue;

            m2Candidates.addCandidate(m2Object);
        }
    }
}

bool WmoObject::startTraversingWMOGroup(
    mathfu::vec4 &cameraVec4,
    const MathHelper::FrustumCullingData &frustumDataGlobal,
    int groupId,
    int globalLevel,
    int &renderOrder,
    bool traversingFromInterior,
    FrameViewsHolder &viewsHolder
) {
    ZoneScoped;
    if (!m_loaded)
        return false;

    uint32_t portalCount = (uint32_t) std::max(0, this->mainGeom->portalsLen);

    //Camera inside a portal-less interior group: no culling work is needed at all
    if (portalCount == 0 && groupId > -1 && groupId < mainGeom->groupsLen &&
        mainGeom->groups[groupId].flags.INTERIOR && !mainGeom->groups[groupId].flags.EXTERIOR_CULL && !mainGeom->groups[groupId].flags.ALWAYSDRAW)
    {
        auto nextGroupObject = groupObjects[groupId];

        auto interiorView = viewsHolder.createInterior(frustumDataGlobal);
        interiorView->ownerGroupWMO = groupObjects[groupId];
        interiorView->wmoGroupArray.addToDraw(nextGroupObject);
        interiorView->wmoGroupArray.addToCheckM2(nextGroupObject);

        return true;
    }

    //Batch-cull per-group world bounding boxes against the global frustum. The results drive
    //the per-group exterior loops below and gate the portal traversal descent.
    const int groupsLen = mainGeom->groupsLen;
    m_groupWorldVisScratch.resize(groupsLen);
    m_groupVolumeVisScratch.resize(groupsLen);
#if (__AVX__ && __SSE2__)
    AabbArrayCullingSSE::cull(frustumDataGlobal, cameraVec4, m_groupWorldBorders.data(), groupsLen, m_groupWorldVisScratch.data());
    AabbArrayCullingSSE::cull(frustumDataGlobal, cameraVec4, m_groupVolumeWorldBorders.data(), groupsLen, m_groupVolumeVisScratch.data());
#else
    AabbArrayCulling::cull(frustumDataGlobal, cameraVec4, m_groupWorldBorders.data(), groupsLen, m_groupWorldVisScratch.data());
    AabbArrayCulling::cull(frustumDataGlobal, cameraVec4, m_groupVolumeWorldBorders.data(), groupsLen, m_groupVolumeVisScratch.data());
#endif

    auto resolveGroupVisibility = [&](int i, bool &drawDoodads, bool &drawGroup) {
        if (!this->groupObjects[i]->getIsLoaded()) {
            //Not loaded yet: force draw so the group gets queued for load
            //(same as WmoGroupObject::checkGroupFrustum did for this case)
            drawDoodads = true;
            drawGroup = true;
        } else {
            drawDoodads = m_groupWorldVisScratch[i] != AabbArrayCulling::CULLED;
            drawGroup = m_groupVolumeVisScratch[i] != AabbArrayCulling::CULLED;
        }
    };

    if (portalCount == 0) {
        auto exteriorView = viewsHolder.getOrCreateExterior(frustumDataGlobal);
        bool result = false;
        for (int i = 0; i< groupsLen; i++) {
            if ((mainGeom->groups[i].flags.EXTERIOR) > 0 || (mainGeom->groups[i].flags.EXTERIOR_CULL) > 0 || mainGeom->groups[i].flags.ALWAYSDRAW || !m_api->getConfig()->usePortalCulling) { //exterior
                if (this->groupObjects[i] != nullptr) {
                    bool drawDoodads, drawGroup;
                    resolveGroupVisibility(i, drawDoodads, drawGroup);
                    if (drawDoodads) {
                        exteriorView->wmoGroupArray.addToCheckM2(this->groupObjects[i]);
                    }
                    if (drawGroup) {
                        exteriorView->wmoGroupArray.addToDraw(this->groupObjects[i]);
                    }
                    result |= drawGroup;
                }
            }
        }
        return result;
    }
    framebased::vector<HInteriorView> ivPerWMOGroup = framebased::vector<HInteriorView>(groupsLen);

    framebased::vector<PortalTraversalWorkItem> portalWorkList;
    framebased::vector<framebased::vector<mathfu::vec4>> portalCrossedFrustums =
        framebased::vector<framebased::vector<mathfu::vec4>>(portalCount);

    //CurrentVisibleM2 and visibleWmo is array of global m2 objects, that are visible after frustum
    mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4;
    mathfu::mat4 transposeInverseModelMat = m_placementInvertMatrix.Transpose();

    //For interior cull
    mathfu::mat4 MVPMat = frustumDataGlobal.perspectiveMat*frustumDataGlobal.viewMat*this->m_placementMatrix;
    mathfu::mat4 MVPMatInv = MVPMat.Inverse();

    framebased::vector<mathfu::vec4> frustumPlanesLocal = MathHelper::getFrustumClipsFromMatrix(MVPMat);

    auto globalPlane = frustumDataGlobal.frustums[0].planes[frustumDataGlobal.frustums[0].planes.size() - 2];
    // auto altFarPlane = this->m_placementMatrix.Transpose() * globalPlane;
    PortalTraverseTempData traverseTempData = {
        viewsHolder,
        viewsHolder.getExterior() != nullptr,
        frustumPlanesLocal[frustumPlanesLocal.size() - 1], //farPlane is always last one,
        ivPerWMOGroup,
        cameraVec4,
        cameraLocal,
        transposeInverseModelMat,
        MVPMat,
        MVPMatInv,
        portalWorkList,
        portalCrossedFrustums,
        m_groupWorldVisScratch
    };

    if (traversingFromInterior && m_api->getConfig()->usePortalCulling) {
        traverseTempData.atLeastOneGroupIsDrawn = true;

        std::shared_ptr<WmoGroupObject> nextGroupObject = groupObjects[groupId];
        if (nextGroupObject->getIsLoaded() && nextGroupObject->getWmoGroupGeom()->mogp->flags2.isSplitGroupChild) {
            groupId = nextGroupObject->getWmoGroupGeom()->mogp->parentSplitOrFirstChildGroupIndex;
            nextGroupObject = groupObjects[groupId];
        }

        auto interiorView = ivPerWMOGroup[groupId];
        //5.1 The portal is into interior wmo group. So go on.
        if (interiorView == nullptr) {
            interiorView = viewsHolder.createInterior(frustumDataGlobal);
            ivPerWMOGroup[groupId] = interiorView;
            interiorView->ownerGroupWMO = groupObjects[groupId];
            interiorView->wmoGroupArray.addToDraw(nextGroupObject);
            interiorView->wmoGroupArray.addToCheckM2(nextGroupObject);
        }

        if (nextGroupObject->getIsLoaded() && nextGroupObject->getWmoGroupGeom()->mogp->flags2.isSplitGroupParent) {
            this->addSplitChildWMOsToView(*interiorView, groupId);
        }

        if (globalLevel+1 >= interiorView->level) {
            interiorView->level = globalLevel + 1;
        } else {
            assert("BVH is not working. Something is wrong!");
        }

        portalWorkList.push_back({groupId, frustumPlanesLocal, globalLevel, 0});
    } else {
        auto exteriorView = viewsHolder.getOrCreateExterior(frustumDataGlobal);

        if (globalLevel+1 >= exteriorView->level) {
            exteriorView->level = globalLevel + 1;
        } else {
            assert("BVH is not working. Something is wrong!");
        }

        for (int i = 0; i< mainGeom->groupsLen; i++) {
            if ((mainGeom->groups[i].flags.EXTERIOR) > 0 || !m_api->getConfig()->usePortalCulling) { //exterior
                if (this->groupObjects[i] != nullptr) {
                    bool drawDoodads, drawGroup;
                    resolveGroupVisibility(i, drawDoodads, drawGroup);
                    if (drawDoodads) {
                        exteriorView->wmoGroupArray.addToCheckM2(this->groupObjects[i]);
                    }
                    if (drawGroup) {
                        exteriorView->wmoGroupArray.addToDraw(this->groupObjects[i]);
                        traverseTempData.atLeastOneGroupIsDrawn = true;
                        if (m_api->getConfig()->usePortalCulling && portalCount > 0) {
                            portalWorkList.push_back({i, frustumPlanesLocal, globalLevel, 0});
                        }
                    }
                }
            }
        }
    }

    drainPortalTraversal(traversingFromInterior, traverseTempData);

    //Add all ALWAYSRENDER to Exterior
    for (int i = 0; i< mainGeom->groupsLen; i++) {
        if ((mainGeom->groups[i].flags.ALWAYSDRAW) > 0) { //exterior
            auto exteriorView = viewsHolder.getOrCreateExterior(frustumDataGlobal);
            exteriorView->wmoGroupArray.addToDraw(this->groupObjects[i]);
        }

        if (m_api->getConfig()->renderAntiPortals) {
            auto exteriorView = viewsHolder.getOrCreateExterior(frustumDataGlobal);
            if (!this->groupObjects[i]->getIsLoaded()) {
                exteriorView->wmoGroupArray.addToLoad(this->groupObjects[i]);
            } else {
                if ((mainGeom->groups[i].flags.ANTIPORTAL) > 0) { //ANTIPORTAL
                    if (this->groupObjects[i]->getIsLoaded()) {
                        exteriorView->worldAntiPortalVertices.emplace_back() =
                            createAntiPortal(this->groupObjects[i]->getWmoGroupGeom(), m_placementMatrix);
                    }
                }
            }
        }
    }

    //Process results
    ivPerWMOGroup.erase(std::remove(ivPerWMOGroup.begin(), ivPerWMOGroup.end(), nullptr), ivPerWMOGroup.end());
    std::sort(ivPerWMOGroup.begin(), ivPerWMOGroup.end(), [](const HInteriorView &a, const HInteriorView &b) -> bool {
        if (a->level != b->level) {
            return a->level < b->level;
        }
        return false;
    });

    for (auto &createdInteriorView : ivPerWMOGroup) {
        createdInteriorView->renderOrder = renderOrder++;
    }

    bool result = ivPerWMOGroup.size() > 0;
    {
        auto exterior = viewsHolder.getExterior();

        if (exterior != nullptr) {
            exterior->renderOrder = renderOrder++;
            result = true;
        }
    }

    //M2s will be collected later from separate function call
    return traverseTempData.atLeastOneGroupIsDrawn;
}

void WmoObject::triggerExteriorGroupLoads(WMOGroupListContainer &wmoGroupArray) {
    const bool portalCullingOff = !m_api->getConfig()->usePortalCulling;
    const bool hasPortals = mainGeom->portalsLen > 0;

    for (int i = 0; i < mainGeom->groupsLen; i++) {
        auto &flags = mainGeom->groups[i].flags;
        //Same group filters as the exterior branches of startTraversingWMOGroup
        bool groupIsLoadTriggered = hasPortals
            ? ((flags.EXTERIOR > 0) || portalCullingOff)
            : ((flags.EXTERIOR > 0) || (flags.EXTERIOR_CULL > 0) || (flags.ALWAYSDRAW > 0) || portalCullingOff);
        if (!groupIsLoadTriggered) continue;

        auto &groupObject = this->groupObjects[i];
        if (groupObject != nullptr && !groupObject->getIsLoaded()) {
            //addToDraw routes not-loaded groups to the load list, same as the force-drawn
            //path of WmoGroupObject::checkGroupFrustum for unloaded groups
            wmoGroupArray.addToDraw(groupObject);
        }
    }
}
void WmoObject::addSplitChildWMOsToView(InteriorView &interiorView, int groupId) {
    if (!groupObjects[groupId]->getIsLoaded())
        return;

    auto &parentMogp = groupObjects[groupId]->getWmoGroupGeom()->mogp;
    if (!parentMogp->flags2.isSplitGroupParent)
        return;

    int nextChildGroupIndex = parentMogp->parentSplitOrFirstChildGroupIndex;
    while (nextChildGroupIndex != -1) {
        auto &groupWmo = groupObjects[nextChildGroupIndex];
        if (!groupWmo->getIsLoaded()) {
            interiorView.wmoGroupArray.addToLoad(groupWmo);
            return;
        }

        auto &mogp = groupWmo->getWmoGroupGeom()->mogp;
        if (!mogp->flags2.isSplitGroupChild)
            break;

        interiorView.wmoGroupArray.addToDraw(groupWmo);
        interiorView.wmoGroupArray.addToCheckM2(groupWmo);

        nextChildGroupIndex = mogp->nextSplitGroupChildIndex;
    }

}


static const float dotepsilon = pow(1.5f, 2.0f);
static bool frustumPlanesAlmostEqual(const framebased::vector<mathfu::vec4> &a,
                                     const framebased::vector<mathfu::vec4> &b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        for (int c = 0; c < 4; c++) {
            float av = a[i][c], bv = b[i][c];
            float scale = std::max(std::max(std::fabs(av), std::fabs(bv)), 1.0f);
            if (std::fabs(av - bv) > 1e-3f * scale) return false;
        }
    }
    return true;
}

void WmoObject::drainPortalTraversal(
    bool traversingStartedFromInterior,
    PortalTraverseTempData &traverseTempData
) {
    auto &portalWorkList = traverseTempData.portalWorkList;

    //Breadth-first over the portal graph: items are processed in arrival order, so a group's
    //first arrival is always a shortest portal path (keeps interiorView->level bookkeeping
    //consistent). A group may be enqueued several times with different frustums (diamond paths
    //like 1->2 and 1->3->2); every distinct frustum contribution is propagated onward exactly
    //once, duplicates are suppressed by portalCrossedFrustums.
    for (size_t workIndex = 0; workIndex < portalWorkList.size(); workIndex++) {
        //Copy the item out: the worklist grows inside the loop
        PortalTraversalWorkItem workItem = portalWorkList[workIndex];

        if (workItem.localLevel > 8) continue;

        int groupId = workItem.groupId;
        auto &localFrustumPlanes = workItem.frustumPlanes;
        int globalLevel = workItem.globalLevel;

        if (groupObjects[groupId] == nullptr || !groupObjects[groupId]->getIsLoaded()) {
            //The group has not been loaded yet
            continue;
        }

        //2. Loop through portals of current group

        int moprIndex = groupObjects[groupId]->getWmoGroupGeom()->mogp->moprIndex;
        int numItems = groupObjects[groupId]->getWmoGroupGeom()->mogp->moprCount;

        if (groupObjects[groupId]->getWmoGroupGeom()->mogp->flags.showSkyBox) {
            if (groupObjects[groupId]->getWmoGroupGeom()->mogp->flags.INTERIOR > 0 || !m_api->getConfig()->usePortalCulling) {
                if (traversingStartedFromInterior && skyBox != nullptr) {
                    traverseTempData.viewsHolder.getSkybox()->m2List.addToDraw(skyBox);
                }
            } else {
                //TODO: WHAT ????
                //Example: main wmo: 850548, group WMO 901743
            }
        }

        for (int j = moprIndex; j < moprIndex+numItems; j++) {
            const SMOPortalRef * relation = &mainGeom->portalReferences[j];
            const SMOPortal * portalInfo = &mainGeom->portals[relation->portal_index];

            int nextGroup = relation->group_index;
            const C4Plane &plane = portalInfo->plane;

            //Skip portals into groups whose world box (incl. doodads) is outside the global
            //frustum: the portal lies on the group's boundary, so it cannot be visible either.
            //Saves the planeCull and portal-frustum construction work below.
            if (nextGroup >= 0 && nextGroup < traverseTempData.groupWorldVisible.size() &&
                traverseTempData.groupWorldVisible[nextGroup] == AabbArrayCulling::CULLED) continue;

            //Local coordinanes plane DOT local camera
            const mathfu::vec4 planeV4 = mathfu::vec4(plane.planeVector);
            float dotResult = mathfu::vec4::DotProduct(planeV4, traverseTempData.cameraLocal);
            //dotResult = dotResult + relation.side * 0.01;
            bool isInsidePortalThis = (relation->side < 0) ? (dotResult <= 0) : (dotResult >= 0);

            //This condition checks if camera is very close to the portal. In this case the math doesnt work very properly
            //So I need to make this hack exactly for this case.z

            bool hackCondition = (fabs(dotResult) > dotepsilon);

            if (!isInsidePortalThis && hackCondition) continue;

            //2.1 If portal has less than 4 vertices - skip it(invalid?)
            if (portalInfo->index_count < 4) continue;

            //2.2 Check if Portal BB made from portal vertexes intersects frustum
            auto &portalSortedVertices = geometryPerPortal[relation->portal_index].sortedVericles;
            framebased::vector<mathfu::vec3> portalVerticesVec;
            portalVerticesVec.reserve(portalSortedVertices.size());

            std::transform(
                portalSortedVertices.begin(),
                portalSortedVertices.end(),
                std::back_inserter(portalVerticesVec),
                [](mathfu::vec3 &d) -> mathfu::vec3 { return d;}
                );

            bool visible = MathHelper::planeCull(portalVerticesVec, localFrustumPlanes);

            if (!visible && hackCondition) continue;

            int lastFrustumPlanesLen = localFrustumPlanes.size();

            //3. Construct frustum planes for this portal
            framebased::vector<mathfu::vec4> thisPortalPlanes;
            thisPortalPlanes.reserve(lastFrustumPlanesLen);

            if (hackCondition) {
                //Transform portal vertexes into clip space (this code should allow to use this logic with both Perspective and Ortho)
                framebased::vector<mathfu::vec4> portalVerticesClip(portalVerticesVec.size());
                framebased::vector<mathfu::vec4> portalVerticesClipNearPlane(portalVerticesVec.size());

                for (int i = 0; i < portalVerticesVec.size(); i++) {
                    portalVerticesClip[i] = traverseTempData.MVPMat * mathfu::vec4(portalVerticesVec[i], 1.0f);
                    portalVerticesClip[i] /= portalVerticesClip[i].w;
                }



                for (int i = 0; i < portalVerticesVec.size(); i++) {
                    //Project Portal vertex to near plane in clip space
                    portalVerticesClipNearPlane[i] = portalVerticesClip[i];
                    portalVerticesClipNearPlane[i].z = -1;
                    //Transform back to local space
                    portalVerticesClipNearPlane[i] = traverseTempData.MVPMatInv * portalVerticesClipNearPlane[i];
                    portalVerticesClipNearPlane[i] /= portalVerticesClipNearPlane[i].w;
                }

                //This condition works, bcuz earlier it's verified we are inside the portal using the
                //dot product AND `relation->side`
                bool flip = (relation->side > 0);

                int vertexCnt = portalVerticesVec.size();
                for (int i = 0; i < vertexCnt; ++i) {
                    int i2 = (i + 1) % vertexCnt;

                    mathfu::vec4 n = MathHelper::createPlaneFromEyeAndVertexes(portalVerticesClipNearPlane[i].xyz(),
                                                                               portalVerticesVec[i],
                                                                               portalVerticesVec[i2]);

                    if (flip) {
                        n *= -1.0f;
                    }

                    thisPortalPlanes.emplace_back() = n;
                }
                //The portalPlanes do not have far and near plane. So we need to add them
                //Near plane is this portal's plane, far plane is a global one
                auto nearPlane = mathfu::vec4(portalInfo->plane.planeVector);
                if (flip)
                    nearPlane *= -1.0f;

                auto &farPlane = traverseTempData.farPlane;
                thisPortalPlanes.emplace_back() = nearPlane;
                thisPortalPlanes.emplace_back() = farPlane;
            } else {
                // If camera is too close - just use usual frustums
                thisPortalPlanes = localFrustumPlanes;
            }

            //Suppress duplicate contributions on cyclic portal paths: if this portal already
            //produced an equivalent frustum, propagating it again adds nothing new
            if (frustumPlanesAlmostEqual(traverseTempData.portalCrossedFrustums[relation->portal_index], thisPortalPlanes))
                continue;
            traverseTempData.portalCrossedFrustums[relation->portal_index] = thisPortalPlanes;

            //Transform local planes into world planes to use with frustum culling of M2Objects
            MathHelper::PlanesUndPoints worldSpaceFrustum;
            worldSpaceFrustum.planes = framebased::vector<mathfu::vec4>(thisPortalPlanes.size());
            worldSpaceFrustum.points = framebased::vector<mathfu::vec3>();
            worldSpaceFrustum.points.reserve(thisPortalPlanes.size());

            for (int x = 0; x < thisPortalPlanes.size(); x++) {
                worldSpaceFrustum.planes[x] = traverseTempData.transposeInverseModelMat * thisPortalPlanes[x];
            }

            {
                worldSpaceFrustum.points = MathHelper::getIntersectionPointsFromPlanes(worldSpaceFrustum.planes);
                //worldSpaceFrustum.hullLines = MathHelper::getHullLines(worldSpaceFrustum.points);
            }

            std::vector<mathfu::vec3> worldSpacePortalVertices;
            worldSpacePortalVertices.reserve(thisPortalPlanes.size());
            std::transform(portalVerticesVec.begin(), portalVerticesVec.end(),
                           std::back_inserter(worldSpacePortalVertices),
                           [&](mathfu::vec3 &p) -> mathfu::vec3 {
                               return (this->m_placementMatrix * mathfu::vec4(p, 1.0f)).xyz();
                           }
            );

            //5. Traverse next
            std::shared_ptr<WmoGroupObject> &nextGroupObject = groupObjects[nextGroup];
            const SMOGroupInfo &nextGroupInfo = mainGeom->groups[nextGroup];
            if ((nextGroupInfo.flags.EXTERIOR) == 0) {
                auto &interiorView = traverseTempData.ivPerWMOGroup[nextGroup];
                //5.1 The portal is into interior wmo group. So go on.
                if (interiorView == nullptr) {
                    interiorView = traverseTempData.viewsHolder.createInterior({});
                    traverseTempData.ivPerWMOGroup[nextGroup] = interiorView;

                    interiorView->ownerGroupWMO = nextGroupObject;
                    interiorView->wmoGroupArray.addToDraw(nextGroupObject);
                    interiorView->wmoGroupArray.addToCheckM2(nextGroupObject);
                    interiorView->portalIndexes.emplace_back() = relation->portal_index;
                }

                interiorView->worldPortalVertices.emplace_back() = worldSpacePortalVertices;
                interiorView->frustumData.frustums.emplace_back() = worldSpaceFrustum;
                //BFS processes groups in non-decreasing level order, so the first (smallest)
                //level wins; later arrivals only grow it
                if (globalLevel+1 >= interiorView->level) {
                    interiorView->level = globalLevel + 1;
                }

                if (nextGroupObject->getIsLoaded() && nextGroupObject->getWmoGroupGeom()->mogp->flags2.isSplitGroupParent) {
                    this->addSplitChildWMOsToView(*interiorView, nextGroup);
                }

                portalWorkList.push_back({nextGroup, thisPortalPlanes, globalLevel + 1, workItem.localLevel + 1});

            } else if (((nextGroupInfo.flags.EXTERIOR) > 0) && traversingStartedFromInterior) {
                //5.2 The portal is from interior into exterior wmo group.
                //Makes sense to try to create or get only if exterior was not already created before traversing this entire WMO
                if (!traverseTempData.exteriorWasCreatedBeforeTraversing) {
                    auto exteriorView = traverseTempData.viewsHolder.getOrCreateExterior({});
                    exteriorView->level = globalLevel + 1;
                    exteriorView->worldPortalVertices.emplace_back() = worldSpacePortalVertices;
                    exteriorView->frustumData.frustums.emplace_back() = worldSpaceFrustum;
                }
            }
        }
    }
}

bool WmoObject::isGroupWmoInterior(int groupId) {
    const SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.EXTERIOR) == 0);
    return result;
}

bool WmoObject::isGroupWmoExteriorLit(int groupId) {
    const SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.EXTERIOR_LIT) == 1) || ((groupInfo->flags.EXTERIOR) == 1) || ((groupInfo->flags.INTERIOR) == 0);
    return result;
}

bool WmoObject::isGroupWmoExtSkybox(int groupId) {
    const SMOGroupInfo *groupInfo = &this->mainGeom->groups[groupId];
    bool result = ((groupInfo->flags.SHOW_EXTERIOR_SKYBOX) == 1);
    return result;
}

bool WmoObject::getGroupWmoThatCameraIsInside (mathfu::vec4 cameraVec4, WmoGroupResult &groupResult, float &bottomBorder) {
    if (this->groupObjects.size() ==0) return false;

    //Transform camera into local coordinates
    mathfu::vec4 cameraLocal = this->m_placementInvertMatrix * cameraVec4 ;

    //Check if camera inside wmo
    const auto &mainGeomBB = this->mainGeom->header->bounding_box;
    bool isInsideWMOBB = (
        cameraLocal[0] > mainGeomBB.min.x && cameraLocal[0] < mainGeomBB.max.x &&
        cameraLocal[1] > mainGeomBB.min.y && cameraLocal[1] < mainGeomBB.max.y &&
        cameraLocal[2] > mainGeomBB.min.z && cameraLocal[2] < mainGeomBB.max.z
    );
    if (!isInsideWMOBB) return false;

    float localBorder = (this->m_placementInvertMatrix * mathfu::vec4(cameraVec4.x,cameraVec4.y,bottomBorder, 1.0f)).z;

    //Loop
    int wmoGroupsInside = 0;
    int interiorGroups = 0;
    int lastWmoGroupInside = -1;
    std::vector<WmoGroupResult> candidateGroups;

    for (int i = 0; i < this->groupObjects.size(); i++) {
        if (this->groupObjects[i] == nullptr) continue;
        this->groupObjects[i]->checkIfInsideGroup(
                cameraVec4,
                cameraLocal,
                this->mainGeom->portal_vertices,
                this->mainGeom->portals,
                this->mainGeom->portalReferences,
                candidateGroups);
    }

    //6. Iterate through result group list and find the one with maximal bottom z coordinate for object position
    float minDist = 999999;
    bool result = false;
    for (int i = 0; i < candidateGroups.size(); i++) {
        WmoGroupResult *candidate = &candidateGroups[i];
        const SMOGroupInfo *groupInfo = &this->mainGeom->groups[candidate->groupIndex];
        /*if ((candidate.topBottom.bottomZ < 99999) && (candidate.topBottom.topZ > -99999)){
            if ((cameraLocal[2] < candidateGroups[i].topBottom.bottomZ) || (cameraLocal[2] > candidateGroups[i].topBottom.topZ))
                continue
        } */
        if (cameraLocal[2] < candidate->topBottom.min) continue;

        if (candidate->topBottom.min < 99999 && candidate->topBottom.min > localBorder) {
            float dist = cameraLocal[2] - candidate->topBottom.min;
            if (dist > 0 && dist < minDist) {
                result = true;
                minDist = dist;
                groupResult = *candidate;

                localBorder = candidate->topBottom.min;
            }
        }
    }

    bottomBorder = (this->m_placementMatrix * mathfu::vec4(cameraLocal.x,cameraLocal.y,localBorder, 1.0f)).z;

    return result;
}

std::string WmoObject::getModelFileName() {
    return m_modelName;
}
void WmoObject::setModelFileName(std::string modelName) {
    m_modelName = modelName;
}
int WmoObject::getModelFileId() {
   return m_modelFileId;
}
void WmoObject::setModelFileId(int fileId) {
    useFileId = true;
    m_modelFileId = fileId;
}

void WmoObject::fillLodGroup(mathfu::vec3 &cameraLocal) {
    for(int i = 0; i < mainGeom->groupsLen; i++) {
        if (drawGroupWMO[i]) {
            float distance = MathHelper::distanceFromAABBToPoint(groupObjects[i]->getLocalAABB(), cameraLocal);
            if (distance > 800) {
                lodGroupLevelWMO[i] = 2;
            } else if (distance > 500) {
                lodGroupLevelWMO[i] = 1;
            } else {
                lodGroupLevelWMO[i] = 0;
            }

            lodGroupLevelWMO[i] = std::min(lodGroupLevelWMO[i], getWmoHeader()->numLod-1);
        } else {
            lodGroupLevelWMO[i] = 0;
        }
    }

}
float distance(C4Plane &plane, C3Vector vertex) {
    return (float) sqrt(mathfu::vec4::DotProduct(mathfu::vec4(plane.planeVector), mathfu::vec4(vertex.x, vertex.y, vertex.z, 1.0)));
}


void attenuateTransVerts(WmoMainGeom &mainGeom, WmoGroupGeom& wmoGroupGeom) {

    if (!wmoGroupGeom.mogp->transBatchCount)
    {
        return;
    }

//    for ( std::size_t vertex_index (0); vertex_index < wmoGroupGeom.batches[wmoGroupGeom.mogp->transBatchCount-1].last_vertex; ++vertex_index) {
//        float opacity_accum (0.0);
//
//        for ( std::size_t portal_ref_index (wmoGroupGeom.mogp->moprIndex);
//              portal_ref_index < (wmoGroupGeom.mogp->moprIndex + wmoGroupGeom.mogp->moprCount);
//              ++portal_ref_index)
//        {
//            SMOPortalRef const& portalRef (mainGeom->portalReferences[portal_ref_index]);
//            SMOPortal const& portal (mainGeom->portals[portalRef.portal_index]);
//            C3Vector const& vertex (wmoGroupGeom.verticles[vertex_index]);
//
//            float const portal_to_vertex (distance(portal.plane, vertex));
//
//            C3Vector vertex_to_use (vertex);
//
//            if (portal_to_vertex > 0.001 || portal_to_vertex < -0.001)
//            {
//                C3Ray ray ( C3Ray::FromStartEnd
//                                    ( vertex
//                                            , vertex
//                                              + (portal_to_vertex > 0 ? -1 : 1) * portal.plane.planeGeneral.normal
//                                            , 0
//                                    )
//                );
//                NTempest::Intersect
//                        (ray, &portal.plane, 0LL, &vertex_to_use, 0.0099999998);
//            }
//
//            float distance_to_use;
//
//            if ( NTempest::Intersect ( vertex_to_use
//                    , &mainGeom->portal_vertices[portal.base_index]
//                    , portal.index_count
//                    , C3Vector::MajorAxis (portal.plane.normal)
//            )
//                    )
//            {
//                distance_to_use = portalRef.side * distance (portal.plane, vertex);
//            }
//            else
//            {
//                distance_to_use = NTempest::DistanceFromPolygonEdge
//                        (vertex, &mainGeom->portal_vertices[portal.base_index], portal.index_count);
//            }
//
//            if (mainGeom->groups[portalRef.group_index].flags.EXTERIOR ||
//                mainGeom->groups[portalRef.group_index].flags.EXTERIOR_LIT)
//            {
//                float v25 (distance_to_use >= 0.0 ? distance_to_use / 6.0f : 0.0f);
//                if ((1.0 - v25) > 0.001)
//                {
//                    opacity_accum += 1.0 - v25;
//                }
//            }
//            else if (distance_to_use > -1.0)
//            {
//                opacity_accum = 0.0;
//                if (distance_to_use < 1.0)
//                {
//                    break;
//                }
//            }
//        }
//
//        float const opacity ( opacity_accum > 0.001
//                              ? std::min (1.0f, opacity_accum)
//                              : 0.0f
//        );
//
//        //! \note all assignments asserted to be > -0.5 && < 255.5f
//        CArgb& color (wmoGroupGeom.colorArray[vertex_index]);
//        color.r = (unsigned char) (((127.0f - color.r) * opacity) + color.r);
//        color.g = (unsigned char) (((127.0f - color.g) * opacity) + color.g);
//        color.b = (unsigned char) (((127.0f - color.b) * opacity) + color.b);
//        color.a = opacity * 255.0;
//    }
}

std::function<void(WmoGroupGeom &wmoGroupGeom)> WmoObject::getAttenFunction() {
    std::weak_ptr<WmoMainGeom> l_mainGeom = this->mainGeom;
    return [l_mainGeom](  WmoGroupGeom &wmoGroupGeom ) -> void {
        auto s_mainGeom = l_mainGeom.lock();
        if (!s_mainGeom) return;

        attenuateTransVerts(*s_mainGeom, wmoGroupGeom);
    } ;
}

namespace {
    inline float wmoFogRadialWeight(float dist, float smallerRadius, float largerRadius) {
        // 1.0 at smaller_radius, fading linearly to 0.0 at larger_radius
        float d = dist >= 0.0f ? std::min<float>(dist, largerRadius) : 0.0f;
        if (d < smallerRadius) return 1.0f;
        return 1.0f - (d - smallerRadius) / (largerRadius - smallerRadius);
    }

    inline mathfu::vec3 imVectorToVec3(const CImVector &color) {
        return mathfu::vec3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
    }

    inline CImVector vec3ToImVector(const mathfu::vec3 &v) {
        auto toByte = [](float c) -> unsigned char {
            c = std::min<float>(std::max<float>(c, 0.0f), 1.0f);
            return (unsigned char)(c * 255.0f + 0.5f);
        };
        CImVector result;
        result.b = toByte(v.z);
        result.g = toByte(v.y);
        result.r = toByte(v.x);
        result.a = 255;
        return result;
    }

    // Per-channel lerp of the BGR(A) color, alpha of dst is kept
    inline CImVector lerpImVector(const CImVector &dst, const CImVector &src, float alpha) {
        int a = (int)(alpha * 255.0f + 0.5f);
        if (a <= 0) return dst;
        if (a >= 255) {
            CImVector result = dst;
            result.b = src.b; result.g = src.g; result.r = src.r;
            return result;
        }
        auto lerpCh = [a](unsigned char d, unsigned char s) -> unsigned char {
            return (unsigned char)(d + ((a * ((int)s - (int)d)) >> 8));
        };
        CImVector result = dst;
        result.b = lerpCh(dst.b, src.b);
        result.g = lerpCh(dst.g, src.g);
        result.r = lerpCh(dst.r, src.r);
        return result;
    }

    inline void copyFogFlags(SMOFog &dst, const SMOFog &src) {
        // flags are the first dword of SMOFog
        reinterpret_cast<uint32_t &>(dst) = reinterpret_cast<const uint32_t &>(src);
    }

    float distanceToPortalPolygon(const mathfu::vec3 &p, const std::vector<mathfu::vec3> &poly) {
        const float maxDist = std::numeric_limits<float>::max();
        if (poly.size() < 3) return maxDist;

        mathfu::vec3 normal = mathfu::vec3::CrossProduct(poly[1] - poly[0], poly[2] - poly[0]);
        float normalLen = normal.Length();
        if (normalLen < 1e-6f) return maxDist;
        normal = normal * (1.0f / normalLen);

        float planeDist = mathfu::vec3::DotProduct(normal, p - poly[0]);
        mathfu::vec3 proj = p - normal * planeDist;

        bool inside = true;
        for (size_t i = 0; i < poly.size(); i++) {
            const mathfu::vec3 &a = poly[i];
            const mathfu::vec3 &b = poly[(i + 1) % poly.size()];
            mathfu::vec3 edge = b - a;
            if (mathfu::vec3::DotProduct(mathfu::vec3::CrossProduct(edge, proj - a), normal) < 0.0f) {
                inside = false;
                break;
            }
        }
        if (inside) return fabsf(planeDist);

        float minDist = maxDist;
        for (size_t i = 0; i < poly.size(); i++) {
            const mathfu::vec3 &a = poly[i];
            const mathfu::vec3 &b = poly[(i + 1) % poly.size()];
            mathfu::vec3 ab = b - a;
            float l2 = ab.LengthSquared();
            float t = l2 > 0.0f ? std::min<float>(std::max<float>(mathfu::vec3::DotProduct(p - a, ab) / l2, 0.0f), 1.0f) : 0.0f;
            minDist = std::min<float>(minDist, (p - (a + ab * t)).Length());
        }
        return minDist;
    }
}

// Computes the WMO fog that applies at the camera position.
// - fog record #0 is the base; fogs referenced by the current group's fogIndicies are blended in
// - flag 0x1000 on record #0 selects weighted-average blending, otherwise fogs are lerped
//   sequentially from farthest to nearest with a radial falloff weight
// - also reports whether the camera is inside a non-exterior(-lit) group and the distance
//   to the nearest portal of that group (used by the caller as the WMO fog blend weight)
void WmoObject::checkFog(const mathfu::vec3 &cameraPos, int currentGroupIndex, WmoFogBlendResult &result) {
    result = WmoFogBlendResult();

    if (!m_loaded || mainGeom == nullptr || mainGeom->fogsLen == 0)
        return;

    mathfu::vec3 cameraLocal = (m_placementInvertMatrix * mathfu::vec4(cameraPos, 1.0)).xyz();

    // 1. Interior state + distance to the nearest portal of the interior group
    MOGP *mogp = nullptr;
    if (currentGroupIndex >= 0 && currentGroupIndex < (int)groupObjects.size()) {
        auto &groupObj = groupObjects[currentGroupIndex];
        if (groupObj != nullptr && groupObj->getIsLoaded() && groupObj->getWmoGroupGeom() != nullptr) {
            mogp = groupObj->getWmoGroupGeom()->mogp;
        }
    }

    float distToExit = std::numeric_limits<float>::max();
    bool insideInterior = false;
    if (mogp != nullptr && !mogp->flags.EXTERIOR && !mogp->flags.EXTERIOR_LIT) {
        insideInterior = true;
        for (int i = mogp->moprIndex; i < mogp->moprIndex + mogp->moprCount; i++) {
            if (i >= mainGeom->portalReferencesLen) break;
            int portalIndex = mainGeom->portalReferences[i].portal_index;
            if (portalIndex >= (int)geometryPerPortal.size()) continue;
            distToExit = std::min<float>(distToExit,
                distanceToPortalPolygon(cameraLocal, geometryPerPortal[portalIndex].sortedVericles));
        }
    }
    if (!insideInterior)
        distToExit = 0.0f;

    result.insideInterior = insideInterior;
    result.distToExit = distToExit;

    // 2. Base fog = record #0; bail out when the WMO fog must not apply (client logic)
    const SMOFog &fog0 = mainGeom->fogs[0];
    bool hasFogVolume = fog0.flag_0x1000 != 0;
    if ((mainGeom->fogsLen == 1 && !hasFogVolume) ||
        (fog0.flag_0x1000 && fog0.flag_0x10000)) {
        return;
    }

    result.fog = fog0;

    // 3. Fog records referenced by the current group (index 0 is the default, never referenced).
    // When the group is unknown (standalone WMO scenes) fall back to all fog records.
    std::vector<int> fogIndices;
    if (mogp != nullptr) {
        for (int i = 0; i < 4; i++) {
            int idx = mogp->fogIndicies[i];
            if (idx > 0 && idx < mainGeom->fogsLen)
                fogIndices.push_back(idx);
        }
    } else {
        for (int idx = 1; idx < mainGeom->fogsLen; idx++)
            fogIndices.push_back(idx);
    }

    // 4. Blend
    if (hasFogVolume) {
        // Weighted-average path
        // Weights are the radial falloff; record #0 fills the remainder to 1.0;
        // resulting flags come from the nearest referenced fog record.
        float weightSum = 0.0f;
        float endAcc = 0.0f, startAcc = 0.0f, uwEndAcc = 0.0f, uwStartAcc = 0.0f;
        mathfu::vec3 colorAcc(0.0f, 0.0f, 0.0f), uwColorAcc(0.0f, 0.0f, 0.0f);

        int nearestIndex = 0;
        float nearestDist = std::numeric_limits<float>::max();
        for (int idx : fogIndices) {
            float dist = (mathfu::vec3(mainGeom->fogs[idx].pos) - cameraLocal).Length();
            if (dist < nearestDist) {
                nearestDist = dist;
                nearestIndex = idx;
            }
        }

        for (int idx : fogIndices) {
            const SMOFog &rec = mainGeom->fogs[idx];
            float dist = (mathfu::vec3(rec.pos) - cameraLocal).Length();
            if (dist >= rec.larger_radius || rec.flag_infinite_radius)
                continue;

            float w = wmoFogRadialWeight(dist, rec.smaller_radius, rec.larger_radius);
            weightSum += w;
            endAcc += rec.fog.end * w;
            startAcc += rec.fog.start_scalar * w;
            colorAcc += imVectorToVec3(rec.fog.color) * w;
            uwEndAcc += rec.underwater_fog.end * w;
            uwStartAcc += rec.underwater_fog.start_scalar * w;
            uwColorAcc += imVectorToVec3(rec.underwater_fog.color) * w;
        }

        if (weightSum < 1.0f) {
            float w = 1.0f - weightSum;
            endAcc += fog0.fog.end * w;
            startAcc += fog0.fog.start_scalar * w;
            colorAcc += imVectorToVec3(fog0.fog.color) * w;
            uwEndAcc += fog0.underwater_fog.end * w;
            uwStartAcc += fog0.underwater_fog.start_scalar * w;
            uwColorAcc += imVectorToVec3(fog0.underwater_fog.color) * w;
            weightSum = 1.0f;
        }

        float inv = 1.0f / weightSum;
        result.fog.fog.end = endAcc * inv;
        result.fog.fog.start_scalar = startAcc * inv;
        result.fog.fog.color = vec3ToImVector(colorAcc * inv);
        result.fog.underwater_fog.end = uwEndAcc * inv;
        result.fog.underwater_fog.start_scalar = uwStartAcc * inv;
        result.fog.underwater_fog.color = vec3ToImVector(uwColorAcc * inv);

        if (nearestIndex != 0)
            copyFogFlags(result.fog, mainGeom->fogs[nearestIndex]);
    } else {
        // Sequential path containing fogs are lerped into record #0 from farthest to nearest
        struct FogCand {
            int index;
            float dist;
        };
        std::vector<FogCand> candidates;
        for (int idx : fogIndices) {
            const SMOFog &rec = mainGeom->fogs[idx];
            float dist = (mathfu::vec3(rec.pos) - cameraLocal).Length();
            if (dist < rec.larger_radius && !rec.flag_infinite_radius)
                candidates.push_back({idx, dist});
        }
        std::sort(candidates.begin(), candidates.end(), [](const FogCand &a, const FogCand &b) {
            return a.dist > b.dist;
        });

        for (const FogCand &cand : candidates) {
            const SMOFog &rec = mainGeom->fogs[cand.index];
            float w = wmoFogRadialWeight(cand.dist, rec.smaller_radius, rec.larger_radius);

            result.fog.fog.end = (rec.fog.end - result.fog.fog.end) * w + result.fog.fog.end;
            result.fog.fog.start_scalar = (rec.fog.start_scalar - result.fog.fog.start_scalar) * w + result.fog.fog.start_scalar;
            result.fog.fog.color = lerpImVector(result.fog.fog.color, rec.fog.color, w);
            result.fog.underwater_fog.end = (rec.underwater_fog.end - result.fog.underwater_fog.end) * w + result.fog.underwater_fog.end;
            result.fog.underwater_fog.start_scalar = (rec.underwater_fog.start_scalar - result.fog.underwater_fog.start_scalar) * w + result.fog.underwater_fog.start_scalar;
            result.fog.underwater_fog.color = lerpImVector(result.fog.underwater_fog.color, rec.underwater_fog.color, w);

            copyFogFlags(result.fog, rec);
        }
    }

    result.fogFound = true;
}

bool WmoObject::hasPortals() {
    return mainGeom->header->nPortals != 0;
}

SMOHeader *WmoObject::getWmoHeader() {
    return mainGeom->header;
}

PointerChecker<SMOLight> &WmoObject::getLightArray() {
    return mainGeom->lights;
}

PointerChecker<SMOMaterial> &WmoObject::getMaterials() {
    return mainGeom->materials;
}

std::shared_ptr<IWMOMaterial> WmoObject::getMaterialInstance(int materialIndex, const HMapSceneBufferCreate &sceneRenderer) {
    assert(materialIndex < m_materialCache.size());

    auto materialInstance = m_materialCache[materialIndex];
    if (materialInstance != nullptr)
        return materialInstance;

    //Otherwise create goddamit material

    const SMOMaterial &material = getMaterials()[materialIndex];
    assert(material.shader < MAX_WMO_SHADERS && material.shader >= 0);
    auto shaderId = material.shader;
    if (shaderId >= MAX_WMO_SHADERS) {
        shaderId = 0;
    }

    int pixelShader = wmoMaterialShader[shaderId].pixelShader;
    int vertexShader = wmoMaterialShader[shaderId].vertexShader;

    auto blendMode = material.blendMode;

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = blendMode <= 1;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = !(material.flags.F_UNCULLED);

    pipelineTemplate.blendMode = static_cast<EGxBlendEnum>(blendMode);

    pipelineTemplate.stencilTestEnable = false;
    pipelineTemplate.stencilWrite = true;
    pipelineTemplate.stencilWriteVal = ObjStencilValues::WMO_STENCIL_VAL;

    bool isSecondTextSpec = material.shader == 8;

    HGSamplableTexture texture1 = getTexture(material.diffuseNameIndex, false);
    HGSamplableTexture texture2 = getTexture(material.envNameIndex, isSecondTextSpec);
    HGSamplableTexture texture3 = getTexture(material.texture_2, false);

    WMOMaterialTemplate materialTemplate;

    materialTemplate.textures[0] = texture1;
    materialTemplate.textures[1] = texture2;
    materialTemplate.textures[2] = texture3;

    if (pixelShader == (int)WmoPixelShader::MapObjParallax) {
        materialTemplate.textures[3] = getTexture(material.color_2, false);
        materialTemplate.textures[4] = getTexture(material.flags_2, false);
        materialTemplate.textures[5] = getTexture(material.runTimeData[0], false);
    } else if (pixelShader == (int)WmoPixelShader::MapObjDFShader) {
        materialTemplate.textures[3] = getTexture(material.color_2, false);
        materialTemplate.textures[4] = getTexture(material.flags_2, false);
        materialTemplate.textures[5] = getTexture(material.runTimeData[0], false);
        materialTemplate.textures[6] = getTexture(material.runTimeData[1], false);
        materialTemplate.textures[7] = getTexture(material.runTimeData[2], false);
        materialTemplate.textures[8] = getTexture(material.runTimeData[3], false);
    }

    materialInstance = sceneRenderer->createWMOMaterial(m_wmoModelChunk, pipelineTemplate, materialTemplate);
    m_materialCache[materialIndex] = materialInstance;

    C4Vector matUVAnim = C4Vector(mathfu::vec4(0.0f,0.0f,0.0f,0.0f));
    if (mainGeom->materialUVSpeedLen > 0 && (materialIndex < mainGeom->materialUVSpeedLen)) {
        matUVAnim = mainGeom->materialUVSpeed[materialIndex];
    }
    {
        float alphaTest = (blendMode > 0) ? 0.00392157f : -1.0f;

        //Update VS block
        auto &blockVS = materialInstance->m_materialVS->getObject();
        blockVS.UseLitColor = (material.flags.F_UNLIT > 0) ? 0 : 1;
        blockVS.VertexShader = vertexShader;
        blockVS.translationSpeedXY = matUVAnim;
        materialInstance->m_materialVS->save();

        //Update PS block
        auto &blockPS = materialInstance->m_materialPS->getObject();
        blockPS.UseLitColor = (material.flags.F_UNLIT > 0) ? 0 : 1;
        blockPS.EnableAlpha = (blendMode > 0) ? 1 : 0;
        blockPS.PixelShader = pixelShader;
        blockPS.BlendMode = blendMode;
        blockPS.uFogColor_AlphaTest = mathfu::vec4_packed(
            mathfu::vec4(0,0,0, alphaTest));
        materialInstance->m_materialPS->save();
    }

    return materialInstance;
};

std::shared_ptr<M2Object> WmoObject::getSkyBoxForGroup(int groupNum) {
    if (!m_loaded) return nullptr;
    if (groupNum < 0 || groupNum >= this->groupObjects.size()) return nullptr;
    if (!this->groupObjects[groupNum]->getIsLoaded()) return nullptr;
    if (!this->groupObjects[groupNum]->getWmoGroupGeom()->mogp->flags.showSkyBox) return nullptr;

    return skyBox;
}

WmoObject::~WmoObject() {
}

int WmoObject::getWmoGroupId(int groupNum) {
    if (!m_loaded) return 0;
    if (!groupObjects[groupNum]->getIsLoaded()) return 0;

    return groupObjects[groupNum]->getWmoGroupGeom()->mogp->wmoGroupID;
}

std::array<mathfu::vec3,3> WmoObject::getAmbientColors() {
    return m_ambientColors;
}

void WmoObject::createNewLights() {
    m_newLights.resize(mainGeom->newLightsLen);
    for (int i = 0; i < mainGeom->newLightsLen; i++) {
        auto &newLightRec = mainGeom->newLights[i];
        //It seems doodadset is not affecting the lights. Oh well

        // if (!m_activeDoodadSets[newLightRec.doodadSet]) {
        //     m_newLights[i] = nullptr;
        //     std::cout << "DoodadSet " << newLightRec.doodadSet << " is not active. Skipping new wmo light" << std::endl;
        //     continue;
        // }
        m_newLights[i] = std::make_shared<CEngineLight>(m_placementMatrix, mainGeom->newLights[i]);
    }
}
void WmoObject::calculateAmbient() {
    mathfu::vec3 ambientColor;
    mathfu::vec3 horizontAmbientColor;
    mathfu::vec3 groundAmbientColor;

    if (mainGeom->mavgsLen > 0) {
        //Take ambient from MAVG
        int recordIndex = 0;
        for (int i = 0; i < mainGeom->mavgsLen; i++) {
            if (m_activeDoodadSets[mainGeom->mavgs[i].doodadSetID]) {
                recordIndex = i;
                break;
            }
        }
        auto &record = mainGeom->mavgs[recordIndex];
        if ((record.flags & 1) != 0) {
            ambientColor = ImVectorToVec4(record.color1).xyz();
            horizontAmbientColor = ImVectorToVec4(record.color2).xyz();
            groundAmbientColor = ImVectorToVec4(record.color3).xyz();
        } else {
            auto amb = ImVectorToVec4(record.color1).xyz();
            ambientColor = amb;
            horizontAmbientColor = amb;
            groundAmbientColor = amb;
        }
    } else if (mainGeom->mavdsLen > 0) {
        //Take ambient from MAVD
        auto const &mavds = mainGeom->mavds;
        if ((mavds->flags & 1) != 0) {
            ambientColor = ImVectorToVec4(mavds->color1).xyz();
            horizontAmbientColor = ImVectorToVec4(mavds->color2).xyz();
            groundAmbientColor = ImVectorToVec4(mavds->color3).xyz();
        } else {
            auto amb = ImVectorToVec4(mavds->color1).xyz();
            ambientColor = amb;
            horizontAmbientColor = amb;
            groundAmbientColor = amb;
        }
    } else if (mainGeom->header) {
        auto amb = ImVectorToVec4(mainGeom->header->ambColor).xyz();
        ambientColor = amb;
        horizontAmbientColor = amb;
        groundAmbientColor = amb;
    }

    m_ambientColors = {ambientColor, horizontAmbientColor, groundAmbientColor};
    m_groupInteriorData = decltype(m_groupInteriorData)(groupObjects.size());
    for (auto &interiorAmbientBlock : m_groupInteriorData) {
         interiorAmbientBlock.uAmbientColorAndIsExteriorLit = mathfu::vec4(ambientColor, 1.0f);
         interiorAmbientBlock.uHorizontAmbientColor = mathfu::vec4(horizontAmbientColor, 1.0f);
         interiorAmbientBlock.uGroundAmbientColor = mathfu::vec4(groundAmbientColor, 1.0f);
    }
    m_interiorAmbientsChanged = true;
}

std::shared_ptr<CEngineLight> WmoObject::getNewLight(int index) {
    if (index > m_newLights.size())
        return nullptr;

    return m_newLights[index];
}
void WmoObject::setInteriorAmbientColor(int groupIndex,
                                        bool isExteriorLighted,
                                        const mathfu::vec3 &ambient,
                                        const mathfu::vec3 &horizontAmbient,
                                        const mathfu::vec3 &groundAmbient)
{
    assert(groupIndex < m_groupInteriorData.size());

    auto &interiorAmbientBlock = m_groupInteriorData[groupIndex];
    interiorAmbientBlock.uAmbientColorAndIsExteriorLit = mathfu::vec4(ambient, isExteriorLighted ? 1.0 : 0.0);
    interiorAmbientBlock.uHorizontAmbientColor = mathfu::vec4(horizontAmbient, 1.0f);
    interiorAmbientBlock.uGroundAmbientColor   = mathfu::vec4(groundAmbient, 1.0f);

    m_interiorAmbientsChanged = true;
}


std::shared_ptr<WMOEntityFactory> wmoFactory = std::make_shared<WMOEntityFactory>();