//
// Created by deamon on 22.06.17.
//

#include <locale>
#include <regex>
#include "m2Object.h"
#include "../algorithms/mathHelper.h"
#include "../managers/animationManager.h"
#include "../../../3rdparty/mathfu/include/mathfu/matrix.h"
#include "../persistance/header/M2FileHeader.h"

std::unordered_map<std::string, int> pixelShaderTable = {
        {"Combiners_Opaque", 0},
        {"Combiners_Decal" , 1},
        {"Combiners_Add" , 2},
        {"Combiners_Mod2x" , 3},
        {"Combiners_Fade" , 4},
        {"Combiners_Mod" , 5},
        {"Combiners_Opaque_Opaque" , 6},
        {"Combiners_Opaque_Add" , 7},
        {"Combiners_Opaque_Mod2x" , 8},
        {"Combiners_Opaque_Mod2xNA" , 9},
        {"Combiners_Opaque_AddNA" , 10},
        {"Combiners_Opaque_Mod" , 11},
        {"Combiners_Mod_Opaque" , 12},
        {"Combiners_Mod_Add" , 13},
        {"Combiners_Mod_Mod2x" , 14},
        {"Combiners_Mod_Mod2xNA" , 15},
        {"Combiners_Mod_AddNA" , 16},
        {"Combiners_Mod_Mod" , 17},
        {"Combiners_Add_Mod" , 18},
        {"Combiners_Mod2x_Mod2x" , 19}
};

int getTabledShaderNames(uint16_t shaderId, uint16_t op_count, uint16_t tex_unit_number2,
    std::string &vertexShaderName, std::string &pixelShaderName
){
    uint16_t v4 = (shaderId >> 4) & 7;
    uint16_t v5 = shaderId & 7;
    uint16_t v6 = (shaderId >> 4) & 8;
    uint16_t v7 = shaderId & 8;

    if ( op_count == 1 ) {
        if ( v6 )
        {
            vertexShaderName = "Diffuse_Env";
        }
        else
        {
            vertexShaderName = "Diffuse_T2";
            if ( tex_unit_number2 )
                vertexShaderName = "Diffuse_T1";
        }
        switch ( v4 )
        {

            case 0:
                pixelShaderName = "Combiners_Opaque";
                break;
            case 2:
                pixelShaderName = "Combiners_Decal";
                break;
            case 3:
                pixelShaderName = "Combiners_Add";
                break;
            case 4:
                pixelShaderName = "Combiners_Mod2x";
                break;
            case 5:
                pixelShaderName = "Combiners_Fade";
                break;
            default:
                pixelShaderName = "Combiners_Mod";
                break;
        }
    } else {
        if ( v6 )
        {
            vertexShaderName = "Diffuse_Env_T2";
            if ( v7 )
                vertexShaderName = "Diffuse_Env_Env";
        }
        else if ( shaderId & 8 )
        {
            vertexShaderName = "Diffuse_T1_Env";
        }
        else
        {
            vertexShaderName = "Diffuse_T1_T2";
        }
        if ( !v4 )
        {
            switch ( v5 )
            {
                case 0:
                    pixelShaderName = "Combiners_Opaque_Opaque";
                    break;
                case 3:
                    pixelShaderName = "Combiners_Opaque_Add";
                    break;
                case 4:
                    pixelShaderName = "Combiners_Opaque_Mod2x";
                    break;
                case 6:
                    pixelShaderName = "Combiners_Opaque_Mod2xNA";
                    break;
                case 7:
                    pixelShaderName = "Combiners_Opaque_AddNA";
                    break;
                default:
                    pixelShaderName = "Combiners_Opaque_Mod";
                    break;
            }
        } else if ( v4 == 1 ) {
            switch ( v5 )
            {
                case 0:
                    pixelShaderName = "Combiners_Mod_Opaque";
                    break;
                case 3:
                    pixelShaderName = "Combiners_Mod_Add";
                    break;
                case 4:
                    pixelShaderName = "Combiners_Mod_Mod2x";
                    break;
                case 6:
                    pixelShaderName = "Combiners_Mod_Mod2xNA";
                    break;
                case 7:
                    pixelShaderName = "Combiners_Mod_AddNA";
                    break;
                default:
                    pixelShaderName = "Combiners_Mod_Mod";
                    break;

            }
        } else if ( v4 == 3 ) {
            if ( v5 == 1 )
            {
                pixelShaderName = "Combiners_Add_Mod";
            }
            return 0;
        } else if ( v4 != 4 ) {
            return 0;
        } else if ( v5 == 1 ) {
            pixelShaderName = "Combiners_Mod_Mod2x";
        } else {
            if ( v5 != 4 )
                return 0;
            pixelShaderName = "Combiners_Mod2x_Mod2x";
        }
    }

    return 1;
}

