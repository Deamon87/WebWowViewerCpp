//
// Created by deamon on 22.06.17.
//

#include <locale>
#include "m2Object.h"
#include "../algorithms/mathHelper.h"


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

void M2Object::setLoadParams (std::string modelName, int skinNum, std::vector<uint8_t> meshIds, std::vector<std::string> replaceTextures) {
    modelName;
    this->m_skinNum = skinNum;
    this->m_meshIds = meshIds;
    this->m_replaceTextures = replaceTextures;

    std::locale loc = std::locale::global(std::locale("en_US.utf8"));

    std::string delimiter = ".";
    std::string nameTemplate = modelName.substr(0, modelName.find(delimiter));
    std::string modelFileName = nameTemplate + ".m2";
    std::string skinFileName = nameTemplate + "00.skin";
    modelFileName = std::tolower(modelFileName, loc);
    skinFileName = std::tolower(skinFileName, loc);

    this->m_modelName = modelFileName;
    this->m_skinName = skinFileName;
}

void M2Object::startLoading() {
    if (!m_loading) {
        m_loading = true;

        Cache<M2Geom> *m2GeomCache =  m_api->getM2GeomCache();
        Cache<SkinGeom> *skinGeomCache = m_api->getSkinGeomCache();

        m_m2Geom = m2GeomCache->get(m_modelName);
        m_skinGeom = skinGeomCache->get(m_modelName);
    }
}

void M2Object::update(int deltaTime, mathfu::vec3 cameraPos, mathfu::mat4 viewMat) {
    if (!this->m_loaded) {
        if (m_m2Geom->isLoaded() && m_skinGeom->isLoaded()) {
            this->m_loaded = true;
        } else {
            return;
        }
    };


//    var invPlacementMat = this.getInvertModelMatrix();
//
//    //if (!this.materialArray) return;
//
//    /* 1. Calc local camera */
//    var cameraInlocalPos = vec4.create();
//    vec4.copy(cameraInlocalPos, cameraPos);
//    vec4.transformMat4(cameraInlocalPos, cameraInlocalPos, invPlacementMat);
//
//    /* 2. Update animation values */
//    this.animationManager.update(deltaTime, cameraInlocalPos, this.bonesMatrices, this.textAnimMatrices,
//    this.subMeshColors, this.transparencies, this.cameras, this.lights, this.particleEmittersArray);
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

void M2Object::draw(bool drawTransparent, mathfu::mat4 placementMatrix, mathfu::vec4 diffuseColor) {
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
    std::vector<mathfu::mat4> combinedMatrix(60, mathfu::mat4::Identity());

    this->m_m2Geom->setupUniforms(this->m_api, placementMatrix, combinedMatrix, diffuseColor, drawTransparent);

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
        for (int i = this->m_materialArray.size()-1; i >=0; i--) {
            auto materialData = this->m_materialArray[i];
            this->drawMaterial(materialData, drawTransparent, instanceCount);
        }
    }
}

void M2Object::drawMaterial(M2MaterialInst &materialData, bool drawTransparent, int instanceCount) {
//    if (!(materialData.isTransparent ^ !drawTransparent)) return;
//    /*
//    var meshIdsTobeRendered = window.meshestoBeRendered;
//    if (meshIdsTobeRendered && !meshIdsTobeRendered[materialData.texUnit1TexIndex]) return;
//    if (window.shownLayer != null && materialData.layer != window.shownLayer ) return;
//      */
//
//    var identMat = mat4.create();
//    mat4.identity(identMat);
//    var originalFogColor = this.sceneApi.getFogColor();
//
//
//
//    /* Get right texture animation matrix */
//    var textureMatrix1 = identMat;
//    var textureMatrix2 = identMat;
//    var skinData = this.skinGeom.skinFile.header;
//    if (materialData.texUnit1TexIndex >= 0 && skinData.texs[materialData.texUnit1TexIndex]) {
//    var textureAnim = skinData.texs[materialData.texUnit1TexIndex].textureAnim;
//    var textureMatIndex = this.m2Geom.m2File.texAnimLookup[textureAnim];
//    if (textureMatIndex !== undefined && this.textAnimMatrices && textureMatIndex >= 0 && textureMatIndex <  this.textAnimMatrices.length) {
//    textureMatrix1 = this.textAnimMatrices[textureMatIndex];
//    }
//    if (materialData.texUnit2TexIndex >= 0) {
//    var textureMatIndex = this.m2Geom.m2File.texAnimLookup[textureAnim+1];
//    if (textureMatIndex !== undefined && this.textAnimMatrices && textureMatIndex >= 0 && textureMatIndex <  this.textAnimMatrices.length) {
//    textureMatrix2 = this.textAnimMatrices[textureMatIndex];
//    }
//    }
//    }
//    var meshColor = this.getCombinedColor(skinData, materialData, this.subMeshColors);
//    var transparency = this.getTransparency(skinData, materialData, this.transparencies);
//
//    //Don't draw meshes with 0 transp
//    if ((transparency < 0.0001) || (meshColor[3] < 0.0001)) return;
//
//    var pixelShaderIndex = pixelShaderTable[materialData.shaderNames.pixel];
//    this.m2Geom.drawMesh(materialData, this.skinGeom, meshColor, transparency, textureMatrix1, textureMatrix2, pixelShaderIndex, originalFogColor, instanceCount)
}