//
// Created by deamon on 22.06.17.
//

#include <locale>
#include "m2Object.h"
#include "../algorithms/mathHelper.h"
#include "../managers/animationManager.h"


void M2Object::createAABB() {
    M2Data *m2Data = m_m2Geom->getM2Data();

    C3Vector min = m2Data->bounding_box.min;
    C3Vector max = m2Data->bounding_box.max;
    mathfu::vec4 minVec = mathfu::vec4(min.x, min.y, min.z, 1);
    mathfu::vec4 maxVec = mathfu::vec4(max.x, max.y, max.z, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, minVec, maxVec);

    //this.diameter = vec3.distance(worldAABB[0],worldAABB[1]);
    this->aabb = worldAABB;
}

void M2Object::createPlacementMatrix(SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat) {
    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix = placementMatrix * wmoPlacementMat;
    placementMatrix = placementMatrix * mathfu::mat4::FromTranslationVector(mathfu::vec3(def.position));

    mathfu::quat quat4(def.orientation.w, def.orientation.x, def.orientation.y, def.orientation.z);
    placementMatrix = placementMatrix * quat4.ToMatrix4();

    float scale = def.scale;
    placementMatrix = placementMatrix * mathfu::mat4::FromScaleVector(mathfu::vec3(scale,scale,scale));

    mathfu::mat4 invertPlacementMatrix = placementMatrix.Inverse();

    m_placementMatrix = placementMatrix;
    m_placementInvertMatrix = invertPlacementMatrix;
}

void M2Object::createPlacementMatrix(SMDoodadDef &def) {
    const float TILESIZE = 533.333333333;

    float posx = def.position.x;
    float posy = def.position.y;
    float posz = def.position.z;

    mathfu::mat4 adtToWorldMat4 = MathHelper::getAdtToWorldMat4();
    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();

    placementMatrix *= adtToWorldMat4;
    placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(posx, posy, posz));
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(-1, 1, -1));

    placementMatrix *= MathHelper::RotationY(toRadian(def.rotation.y-270));
    placementMatrix *= MathHelper::RotationZ(toRadian(-def.rotation.x));
    placementMatrix *= MathHelper::RotationX(toRadian(def.rotation.z-90));

    float scale = def.scale / 1024;
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(scale, scale, scale));

    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;
}

void M2Object::calcDistance(mathfu::vec3 cameraPos) {
    m_currentDistance = (m_worldPosition-cameraPos).Length();
}

float M2Object::getCurrentDistance() {
    return m_currentDistance;
}

void M2Object::setLoadParams (std::string modelName, int skinNum, std::vector<uint8_t> meshIds, std::vector<std::string> replaceTextures) {
    modelName;
    this->m_skinNum = skinNum;
    this->m_meshIds = meshIds;
    this->m_replaceTextures = replaceTextures;

    std::locale loc = std::locale("");

    std::string delimiter = ".";
    std::string nameTemplate = modelName.substr(0, modelName.find_last_of(delimiter));
    std::string modelFileName = nameTemplate + ".m2";
    std::string skinFileName = nameTemplate + "00.skin";
//    std::for_each(modelFileName.begin(), modelFileName.end(), [](char & c) {
//        c = ::tolower(c);
//    });
//    std::for_each(skinFileName.begin(), skinFileName.end(), [](char & c) {
//        c = ::tolower(c);
//    });

    this->m_modelName = modelFileName;
    this->m_skinName = skinFileName;
}

void M2Object::startLoading() {
    if (!m_loading) {
        m_loading = true;

        Cache<M2Geom> *m2GeomCache =  m_api->getM2GeomCache();
        Cache<SkinGeom> *skinGeomCache = m_api->getSkinGeomCache();

        m_m2Geom = m2GeomCache->get(m_modelName);
        m_skinGeom = skinGeomCache->get(m_skinName);
    }
}