int getShaderNames(M2Batch *m2Batch, std::string &vertexShader, std::string &pixelShader){
        uint16_t shaderId = m2Batch->shader_id;

        if ( !(shaderId & 0x8000) ) {
            int result = getTabledShaderNames(shaderId, m2Batch->textureCount, m2Batch->textureCoordComboIndex, vertexShader, pixelShader);
//            if ( !result )
//                getTabledShaderNames(shaderId, m2Batch->textureCount, 0x11, m2Batch->textureCoordComboIndex, vertexShader, pixelShader);
            return 1;
        }
        switch ( shaderId & 0x7FFF ) {
            case 0:
                return 0;
            case 1:
                vertexShader = "Combiners_Opaque_Mod2xNA_Alpha";
            pixelShader = "Diffuse_T1_Env";
            break;
            case 2:
                vertexShader = "Combiners_Opaque_AddAlpha";
            pixelShader = "Diffuse_T1_Env";
            break;
            case 3:
                vertexShader = "Combiners_Opaque_AddAlpha_Alpha";
            pixelShader = "Diffuse_T1_Env";
            break;
            default:
                break;
        }

    return 1;
}

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

void M2Object::createPlacementMatrix (mathfu::vec3 pos, float f, mathfu::vec3 scaleVec, mathfu::mat4 *rotationMatrix){
    mathfu::mat4 placementMatrix = mathfu::mat4::FromTranslationVector(pos);

    if (rotationMatrix != nullptr) {
        placementMatrix *= *rotationMatrix;
    } else {
        placementMatrix *= MathHelper::RotationZ(toRadian(f));
    }

    placementMatrix *= mathfu::mat4::FromScaleVector(scaleVec);

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

mathfu::vec4 M2Object::getCombinedColor(
        M2SkinProfile *skinData,
        M2MaterialInst &materialData,
        std::vector<mathfu::vec4> subMeshColors
) {
    int colorIndex = skinData->batches[materialData.texUnit1TexIndex]->colorIndex;
    mathfu::vec4 submeshColor = mathfu::vec4(1,1,1,1);

    if ((colorIndex >= 0) && (subMeshColors.size() > colorIndex)) {
        mathfu::vec4 &color = subMeshColors[colorIndex];
        submeshColor = color;
    }

    return submeshColor;
}

float M2Object::getTransparency(
        M2SkinProfile *skinData,
        M2MaterialInst &materialData,
        std::vector<float> transparencies) {
    float transparency = 1.0;

    int transpIndex = skinData->batches[materialData.texUnit1TexIndex]->textureWeightComboIndex;
    if ((transpIndex >= 0) && (transparencies.size() > transpIndex)) {
        transparency = transparencies[transpIndex];
    }

    return transparency;
}

void M2Object::setDiffuseColor(CImVector& value) {
    this->m_localDiffuseColor = value;

    this->m_localDiffuseColorV = mathfu::vec4(
            value.b / 255.0f,
            value.g / 255.0f,
            value.r / 255.0f,
            value.a / 255.0f);
}
void M2Object::setLoadParams (std::string modelName, int skinNum, std::vector<uint8_t> meshIds, std::vector<std::string> replaceTextures) {
    modelName;
    this->m_skinNum = skinNum;
    this->m_meshIds = meshIds;
    this->m_replaceTextures = replaceTextures;

    std::string delimiter = ".";
    std::string nameTemplate = modelName.substr(0, modelName.find_last_of(delimiter));
    std::string modelFileName = nameTemplate + ".m2";
    std::string skinFileName = nameTemplate + "00.skin";

    this->m_modelName = modelFileName;
    this->m_skinName = skinFileName;

    this->m_modelIdent = modelFileName + " " +skinFileName;
    std::transform(m_modelIdent.begin(), m_modelIdent.end(), m_modelIdent.begin(), ::tolower);
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

bool matSortFunc(M2MaterialInst& a, M2MaterialInst& b) {
    if (a.priorityPlane != b.priorityPlane) {
        return b.priorityPlane < a.priorityPlane;
    }

    if (a.flags != b.flags) {
        return b.flags < a.flags;
    }

    return b.layer < a.layer;
}

void M2Object::sortMaterials(mathfu::mat4 &lookAtMat4) {
    if (!m_loading) return;

    /* 3. Resort m2 meshes against distance to screen */
    M2SkinProfile* skinData = this->m_skinGeom->getSkinData();

    mathfu::mat4 modelViewMat = lookAtMat4 * this->m_placementMatrix;


    mathfu::vec3 zeroVect(0,0,0);

    /* 3.1 Transform aabb with current mat */

    std::sort(this->m_materialArray.begin(),
              this->m_materialArray.end(),
              matSortFunc
    );
}

void M2Object::update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat) {
    if (!this->m_loaded) {
        if ((m_m2Geom != nullptr) && m_m2Geom->isLoaded() && (m_skinGeom != nullptr) && m_skinGeom->isLoaded()) {
            this->m_loaded = true;
            this->m_loading = false;

            m_skinGeom->fixData(m_m2Geom->getM2Data());

            this->createAABB();
            this->makeTextureArray();
            this->initAnimationManager();
            this->initBoneAnimMatrices();
            this->initTextAnimMatrices();
            this->initSubmeshColors();
            this->initTransparencies();
            m_hasBillboards = checkIfHasBillboarded();


        } else {
            return;
        }
    };

//    /* 1. Calc local camera */
    mathfu::vec4 cameraInlocalPos = mathfu::vec4(cameraPos, 1);
    cameraInlocalPos = m_placementInvertMatrix * cameraInlocalPos;
//
//    /* 2. Update animation values */
    this->m_animationManager->update(deltaTime, cameraInlocalPos.xyz(),
                                 this->bonesMatrices,
        this->textAnimMatrices,
        this->subMeshColors,
        this->transparencies
        //this->cameras,
        //this->lights,
        //this->particleEmittersArray
    );

    this->sortMaterials(viewMat);
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

const bool M2Object::checkFrustumCulling (const mathfu::vec4 &cameraPos, const std::vector<mathfu::vec4> &frustumPlanes, const std::vector<mathfu::vec3> &frustumPoints) {
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

void M2Object::draw(bool drawTransparent) {
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

    static mathfu::vec4 diffuseNon(1.0, 1.0, 1.0, 1.0);
    mathfu::vec4 localDiffuse = diffuseNon;
    if (m_useLocalDiffuseColor) {
        localDiffuse = m_localDiffuseColorV;
    }

    this->m_m2Geom->setupUniforms(this->m_api, m_placementMatrix, bonesMatrices, localDiffuse, drawTransparent, false);

    this->drawMeshes(drawTransparent, -1);

//    if (vaoBinded) {
//        this.m2Geom.unbindVao()
//    }
}

void M2Object::drawBBInternal(CAaBox &bb, mathfu::vec3 &color, mathfu::mat4 &placementMatrix) {
    mathfu::vec3 center = mathfu::vec3(
        (bb.min.x + bb.max.x) / 2,
        (bb.min.y + bb.max.y) / 2,
        (bb.min.z + bb.max.z) / 2
    );

    mathfu::vec3 scale = mathfu::vec3(
        bb.max.x - center[0],
        bb.max.y - center[1],
        bb.max.z - center[2]
    );

    ShaderRuntimeData *shader = this->m_api->getBBShader();
    glUniform3fv(shader->getUnf("uBBScale"), 1, &scale[0]);
    glUniform3fv(shader->getUnf("uBBCenter"), 1, &center[0]);
    glUniform3fv(shader->getUnf("uColor"), 3, &color[0]); //red
    glUniformMatrix4fv(shader->getUnf("uPlacementMat"), 1, GL_FALSE, &placementMatrix[0]);

    glDrawElements(GL_LINES, 48, GL_UNSIGNED_SHORT, 0);
}

void M2Object::drawBB(mathfu::vec3 &color) {
    if (!this->m_loaded) return;

//    function drawBBInternal2(center, scale, color, placementMatrix) {
//        gl.uniform3fv(uniforms.uBBScale, new Float32Array(scale));
//        gl.uniform3fv(uniforms.uBBCenter, new Float32Array(center));
//        gl.uniform3fv(uniforms.uColor, new Float32Array(color)); //red
//        gl.uniformMatrix4fv(uniforms.uPlacementMat, false, placementMatrix);
//
//        gl.drawElements(gl.LINES, 48, gl.UNSIGNED_SHORT, 0);
//    }
    //CAaBox bb = this->getBoundingBox();

    mathfu::mat4 defMat = mathfu::mat4::Identity();
    drawBBInternal(this->aabb, color, defMat);

}

void M2Object::drawMeshes(bool drawTransparent, int instanceCount) {
    if (!drawTransparent) {
        for (int i = 0; i < this->m_materialArray.size(); i++) {
            auto &materialData = this->m_materialArray[i];
            this->drawMaterial(materialData, drawTransparent, instanceCount);
        }
    } else {
        for (int i = (int) (this->m_materialArray.size() - 1); i >= 0; i--) {
            auto &materialData = this->m_materialArray[i];
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
    mathfu::vec4 meshColor = this->getCombinedColor(skinData, materialData, this->subMeshColors);
    float transparency = this->getTransparency(skinData, materialData, this->transparencies);
//    mathfu::vec4 meshColor = mathfu::vec4(1,1,1,1);
//    float transparency = 1;
//
//    //Don't draw meshes with 0 transp
    if ((transparency < 0.0001) || (meshColor[3] < 0.0001)) return;
//
//
    int pixelShaderIndex = materialData.pixelShader;
    this->m_m2Geom->drawMesh(m_api, materialData, *skinData , meshColor, transparency, textureMatrix1, textureMatrix2, pixelShaderIndex, originalFogColor, instanceCount);
}

void M2Object::makeTextureArray() {
    /* 1. Free previous subMeshArray */
    auto textureCache = m_api->getTextureCache();

    /* 2. Fill the materialArray */
    auto & subMeshes = m_skinGeom->getSkinData()->submeshes;
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

        materialData.layer = skinTextureDefinition->materialLayer;
        materialData.isRendered = true;
         materialData.isTransparent = isTransparent;
        materialData.meshIndex = skinTextureDefinition->skinSectionIndex;
        materialData.renderFlagIndex = skinTextureDefinition->materialIndex;
        materialData.flags = skinTextureDefinition->flags;
        materialData.priorityPlane = skinTextureDefinition->priorityPlane;

        std::string vertexShader;
        std::string pixelShader;
        getShaderNames(skinTextureDefinition, vertexShader, pixelShader);
        materialData.pixelShader = pixelShaderTable.at(pixelShader);
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

bool M2Object::checkIfHasBillboarded() {
    M2Data * m2File = this->m_m2Geom->getM2Data();
    for (int i = 0; i < m2File->bones.size; i++) {
        M2CompBone * boneDefinition = m2File->bones.getElement(i);
        if ((
            boneDefinition->flags.cylindrical_billboard_lock_x &
            boneDefinition->flags.cylindrical_billboard_lock_y &
            boneDefinition->flags.cylindrical_billboard_lock_z &
            boneDefinition->flags.spherical_billboard) > 0) {
            return true;
        }
    }

    return false;
}

void M2Object::initBoneAnimMatrices() {
    this->bonesMatrices = std::vector<mathfu::mat4>(m_m2Geom->getM2Data()->bones.size, mathfu::mat4::Identity());;
}
void M2Object::initTextAnimMatrices() {
    textAnimMatrices = std::vector<mathfu::mat4>(m_m2Geom->getM2Data()->texture_transforms.size, mathfu::mat4::Identity());;
}

void M2Object::initSubmeshColors() {
    subMeshColors = std::vector<mathfu::vec4>(m_m2Geom->getM2Data()->colors.size);

}
void M2Object::initTransparencies() {
    transparencies = std::vector<float>(m_m2Geom->getM2Data()->transparency_lookup_table.size);
}

void M2Object::drawInstanced(bool drawTransparent, int instanceCount, GLuint placementVBO) {
    if (!this->m_loaded) return;

    this->m_m2Geom->setupAttributes(/*this->m_skinGeom*/);
    this->m_skinGeom->setupAttributes();
    this->m_m2Geom->setupUniforms(m_api, m_placementMatrix, bonesMatrices, m_localDiffuseColorV, drawTransparent, true);
    this->m_m2Geom->setupPlacementAttribute(placementVBO);
    this->drawMeshes(drawTransparent, instanceCount);
}