void M2Object::update(double deltaTime, mathfu::vec3 cameraPos, mathfu::mat4 viewMat) {
    if (!this->m_loaded) {
        if ((m_m2Geom != nullptr) && m_m2Geom->isLoaded() && (m_skinGeom != nullptr) && m_skinGeom->isLoaded()) {
            this->m_loaded = true;
            this->m_loading = false;

            this->createAABB();
            this->makeTextureArray();
            this->initAnimationManager();
            this->initBoneAnimMatrices();
        } else {
            return;
        }
    };


//    var invPlacementMat = this.getInvertModelMatrix();
//
//    //if (!this.materialArray) return;
//
//    /* 1. Calc local camera */
    mathfu::vec4 cameraInlocalPos = mathfu::vec4(cameraPos, 0);
    //cameraInlocalPos = invPlacementMat* cameraInlocalPos;
//
//    /* 2. Update animation values */
    this->m_animationManager->update(deltaTime, cameraPos,
                                     this->bonesMatrices,
        this->textAnimMatrices,
        this->subMeshColors,
        this->transparencies
        //this->cameras,
        //this->lights,
        //this->particleEmittersArray
    );
//
//    for (var i = 0; i < this.lights.length; i++) {
//    var light = this.lights[i];
//    vec4.transformMat4(light.position, light.position, this.placementMatrix);
//    vec4.transformMat4(light.position, light.position, viewMat);
//    }
//
//    this.combinedBoneMatrix = this.combineBoneMatrixes();
//
//    this.currentTime += deltaTime;
}

//
//void M2Object::load() {
//    var m2Promise = this.sceneApi.resources.loadM2Geom(modelFileName);
//    var skinPromise = this.sceneApi.resources.loadSkinGeom(skinFileName);
//
//    return $q.all([m2Promise, skinPromise]).then(function (result) {
//        try {
//            var m2Geom = result[0];
//            var skinGeom = result[1];
//
//            self.m2Geom = m2Geom;
//            self.skinGeom = skinGeom;
//
//            skinGeom.fixData(m2Geom.m2File);
//            skinGeom.calcBBForSkinSections(m2Geom.m2File);
//
//            if (!m2Geom) {
//                $log.log("m2 file failed to load : " + modelName);
//            } else {
//                var gl = self.sceneApi.getGlContext();
//                m2Geom.createVAO(skinGeom);
//                self.hasBillboarded = self.checkIfHasBillboarded();
//
//                self.makeTextureArray(self.meshIds, self.replaceTextures);
//                self.updateLocalBB([self.m2Geom.m2File.BoundingCorner1, self.m2Geom.m2File.BoundingCorner2]);
//
//                self.createAABB();
//
//                self.initAnimationManager(m2Geom.m2File);
//                self.initParticleSystem();
//                self.initBoneAnimMatrices();
//                self.initSubmeshColors();
//                self.initTextureAnimMatrices();
//                self.initTransparencies();
//                self.initCameras();
//                self.initLights();
//
//                self.postLoad();
//                self.loaded = true;
//            }
//        } catch (e) {
//            console.log("exception while loading M2", e)
//        }
//
//        return true;
//    });
//}

bool M2Object::checkFrustumCulling (mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints) {
    if (!m_loaded) {
        return true;
    }

    CAaBox &aabb = this->aabb;

    //1. Check if camera position is inside Bounding Box
    if (
        cameraPos[0] > aabb.min.z && cameraPos[0] < aabb.max.x &&
        cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y &&
        cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
    ) return true;

    //2. Check aabb is inside camera frustum
    bool result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);
    return result;
}

void M2Object::draw(bool drawTransparent, mathfu::vec4 diffuseColor) {
    if (!this->m_loaded) {
        this->startLoading();
        return;
    }

//    var vaoBinded = this.m2Geom.bindVao();
//    if (!vaoBinded) {
        this->m_m2Geom->setupAttributes();
        this->m_skinGeom->setupAttributes();
//    }

//    var combinedMatrix = this.combinedBoneMatrix;
//    std::vector<mathfu::mat4> combinedMatrix(120, mathfu::mat4::Identity());

    this->m_m2Geom->setupUniforms(this->m_api, m_placementMatrix, bonesMatrices, diffuseColor, drawTransparent);

    this->drawMeshes(drawTransparent, -1);

//    if (vaoBinded) {
//        this.m2Geom.unbindVao()
//    }
}

void M2Object::drawMeshes(bool drawTransparent, int instanceCount) {
    if (!drawTransparent) {
        for (int i = 0; i < this->m_materialArray.size(); i++) {
            auto materialData = this->m_materialArray[i];
            this->drawMaterial(materialData, drawTransparent, instanceCount);
        }
    } else {
        for (int i = (int) (this->m_materialArray.size() - 1); i >= 0; i--) {
            auto materialData = this->m_materialArray[i];
            this->drawMaterial(materialData, drawTransparent, instanceCount);
        }
    }
}

void M2Object::drawMaterial(M2MaterialInst &materialData, bool drawTransparent, int instanceCount) {
    if (!(materialData.isTransparent ^ !drawTransparent)) return;
//    /*
//    var meshIdsTobeRendered = window.meshestoBeRendered;
//    if (meshIdsTobeRendered && !meshIdsTobeRendered[materialData.texUnit1TexIndex]) return;
//    if (window.shownLayer != null && materialData.layer != window.shownLayer ) return;
//      */
//
    mathfu::mat4 identMat = mathfu::mat4::Identity();

    //mathfu::vec4 originalFogColor = this.sceneApi.getFogColor();
    mathfu::vec4 originalFogColor = mathfu::vec4(1,1,1,1);
//
//
    /* Get right texture animation matrix */
    mathfu::mat4 textureMatrix1 = identMat;
    mathfu::mat4 textureMatrix2 = identMat;
    auto skinData = this->m_skinGeom->getSkinData();
    auto m2Data = this->m_m2Geom->getM2Data();
    if (materialData.texUnit1TexIndex >= 0) {
        auto textureAnim = skinData->batches[materialData.texUnit1TexIndex]->textureTransformComboIndex;
        uint16_t textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnim];
        if (textureMatIndex >= 0 && textureMatIndex < this->textAnimMatrices.size()) {
            textureMatrix1 = this->textAnimMatrices[textureMatIndex];
        }
        if (materialData.texUnit2TexIndex >= 0 && (textureAnim+1 < m2Data->texture_transforms_lookup_table.size)) {
            int textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnim+1];
            if (textureMatIndex >= 0 && textureMatIndex < this->textAnimMatrices.size()) {
                textureMatrix2 = this-> textAnimMatrices[textureMatIndex];
            }
        }
    }
//    var meshColor = this.getCombinedColor(skinData, materialData, this.subMeshColors);
//    var transparency = this.getTransparency(skinData, materialData, this.transparencies);
    mathfu::vec4 meshColor = mathfu::vec4(1,1,1,1);
    float transparency = 1;
//
//    //Don't draw meshes with 0 transp
    if ((transparency < 0.0001) || (meshColor[3] < 0.0001)) return;
//
//
//    int pixelShaderIndex = pixelShaderTable[materialData.shaderNames.pixel];
    int pixelShaderIndex = 0;
    this->m_m2Geom->drawMesh(m_api, materialData, *skinData , meshColor, transparency, textureMatrix1, textureMatrix2, pixelShaderIndex, originalFogColor, instanceCount);
}

void M2Object::makeTextureArray() {
    /* 1. Free previous subMeshArray */
    auto textureCache = m_api->getTextureCache();

    /* 2. Fill the materialArray */
    auto subMeshes = m_skinGeom->getSkinData()->submeshes;
    M2Array<M2Batch>* batches = &m_skinGeom->getSkinData()->batches;

    auto m2File = m_m2Geom->getM2Data();

    for (int i = 0; i < batches->size; i++) {
        M2MaterialInst materialData;

        M2Batch* skinTextureDefinition = batches->getElement(i);
        auto subMesh = subMeshes[skinTextureDefinition->skinSectionIndex];

        if ((this->m_meshIds.size() > 0) && (subMesh->skinSectionId > 0) &&
                (m_meshIds[(subMesh->skinSectionId / 100)] != (subMesh->skinSectionId % 100))) {
            continue;
        }
//        materialArray.push(materialData);

        auto op_count = skinTextureDefinition->textureCount;

        auto renderFlagIndex = skinTextureDefinition->materialIndex;
        //var isTransparent = (mdxObject.m2File.renderFlags[renderFlagIndex].blend >= 2);
        auto isTransparent = (m2File->materials[renderFlagIndex]->blending_mode >= 2) ||
                            ((m2File->materials[renderFlagIndex]->flags & 0x10) > 0);

//        var shaderNames = this.getShaderNames(skinTextureDefinition);

        materialData.layer = skinTextureDefinition->materialLayer;
        materialData.isRendered = true;
        materialData.isTransparent = isTransparent;
        materialData.meshIndex = skinTextureDefinition->skinSectionIndex;
        materialData.renderFlagIndex = skinTextureDefinition->materialIndex;
        materialData.flags = skinTextureDefinition->flags;
//        materialData.shaderNames = shaderNames;
//        materialData.m2BatchIndex = i;


//        materialData.renderFlag =  m2File->materials[renderFlagIndex]->flags;
//        materialData.renderBlending = m2File->materials[renderFlagIndex]->blending_mode;

        int textureUnit;
        if (skinTextureDefinition->textureCoordComboIndex <= m2File->tex_unit_lookup_table.size) {
            textureUnit = *m2File->tex_unit_lookup_table[skinTextureDefinition->textureCoordComboIndex];
            if (textureUnit == 0xFFFF) {
                //Enviroment mapping
                materialData.isEnviromentMapping = true;
            }
        }

        if (op_count > 0) {
            auto mdxTextureIndex = *m2File->texture_lookup_table[skinTextureDefinition->textureComboIndex];
            M2Texture* mdxTextureDefinition = m2File->textures[mdxTextureIndex];
            materialData.texUnit1TexIndex = i;
            materialData.mdxTextureIndex1 = mdxTextureIndex;
            materialData.xWrapTex1 = (mdxTextureDefinition->flags & 1) > 0;
            materialData.yWrapTex1 = (mdxTextureDefinition->flags & 2) > 0;

            if (mdxTextureDefinition->type == 0) {
                materialData.textureUnit1TexName = mdxTextureDefinition->filename.toString();
            } else if (mdxTextureDefinition->type < this->m_replaceTextures.size()){
                materialData.textureUnit1TexName = this->m_replaceTextures[mdxTextureDefinition->type];
            }
        }
        if (op_count > 1) {
            auto mdxTextureIndex = *m2File->texture_lookup_table[skinTextureDefinition->textureComboIndex + 1];
            M2Texture* mdxTextureDefinition = m2File->textures[mdxTextureIndex];
            materialData.texUnit2TexIndex = i;
            materialData.mdxTextureIndex2 = mdxTextureIndex;
            materialData.xWrapTex2 = (mdxTextureDefinition->flags & 1) > 0;
            materialData.yWrapTex2 = (mdxTextureDefinition->flags & 2) > 0;

            if (mdxTextureDefinition->type == 0) {
                materialData.textureUnit2TexName = mdxTextureDefinition->filename.toString();
            } else if (mdxTextureDefinition->type < this->m_replaceTextures.size()){
                materialData.textureUnit2TexName = this->m_replaceTextures[mdxTextureDefinition->type];
            }
        }
        if (op_count > 2) {
            auto mdxTextureIndex = *m2File->texture_lookup_table[skinTextureDefinition->textureComboIndex + 1];
            M2Texture* mdxTextureDefinition = m2File->textures[mdxTextureIndex];
            materialData.texUnit3TexIndex = i;
            materialData.mdxTextureIndex3 = mdxTextureIndex;
            materialData.xWrapTex3 = (mdxTextureDefinition->flags & 1) > 0;
            materialData.yWrapTex3 = (mdxTextureDefinition->flags & 2) > 0;

            if (mdxTextureDefinition->type == 0) {
                materialData.textureUnit3TexName = mdxTextureDefinition->filename.toString();
            } else if (mdxTextureDefinition->type < this->m_replaceTextures.size()){
                materialData.textureUnit3TexName = this->m_replaceTextures[mdxTextureDefinition->type];
            }
        }

        this->m_materialArray.push_back(materialData);
    }

    for (int i = 0; i < this->m_materialArray.size(); i++) {
        M2MaterialInst* materialData = &this->m_materialArray[i];
        if (materialData->textureUnit1TexName.size()>1) {
            materialData->texUnit1Texture = textureCache->get(materialData->textureUnit1TexName);
        }
        if (materialData->textureUnit2TexName.size()>1) {
            materialData->texUnit2Texture = textureCache->get(materialData->textureUnit2TexName);
        }
        if (materialData->textureUnit3TexName.size()>1) {
            materialData->texUnit3Texture = textureCache->get(materialData->textureUnit3TexName);
        }
    }
}

void M2Object::initAnimationManager() {
    this->m_animationManager = new AnimationManager(m_m2Geom->getM2Data());
}

void M2Object::initBoneAnimMatrices() {
    this->bonesMatrices = std::vector<mathfu::mat4>(m_m2Geom->getM2Data()->bones.size, mathfu::mat4::Identity());;
}

