//
// Created by deamon on 22.06.17.
//

#include <locale>
#include <iomanip>
#include "m2Object.h"
#include "../../algorithms/mathHelper.h"
#include "../../managers/animationManager.h"
#include "mathfu/matrix.h"
#include "../../persistance/header/M2FileHeader.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "m2Helpers/M2MeshBufferUpdater.h"

#include "../../../gapi/interface/IOcclusionQuery.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"

//Shader stuff
enum class M2PixelShader : int {
    //Wotlk deprecated shaders
    Combiners_Decal = -1,
    Combiners_Add = -2,
    Combiners_Mod2x = -3,
    Combiners_Fade = -4,
    Combiners_Opaque_Add = -5,
    Combiners_Opaque_AddNA = -6,
    Combiners_Add_Mod = -7,
    Combiners_Mod2x_Mod2x = -8,

    //Legion modern shaders
    Combiners_Opaque = 0,
    Combiners_Mod = 1,
    Combiners_Opaque_Mod = 2,
    Combiners_Opaque_Mod2x = 3,
    Combiners_Opaque_Mod2xNA = 4,
    Combiners_Opaque_Opaque = 5,
    Combiners_Mod_Mod = 6,
    Combiners_Mod_Mod2x = 7,
    Combiners_Mod_Add = 8,
    Combiners_Mod_Mod2xNA = 9,
    Combiners_Mod_AddNA = 10,
    Combiners_Mod_Opaque = 11,
    Combiners_Opaque_Mod2xNA_Alpha = 12,
    Combiners_Opaque_AddAlpha = 13,
    Combiners_Opaque_AddAlpha_Alpha = 14,
    Combiners_Opaque_Mod2xNA_Alpha_Add = 15,
    Combiners_Mod_AddAlpha = 16,
    Combiners_Mod_AddAlpha_Alpha = 17,
    Combiners_Opaque_Alpha_Alpha = 18,
    Combiners_Opaque_Mod2xNA_Alpha_3s = 19,
    Combiners_Opaque_AddAlpha_Wgt = 20,
    Combiners_Mod_Add_Alpha = 21,
    Combiners_Opaque_ModNA_Alpha = 22,
    Combiners_Mod_AddAlpha_Wgt = 23,
    Combiners_Opaque_Mod_Add_Wgt = 24,
    Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha = 25,
    Combiners_Mod_Dual_Crossfade = 26,
    Combiners_Opaque_Mod2xNA_Alpha_Alpha = 27,
    Combiners_Mod_Masked_Dual_Crossfade = 28,
    Combiners_Opaque_Alpha = 29,
    Guild = 30,
    Guild_NoBorder = 31,
    Guild_Opaque = 32,
    Combiners_Mod_Depth = 33,
    Illum = 34,
    Combiners_Mod_Mod_Mod_Const = 35,
    NewUnkCombiner = 36
};

enum class M2VertexShader : int {
    Diffuse_T1 = 0,
    Diffuse_Env = 1,
    Diffuse_T1_T2 = 2,
    Diffuse_T1_Env = 3,
    Diffuse_Env_T1 = 4,
    Diffuse_Env_Env = 5,
    Diffuse_T1_Env_T1 = 6,
    Diffuse_T1_T1 = 7,
    Diffuse_T1_T1_T1 = 8,
    Diffuse_EdgeFade_T1 = 9,
    Diffuse_T2 = 10,
    Diffuse_T1_Env_T2 = 11,
    Diffuse_EdgeFade_T1_T2 = 12,
    Diffuse_EdgeFade_Env = 13,
    Diffuse_T1_T2_T1 = 14,
    Diffuse_T1_T2_T3 = 15,
    Color_T1_T2_T3 = 16,
    BW_Diffuse_T1 = 17,
    BW_Diffuse_T1_T2 = 18,
};

inline constexpr const int operator+ (M2PixelShader const val) { return static_cast<const int>(val); };
inline constexpr const int operator+ (M2VertexShader const val) { return static_cast<const int>(val); };

EGxBlendEnum M2BlendingModeToEGxBlendEnum [8] =
    {
        EGxBlendEnum::GxBlend_Opaque,
        EGxBlendEnum::GxBlend_AlphaKey,
        EGxBlendEnum::GxBlend_Alpha,
        EGxBlendEnum::GxBlend_NoAlphaAdd,
        EGxBlendEnum::GxBlend_Add,
        EGxBlendEnum::GxBlend_Mod,
        EGxBlendEnum::GxBlend_Mod2x,
        EGxBlendEnum::GxBlend_BlendAdd
    };

struct M2Shaders{
    unsigned int pixel;
    unsigned int vertex;
    unsigned int hull;
    unsigned int domain;};
static std::array<M2Shaders, 36> M2ShaderTable = {{
        { +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha,              +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_AddAlpha,                   +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_AddAlpha_Alpha,             +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha_Add,          +M2VertexShader::Diffuse_T1_Env_T1, 2, 2 },
        { +M2PixelShader::Combiners_Mod_AddAlpha,                      +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_AddAlpha,                   +M2VertexShader::Diffuse_T1_T1, 1, 1 },
        { +M2PixelShader::Combiners_Mod_AddAlpha,                      +M2VertexShader::Diffuse_T1_T1, 1, 1 },
        { +M2PixelShader::Combiners_Mod_AddAlpha_Alpha,                +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Alpha_Alpha,                +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha_3s,           +M2VertexShader::Diffuse_T1_Env_T1, 2, 2 },
        { +M2PixelShader::Combiners_Opaque_AddAlpha_Wgt,               +M2VertexShader::Diffuse_T1_T1, 1, 1 },
        { +M2PixelShader::Combiners_Mod_Add_Alpha,                     +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_ModNA_Alpha,                +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Mod_AddAlpha_Wgt,                  +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Mod_AddAlpha_Wgt,                  +M2VertexShader::Diffuse_T1_T1, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_AddAlpha_Wgt,               +M2VertexShader::Diffuse_T1_T2, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Mod_Add_Wgt,                +M2VertexShader::Diffuse_T1_Env, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha,    +M2VertexShader::Diffuse_T1_Env_T1, 2, 2 },
        { +M2PixelShader::Combiners_Mod_Dual_Crossfade,                +M2VertexShader::Diffuse_T1, 0, 0 },
        { +M2PixelShader::Combiners_Mod_Depth,                         +M2VertexShader::Diffuse_EdgeFade_T1, 0, 0 },
        { +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha_Alpha,        +M2VertexShader::Diffuse_T1_Env_T2, 2, 2 },
        { +M2PixelShader::Combiners_Mod_Mod,                           +M2VertexShader::Diffuse_EdgeFade_T1_T2, 1, 1 },
        { +M2PixelShader::Combiners_Mod_Masked_Dual_Crossfade,         +M2VertexShader::Diffuse_T1_T2, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Alpha,                      +M2VertexShader::Diffuse_T1_T1, 1, 1 },
        { +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha_UnshAlpha,    +M2VertexShader::Diffuse_T1_Env_T2, 2, 2 },
        { +M2PixelShader::Combiners_Mod_Depth,                         +M2VertexShader::Diffuse_EdgeFade_Env, 0, 0 },
        { +M2PixelShader::Guild,                                       +M2VertexShader::Diffuse_T1_T2_T1, 2, 1 },
        { +M2PixelShader::Guild_NoBorder,                              +M2VertexShader::Diffuse_T1_T2, 1, 2 },
        { +M2PixelShader::Guild_Opaque,                                +M2VertexShader::Diffuse_T1_T2_T1, 2, 1 },
        { +M2PixelShader::Illum,                                       +M2VertexShader::Diffuse_T1_T1, 1, 1 },
        { +M2PixelShader::Combiners_Mod_Mod_Mod_Const,                 +M2VertexShader::Diffuse_T1_T2_T3, 2, 2 },
        { +M2PixelShader::Combiners_Mod_Mod_Mod_Const,                 +M2VertexShader::Color_T1_T2_T3, 2, 2 },
        { +M2PixelShader::Combiners_Opaque,                            +M2VertexShader::Diffuse_T1, 0, 0 },
        { +M2PixelShader::Combiners_Mod_Mod2x,                         +M2VertexShader::Diffuse_EdgeFade_T1_T2, 1, 1 },
        { +M2PixelShader::Combiners_Mod,                               +M2VertexShader::Diffuse_EdgeFade_T1, 1, 1 },
        { +M2PixelShader::NewUnkCombiner,                              +M2VertexShader::Diffuse_EdgeFade_T1_T2, 1, 1 },
}};

int getVertexShaderId(int textureCount, int16_t shaderId) {
    int result;
    if ( shaderId < 0 )
    {
        int vertexShaderId = shaderId & 0x7FFF;
        if ( (unsigned int)vertexShaderId >= M2ShaderTable.size()) {
            std::cout << "Wrong shaderId for vertex shader";
            assert(false);
        }
        result = (unsigned int)M2ShaderTable[(shaderId & 0x7FFF)].vertex;
    }
    else if ( textureCount == 1 )
    {
        if ( (shaderId & 0x80u) != 0 )
        {
            result = 1LL;
        }
        else
        {
            result = 10LL;
            if ( !(shaderId & 0x4000) )
                result = 0LL;
        }
    }
    else if ( (shaderId & 0x80u) != 0 )
    {
        result = ((shaderId & 8u) >> 3) | 4;
    }
    else
    {
        result = 3LL;
        if ( !(shaderId & 8) )
            result = 5 * (unsigned int)((shaderId & 0x4000) == 0) + 2;
    }
    return result;
}

int getPixelShaderId(int textureCount, uint16_t shaderId) {
    static const std::array<uint32_t, 8> array1 = {
            +M2PixelShader::Combiners_Mod_Mod2x,
            +M2PixelShader::Combiners_Mod_Mod,
            +M2PixelShader::Combiners_Mod_Mod2xNA,
            +M2PixelShader::Combiners_Mod_AddNA,
            +M2PixelShader::Combiners_Mod_Opaque,
            +M2PixelShader::Combiners_Mod_Mod,
            +M2PixelShader::Combiners_Mod_Mod,
            +M2PixelShader::Combiners_Mod_Add
    };
    static const std::array<uint32_t, 8> array2 = {
            +M2PixelShader::Combiners_Opaque_Mod2x,
            +M2PixelShader::Combiners_Opaque_Mod,
            +M2PixelShader::Combiners_Opaque_Mod2xNA,
            +M2PixelShader::Combiners_Opaque_AddAlpha_Alpha,
            +M2PixelShader::Combiners_Opaque_Opaque,
            +M2PixelShader::Combiners_Opaque_Mod,
            +M2PixelShader::Combiners_Opaque_Mod,
            +M2PixelShader::Combiners_Opaque_AddAlpha_Alpha
    };

    int result;
    if ( (shaderId & 0x8000) > 0 )
    {
        int pixelShaderId = shaderId & 0x7FFF;
        if ( (unsigned int)pixelShaderId >= M2ShaderTable.size()) {
            std::cout << "Wrong shaderId for pixel shader";
            assert(false);
        }
        result = (unsigned int)M2ShaderTable[shaderId & 0x7FFF].pixel;
    }
    else if ( textureCount == 1 )
    {
        result = (shaderId & 0x70) != 0;
    }
    else
    {

        //For future reference. The arrays are these cases, with inbetween filled with default value
//        result = array2[(shaderId) & 7];
//        if ( shaderId & 0x70 ) {
//            result = array1[(shaderId) & 7];
//        }

        if ( shaderId & 0x70 ) {
            switch (shaderId & 7) {
                case 3 :
                    result = +M2PixelShader::Combiners_Mod_Add;
                    break;
                case 4 :
                    result = +M2PixelShader::Combiners_Mod_Mod2x;
                    break;

                case 6 :
                    result = +M2PixelShader::Combiners_Mod_Mod2xNA;
                    break;
                case 7 :
                    result = +M2PixelShader::Combiners_Mod_AddNA;
                    break;

                default:
                    result = +M2PixelShader::Combiners_Mod_Mod;
                    break;
            }
        } else {
            switch (shaderId & 7) {
                case 0 :
                    result = +M2PixelShader::Combiners_Opaque_Opaque;
                    break;
                case 3:
                case 7:
                    result = +M2PixelShader::Combiners_Opaque_AddAlpha;
                    break;
                case 4:
                    result = +M2PixelShader::Combiners_Opaque_Mod2x;
                    break;
                case 6:
                    result = +M2PixelShader::Combiners_Opaque_Mod2xNA;
                    break;
                default:
                    result = +M2PixelShader::Combiners_Opaque_Mod;
                    break;
            }
        }
    }
    return result;
}

std::unordered_map<std::string, int> pixelShaderTable = {
        {"Combiners_Opaque",                    +M2PixelShader::Combiners_Opaque},
        {"Combiners_Decal" ,                    +M2PixelShader::Combiners_Decal},
        {"Combiners_Add" ,                      +M2PixelShader::Combiners_Add},
        {"Combiners_Mod2x" ,                    +M2PixelShader::Combiners_Mod2x},
        {"Combiners_Fade" ,                     +M2PixelShader::Combiners_Fade},
        {"Combiners_Mod" ,                      +M2PixelShader::Combiners_Mod},
        {"Combiners_Opaque_Opaque" ,            +M2PixelShader::Combiners_Opaque_Opaque},
        {"Combiners_Opaque_Add" ,               +M2PixelShader::Combiners_Opaque_Add},
        {"Combiners_Opaque_Mod2x" ,             +M2PixelShader::Combiners_Opaque_Mod2x},
        {"Combiners_Opaque_Mod2xNA" ,           +M2PixelShader::Combiners_Opaque_Mod2xNA},
        {"Combiners_Opaque_AddNA" ,             +M2PixelShader::Combiners_Opaque_AddNA},
        {"Combiners_Opaque_Mod" ,               +M2PixelShader::Combiners_Opaque_Mod},
        {"Combiners_Mod_Opaque" ,               +M2PixelShader::Combiners_Mod_Opaque},
        {"Combiners_Mod_Add" ,                  +M2PixelShader::Combiners_Mod_Add},
        {"Combiners_Mod_Mod2x" ,                +M2PixelShader::Combiners_Mod_Mod2x},
        {"Combiners_Mod_Mod2xNA" ,              +M2PixelShader::Combiners_Mod_Mod2xNA},
        {"Combiners_Mod_AddNA" ,                +M2PixelShader::Combiners_Mod_AddNA},
        {"Combiners_Mod_Mod" ,                  +M2PixelShader::Combiners_Mod_Mod},
        {"Combiners_Add_Mod" ,                  +M2PixelShader::Combiners_Add_Mod},
        {"Combiners_Mod2x_Mod2x" ,              +M2PixelShader::Combiners_Mod2x_Mod2x},
        {"Combiners_Opaque_Mod2xNA_Alpha" ,     +M2PixelShader::Combiners_Opaque_Mod2xNA_Alpha},
        {"Combiners_Opaque_AddAlpha" ,          +M2PixelShader::Combiners_Opaque_AddAlpha},
        {"Combiners_Opaque_AddAlpha_Alpha" ,    +M2PixelShader::Combiners_Opaque_AddAlpha_Alpha},
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

int getShaderNames(M2Batch *m2Batch, std::string &vertexShader, std::string &pixelShader) {
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
                pixelShader = "Combiners_Opaque_Mod2xNA_Alpha";
                vertexShader = "Diffuse_T1_Env";
            break;
            case 2:
                pixelShader = "Combiners_Opaque_AddAlpha";
                vertexShader= "Diffuse_T1_Env";
            break;
            case 3:
                pixelShader  = "Combiners_Opaque_AddAlpha_Alpha";
                vertexShader = "Diffuse_T1_Env";
            break;
            default:
                break;
        }

    return 1;
}

M2Object::~M2Object() {
    delete m_animationManager;

    for (auto obj: particleEmitters) {
        delete obj;
    }
    for (auto obj: ribbonEmitters) {
        delete obj;
    }
}


void M2Object::createAABB() {
    M2Data *m2Data = m_m2Geom->getM2Data();

    //Debug: calc bounding box from verticies
    {
        auto min = mathfu::vec3(9999, 9999, 9999);
        auto max = mathfu::vec3(-9999, -9999, -9999);

        for (int i = 0; i < m2Data->vertices.size; i++) {
            auto *vertex = m2Data->vertices.getElement(i);

            min = mathfu::vec3(mathfu::vec3(std::min(min.x, vertex->pos.x),
                                                             std::min(min.y, vertex->pos.y),
                                                             std::min(min.z, vertex->pos.z)));

            max = mathfu::vec3(mathfu::vec3(std::max(max.x, vertex->pos.x),
                                                             std::max(max.y, vertex->pos.y),
                                                             std::max(max.z, vertex->pos.z)));
        }

//        std::cout << "calculated min = (" << min.x << ", " << min.y << ", " << min.z << ")" << std::endl;
//        std::cout << "calculated max = (" << max.x << ", " << max.y << ", " << max.z << ")" << std::endl;
    }



    {
        C3Vector min = m2Data->bounding_box.min;
        C3Vector max = m2Data->bounding_box.max;
        mathfu::vec4 minVec = mathfu::vec4(min.x, min.y, min.z, 1);
        mathfu::vec4 maxVec = mathfu::vec4(max.x, max.y, max.z, 1);

        CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, minVec, maxVec);

        //this.diameter = vec3.distance(worldAABB[0],worldAABB[1]);
        this->aabb = worldAABB;
    }

    {
        C3Vector min = m2Data->collision_box.min;
        C3Vector max = m2Data->collision_box.max;
        mathfu::vec4 minVec = mathfu::vec4(min.x, min.y, min.z, 1);
        mathfu::vec4 maxVec = mathfu::vec4(max.x, max.y, max.z, 1);

        CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, minVec, maxVec);

        //this.diameter = vec3.distance(worldAABB[0],worldAABB[1]);
        this->colissionAabb = worldAABB;
    }
    m_hasAABB = true;
}


CAaBox M2Object::getColissionAABB() {
    CAaBox result;
    if (m_m2Geom->m_m2Data->sequences.size > 0) {
        int animationIndex = 0;
        if (m_m2Geom->m_m2Data->sequence_lookups.size > 0) {
            int index = *m_m2Geom->m_m2Data->sequence_lookups[0];
            if (index > 0 && m_m2Geom->m_m2Data->sequences[index] == 0)
                animationIndex = index;
        }
        result = m_m2Geom->m_m2Data->sequences[animationIndex]->bounds.extent;
        C3Vector min = result.min;
        C3Vector max = result.max;
        mathfu::vec4 minVec = mathfu::vec4(min.x, min.y, min.z, 1);
        mathfu::vec4 maxVec = mathfu::vec4(max.x, max.y, max.z, 1);

        result = MathHelper::transformAABBWithMat4(m_placementMatrix, minVec, maxVec);
    } else {
        result = colissionAabb;
    }

    return result;
}

void M2Object:: createPlacementMatrix(SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat) {
    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix = placementMatrix * wmoPlacementMat;
    placementMatrix = placementMatrix * mathfu::mat4::FromTranslationVector(mathfu::vec3(def.position));

    mathfu::quat quat4(def.orientation.w, def.orientation.x, def.orientation.y, def.orientation.z);
    placementMatrix = placementMatrix * quat4.ToMatrix4();

    float scale = def.scale;
    placementMatrix = placementMatrix * mathfu::mat4::FromScaleVector(mathfu::vec3(scale,scale,scale));

    mathfu::mat4 invertPlacementMatrix = placementMatrix.Inverse();

    m_localPosition = mathfu::vec3(def.position);
    m_placementMatrix = placementMatrix;
    m_placementInvertMatrix = invertPlacementMatrix;

    m_localUpVector = (invertPlacementMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();

    hasModf0x2Flag = def.flag_0x2;
}

void M2Object::createPlacementMatrix(SMDoodadDef &def) {
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

    float scale = def.scale / 1024.0f;
    placementMatrix *= mathfu::mat4::FromScaleVector(mathfu::vec3(scale, scale, scale));

    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_localPosition = mathfu::vec3(def.position);
    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;

    m_localUpVector = (placementInvertMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();
    m_localRightVector = (placementInvertMatrix * mathfu::vec4(1,0,0,0)).xyz().Normalized();
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

    m_localUpVector = (placementInvertMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();
    m_localRightVector = (placementInvertMatrix * mathfu::vec4(1,0,0,0)).xyz().Normalized();
}

void M2Object::updatePlacementMatrixFromParentAttachment(M2Object *parent, int attachment, float scale) {
    if (!parent->m_loaded) return;
    auto &m2Geom = parent->m_m2Geom;
    if (m2Geom->m_m2Data->attachment_lookup_table.size == 0) return;
    if (m2Geom->m_m2Data->attachments.size == 0) return;

    int attIndex = *m2Geom->m_m2Data->attachment_lookup_table[2];
    M2Attachment *attachInfo = m2Geom->m_m2Data->attachments[attIndex];

    if (attachInfo == nullptr) return;

    int boneId = attachInfo->bone;
    mathfu::mat4 &parentBoneTransMat = parent->bonesMatrices[boneId];


    mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
    placementMatrix = parent->m_placementMatrix *
        parentBoneTransMat *
        mathfu::mat4::FromTranslationVector(mathfu::vec3(attachInfo->position));

    mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

    m_placementInvertMatrix = placementInvertMatrix;
    m_placementMatrix = placementMatrix;

    m_localUpVector = (placementInvertMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();
    m_localRightVector = (placementInvertMatrix * mathfu::vec4(1,0,0,0)).xyz().Normalized();
}


void M2Object::calcDistance(mathfu::vec3 cameraPos) {
    m_currentDistance = (m_worldPosition-cameraPos).Length();
}

float M2Object::getCurrentDistance() {
    return m_currentDistance;
}
float M2Object::getHeight(){
    return this->aabb.max.z -this->aabb.min.z;
}

mathfu::vec4 M2Object::getCombinedColor(
        M2SkinProfile *skinData,
        int batchIndex,
        const std::vector<mathfu::vec4> &subMeshColors
) {
    int colorIndex = skinData->batches[batchIndex]->colorIndex;
    mathfu::vec4 submeshColor = mathfu::vec4(1,1,1,1);

    if ((colorIndex >= 0) && (subMeshColors.size() > colorIndex)) {
        const mathfu::vec4 &color = subMeshColors[colorIndex];
        submeshColor = color;
    }

    return submeshColor;
}

float M2Object::getTextureWeight(
        M2SkinProfile *skinData,
        M2Data * m2Data,
        int batchIndex,
        int textureIndex,
        const std::vector<float> &transparencies) {
    float transparency = 1.0;

    int transpLookupIndex = skinData->batches[batchIndex]->textureWeightComboIndex + textureIndex;
    int transpIndex = -1;
    if ((transpLookupIndex >= 0) && (transpLookupIndex < m2Data->transparency_lookup_table.size)) {
        transpIndex = *m2Data->transparency_lookup_table[transpLookupIndex];
    }

    if ((transpIndex >= 0) && (transparencies.size() > transpIndex)) {
        transparency = transparencies[transpIndex];
    }

    return transparency;
}
typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

hsv rgb2hsv(mathfu::vec3 in)
{
    hsv         out;
    double      min, max, delta;

    min = in.x < in.y ? in.x : in.y;
    min = min  < in.z ? min  : in.z;

    max = in.x > in.y ? in.x : in.y;
    max = max  > in.z ? max  : in.z;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.x >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.y - in.z ) / delta;        // between yellow & magenta
    else
    if( in.y >= max )
        out.h = 2.0 + ( in.z - in.x ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.x - in.y ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


mathfu::vec3 hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    mathfu::vec3    out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.x = in.v;
        out.y = in.v;
        out.z = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
        case 0:
            out.x = in.v;
            out.y = t;
            out.z = p;
            break;
        case 1:
            out.x = q;
            out.y = in.v;
            out.z = p;
            break;
        case 2:
            out.x = p;
            out.y = in.v;
            out.z = t;
            break;

        case 3:
            out.x = p;
            out.y = q;
            out.z = in.v;
            break;
        case 4:
            out.x = t;
            out.y = p;
            out.z = in.v;
            break;
        case 5:
        default:
            out.x = in.v;
            out.y = p;
            out.z = q;
            break;
    }
    return out;
}

uint8_t miniLogic(const CImVector *a2) {

    uint8_t v7 = a2->r;
    uint8_t v8 = a2->g;
    if ( v8 <= a2->b )
    {
        v8 = a2->b;
    }
    if ( v8 <= v7 )
    {
        v8 = v7;
    }
    uint8_t v10 = 1;
    if ( v8 )
    {
        v10 = v8;
    }

    return v10;
}

void M2Object::setDiffuseColor(CImVector& value) {
    this->m_localDiffuseColor = value;

    this->m_localDiffuseColorV = mathfu::vec4(
            value.r / 255.0f,
            value.g / 255.0f,
            value.b / 255.0f,
            value.a / 255.0f);

    if (value.a != 255) {
//        std::cout << "Found index into MOLT = " << (int)value.a << std::endl;
    }

    /*
    uint8_t result = miniLogic(&value);
    if (result < 0xA8) {
        hsv hsv1 = rgb2hsv(m_localDiffuseColorV.xyz());
        hsv1.v = (((float)0xA8) / ((float)result)) * hsv1.v;
        this->m_localDiffuseColorV = mathfu::vec4(hsv2rgb(hsv1), 0);
    }

    int a6 = 0x60;
    m_ambientAddColor = m_localDiffuseColorV;
    if ( result > a6 )
    {
        float v14 = (float)((float)a6 * 255.0) / (float)result;
        uint8_t green = (uint8_t)m_localDiffuseColor.g;
        uint32_t newImColor = ((uint16_t)(((uint16_t)green * (signed int)v14 + 255) & 0xFF00) | ((uint8_t)((uint16_t)((signed int)v14 * (uint8_t)m_localDiffuseColor.r + 255) >> 8) << 16) | *(uint16_t *)&m_localDiffuseColor & 0xFF000000 | ((uint16_t)((signed int)v14 * (uint8_t)m_localDiffuseColor.b + 255) >> 8));
        CImVector newValue = *(CImVector *)&newImColor;

        this->m_ambientAddColor = mathfu::vec4(
            newValue.r / 255.0f,
            newValue.g / 255.0f,
            newValue.b / 255.0f,
            newValue.a / 255.0f);
    }
     */
}
void M2Object::setLoadParams (int skinNum, std::vector<uint8_t> meshIds, std::vector<HBlpTexture> replaceTextures) {
    this->m_skinNum = skinNum;
    this->m_meshIds = meshIds;
    this->m_replaceTextures = replaceTextures;
}

void M2Object::startLoading() {
    if (!m_loading) {
        m_loading = true;

        Cache<M2Geom> *m2GeomCache = m_api->cacheStorage->getM2GeomCache();
        if (!useFileId) {
            m_m2Geom = m2GeomCache->get(m_modelName);
        } else {
            m_m2Geom = m2GeomCache->getFileId(m_modelFileId);
        }
    }
}

void M2Object::sortMaterials(mathfu::mat4 &modelViewMat) {
    if (!m_loaded) return;

    M2Data * m2File = this->m_m2Geom->getM2Data();
    M2SkinProfile * skinData = this->m_skinGeom->getSkinData();

    if (m_m2Geom->m_wfv3 == nullptr && m_m2Geom->m_wfv1 == nullptr) {
        for (int i = 0; i < this->m_meshNaturalArray.size(); i++) {
            //Update info for sorting
            M2MeshBufferUpdater::updateSortData(this->m_meshNaturalArray[i], *this, m_materialArray[i], m2File,
                                                skinData, modelViewMat);
        }
        for (int i = 0; i < this->m_meshForcedTranspArray.size(); i++) {
            //Update info for sorting
            if (this->m_meshForcedTranspArray[i] != nullptr) {
                M2MeshBufferUpdater::updateSortData(this->m_meshForcedTranspArray[i], *this, m_materialArray[i], m2File,
                                                    skinData, modelViewMat);
            }
        }
    }
}

void M2Object::debugDumpAnimationSequences() {

    std::cout << "Model name = " << m_modelName << std::endl;
    int sequence_lookupsSize = m_m2Geom->m_m2Data->sequence_lookups.size;
    std::cout << "sequences.size = " << m_m2Geom->m_m2Data->sequences.size << std::endl;
    std::cout << "sequence_lookups.size = " << m_m2Geom->m_m2Data->sequence_lookups.size << std::endl;

    int misses = 0;

    for (int i = 0; i < m_m2Geom->m_m2Data->sequences.size; i++) {
        auto sequence = m_m2Geom->m_m2Data->sequences.getElement(i);
        int index = sequence->id % sequence_lookupsSize;
        int seqLookup = * m_m2Geom->m_m2Data->sequence_lookups.getElement(index);
        int trueSeqLookup = -1;
        for (int j = 0; j < m_m2Geom->m_m2Data->sequence_lookups.size; j++) {
            if (*m_m2Geom->m_m2Data->sequence_lookups.getElement(j) == i) {
                trueSeqLookup = j;
                break;
            }
        }

        std::cout << std::right
                  << "sequence[" << i << "].id = "<< sequence->id
                  << std::setw(5) << " (" << sequence->id << " % " << sequence_lookupsSize << ") = " << index
                  << std::setw(20) << " trueSeqLookup = " << trueSeqLookup
                  << std::setw(10) << " theoryOk = " << (bool)(seqLookup == i)
                  << std::endl;

        if (trueSeqLookup != -1 && (trueSeqLookup != index)) {
            misses++;
        }
    }
    std::cout << std::endl << std::endl;
    std::cout << "misses = " << misses << std::endl << std::endl;
    for (int i = 0; i < m_m2Geom->m_m2Data->sequence_lookups.size; i++) {
        auto sequenceLookup = *m_m2Geom->m_m2Data->sequence_lookups.getElement(i);
        std::cout << "sequence_lookups[" << i << "].id = "<< sequenceLookup << std::endl;
    }

}

//Returns true when it finished loading
bool M2Object::doPostLoad(){
    //0. If loading procedures were already done - exit
    if (this->m_loaded) return false;

    //1. Check if .m2 files is loaded
    if (m_m2Geom == nullptr) return false;
    if (m_m2Geom->getStatus() != FileStatus::FSLoaded) return false;
    this->createAABB();

    if (m_skinGeom == nullptr || m_skinGeom->getStatus() != FileStatus::FSLoaded) return false;
    if (m_m2Geom->m_skid > 0 && m_skelGeom->getStatus() != FileStatus::FSLoaded) {
        return false;
    }
    if (m_m2Geom->m_skid > 0 && m_skelGeom->getStatus() == FileStatus::FSLoaded &&
        m_skelGeom->m_skpd!= nullptr && m_skelGeom->m_skpd->parent_skel_file_id != 0 && (
            m_parentSkelGeom == nullptr || m_parentSkelGeom->getStatus() != FileStatus::FSLoaded
        )) {
        return false;
    }

    //3. Do post load procedures
    m_skinGeom->fixData(m_m2Geom->getM2Data());

    this->createVertexBindings();
    this->createMeshes();

    m_boneMasterData = std::make_shared<CBoneMasterData>(m_m2Geom, m_skelGeom, m_parentSkelGeom);

    this->initAnimationManager();
    this->initBoneAnimMatrices();
    this->initTextAnimMatrices();
    this->initSubmeshColors();
    this->initTransparencies();
    this->initLights();
    this->initParticleEmitters();
    this->initRibbonEmitters();


    this->m_loaded = true;
    this->m_loading = false;

    for ( auto &item : m_postLoadEvents) {
        item();
    }
    m_postLoadEvents.clear();

    return true;
}
//deltaTime = miliseconds
void M2Object::update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat) {
    if (!this->m_loaded)  return;

    static const mathfu::mat4 particleCoordinatesFix =
        mathfu::mat4(
            0,1,0,0,
            -1,0,0,0,
            0,0,1,0,
            0,0,0,1
        );

    if (m_boolSkybox && m_overrideSkyModelMat) {
        m_placementMatrix.GetColumn(3) = mathfu::vec4(cameraPos, 1.0);
        m_placementInvertMatrix = m_placementMatrix.Inverse();
    }

//    /* 1. Calc local camera */
    mathfu::vec3 cameraInlocalPos = (m_placementInvertMatrix * mathfu::vec4(cameraPos, 1)).xyz();
//
//    /* 2. Update animation values */
    mathfu::mat4 modelViewMat = viewMat * m_placementMatrix;

    float animDeltaTime = deltaTime;
    if (animationOverrideActive) {
        animDeltaTime = 0;
        this->m_animationManager->setAnimationPercent(animationOverridePercent);
    }
    this->m_animationManager->update(
        animDeltaTime,
        deltaTime,
        cameraInlocalPos,
        this->m_localUpVector,
        this->m_localRightVector,
        modelViewMat,
        this->bonesMatrices,
        this->textAnimMatrices,
        this->subMeshColors,
        this->transparencies,
        //this->cameras,
        this->lights,
        this->particleEmitters,
        this->ribbonEmitters
    );

    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());

    mathfu::mat4 viewMatInv = viewMat.Inverse();

    for (int i = minParticle; i < maxParticle; i++) {
        auto *peRecord = m_m2Geom->m_m2Data->particle_emitters.getElement(i);

        mathfu::mat4 transformMat =
            //Inverted model view is not needed here, because blizzard include modelView mat into boneMatrices for some reason
            (m_placementMatrix *
            bonesMatrices[peRecord->old.bone] *
            mathfu::mat4::FromTranslationVector(
                mathfu::vec3(peRecord->old.Position.x, peRecord->old.Position.y, peRecord->old.Position.z))
                ) *
            particleCoordinatesFix; // <- actually is there in the client

        particleEmitters[i]->Update(deltaTime * 0.001 , transformMat, viewMatInv.TranslationVector3D(), nullptr, viewMat);
        particleEmitters[i]->prepearBuffers(viewMat);
    }
    this->sortMaterials(modelViewMat);

    //Ribbon Emitters
    mathfu::vec3 nullPos(0,0,0);
    for (int i = 0; i < ribbonEmitters.size(); i++) {
        auto *ribbonRecord = m_m2Geom->m_m2Data->ribbon_emitters.getElement(i);

        mathfu::mat4 transformMat =
            //Inverted model view is not needed here, because blizzard include modelView mat into boneMatrices for some reason
            (m_placementMatrix *
             bonesMatrices[ribbonRecord->boneIndex] *
             mathfu::mat4::FromTranslationVector(
                 mathfu::vec3(ribbonRecord->position.x, ribbonRecord->position.y, ribbonRecord->position.z))
            );


        ribbonEmitters[i]->SetPos(transformMat, nullPos, nullptr);
        ribbonEmitters[i]->Update(deltaTime * 0.001f, 0);
    }
}

void M2Object::uploadGeneratorBuffers(mathfu::mat4 &viewMat) {
    if (!this->m_loaded)  return;

    mathfu::mat4 modelViewMat = viewMat * m_placementMatrix;

    M2Data * m2File = this->m_m2Geom->getM2Data();
    M2SkinProfile * skinData = this->m_skinGeom->getSkinData();

    //Manually update vertices for dynamics
    updateDynamicMeshes();

    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());

    for (int i = minParticle; i < maxParticle; i++) {
        particleEmitters[i]->updateBuffers();
    }

    for (int i = 0; i < ribbonEmitters.size(); i++) {
        ribbonEmitters[i]->updateBuffers();
    }
}

const bool M2Object::checkFrustumCulling (const mathfu::vec4 &cameraPos, const std::vector<mathfu::vec4> &frustumPlanes, const std::vector<mathfu::vec3> &frustumPoints) {
    m_cullResult = false;

    if (!this->m_hasAABB) {
        if (!this->m_loaded && !this->m_loading) {
            this->startLoading();
        }
        if (m_m2Geom->getStatus() != FileStatus::FSLoaded) return false;
        if (m_m2Geom != nullptr) {
            this->createAABB();
        }
        return false;
    }

    if (m_alwaysDraw) {
        m_cullResult = true;
        return true;
    }
    if (m_boolSkybox ) {
        return true;
    }

    CAaBox &aabb = this->aabb;

    //1. Check if camera position is inside Bounding Box
    if (
        cameraPos[0] > aabb.min.x && cameraPos[0] < aabb.max.x &&
        cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y &&
        cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
    ) {
        m_cullResult = true;
        return true;
    }

    //2. Check aabb is inside camera frustum
    bool result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);
    m_cullResult = result;
    return result;
}

void M2Object::drawDebugLight() {
/*
    std::vector<float> points;

    for (int i = 0; i < this->lights.size(); i++) {
        auto &light = this->lights[i];

        points.push_back(light.position[0]);
        points.push_back(light.position[1]);
        points.push_back(light.position[2]);

    }

    GLuint bufferVBO;
    glGenBuffers(1, &bufferVBO);
    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
    if (points.size() > 0) {
        glBufferData(GL_ARRAY_BUFFER, points.size() * 4, &points[0], GL_STATIC_DRAW);
    }

    auto drawPointsShader = m_api->getDrawPointsShader();
    static float colorArr[4] = {0.058, 0.058, 0.819607843, 0.3};
    glUniformMatrix4fv(drawPointsShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);
    glUniform3fv(drawPointsShader->getUnf("uColor"), 1, &colorArr[0]);

#ifndef WITH_GLESv2
    glEnable( GL_PROGRAM_POINT_SIZE );
#endif
    glVertexAttribPointer(+drawPoints::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);  // position


    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_POINTS, 0, points.size()/3);

#ifndef WITH_GLESv2
    glDisable( GL_PROGRAM_POINT_SIZE );
#endif
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_ZERO);
    glBindBuffer( GL_ARRAY_BUFFER, GL_ZERO);

    glDepthMask(GL_TRUE);

    glBindBuffer( GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &bufferVBO);
    */
}

void M2Object::drawBBInternal(CAaBox &bb, mathfu::vec3 &color, mathfu::mat4 &placementMatrix) {
    /*
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

     */
}

void M2Object::drawBB(mathfu::vec3 &color) {
    if (!this->m_loaded) return;

    mathfu::mat4 defMat = mathfu::mat4::Identity();
    drawBBInternal(this->aabb, color, defMat);

}

bool M2Object::prepearMaterial(M2MaterialInst &materialData, int batchIndex) {
    auto &skinSections = m_skinGeom->getSkinData()->skinSections;
    M2Array<M2Batch>* batches = &m_skinGeom->getSkinData()->batches;

    auto m2File = m_m2Geom->getM2Data();

    M2Batch* m2Batch = batches->getElement(batchIndex);
    auto skinSection = skinSections[m2Batch->skinSectionIndex];


    {
        auto meshGroup = (skinSection->skinSectionId / 100);
        if ((meshGroup < this->m_meshIds.size()) && (skinSection->skinSectionId > 0) &&
            (m_meshIds[meshGroup] != (skinSection->skinSectionId % 100))) {
            return false;
        }
    }
//        materialArray.push(materialData);

    auto op_count = m2Batch->textureCount;

    auto renderFlagIndex = m2Batch->materialIndex;
    auto isTransparent = (m2File->materials[renderFlagIndex]->blending_mode >= 2) ||
                         ((m2File->materials[renderFlagIndex]->flags & 0x10) > 0);

    materialData.layer = m2Batch->materialLayer;
    materialData.isRendered = true;
    materialData.isTransparent = isTransparent;
    materialData.renderFlagIndex = m2Batch->materialIndex;
    materialData.batchIndex = batchIndex;
    materialData.flags = m2Batch->flags;
    materialData.priorityPlane = m2Batch->priorityPlane;

    if (m_api->getConfig()->useWotlkLogic) {
        std::string vertexShader;
        std::string pixelShader;
        getShaderNames(m2Batch, vertexShader, pixelShader);
        //TODO: this his hack!!!
        if (pixelShader == "") {
            materialData.pixelShader = 0;
        } else {
            materialData.pixelShader = pixelShaderTable.at(pixelShader);
        }
    } else {
        //Legion logic
        materialData.pixelShader = getPixelShaderId(m2Batch->textureCount, m2Batch->shader_id);
        materialData.vertexShader = getVertexShaderId(m2Batch->textureCount, m2Batch->shader_id);
    }

    materialData.textureCount = op_count;
    for (int j = 0; j < op_count; j++) {
        auto m2TextureIndex = *m2File->texture_lookup_table[m2Batch->textureComboIndex + j];
        materialData.textures[j] = getTexture(m2TextureIndex);
    }

    return true;
}

void M2Object::createBoundingBoxMesh() {
    return;
    //Create bounding box mesh
    HGShaderPermutation boundingBoxshaderPermutation = m_api->hDevice->getShader("drawBBShader", nullptr);

    gMeshTemplate meshTemplate(m_api->hDevice->getBBVertexBinding(), boundingBoxshaderPermutation);

    meshTemplate.depthWrite = false;
    meshTemplate.depthCulling = true;
    meshTemplate.backFaceCulling = false;
    meshTemplate.colorMask = 0;
    meshTemplate.start = 0;
    meshTemplate.end = 36;

    meshTemplate.blendMode = EGxBlendEnum ::GxBlend_Opaque;

    meshTemplate.element = DrawElementMode::TRIANGLES;
    meshTemplate.textureCount = 0;

    HGUniformBufferChunk bbBlockVS = m_api->hDevice->createUniformBufferChunk(sizeof(bbModelWideBlockVS));

    meshTemplate.ubo[0] = nullptr; //m_api->getSceneWideUniformBuffer();
    meshTemplate.ubo[1] = bbBlockVS;
    meshTemplate.ubo[2] = nullptr;

    meshTemplate.ubo[3] = nullptr;
    meshTemplate.ubo[4] = nullptr;

    auto l_m2Geom = m_m2Geom;
    bbBlockVS->setUpdateHandler([this, l_m2Geom](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData){
        M2Data *m2Data = l_m2Geom->getM2Data();
        CAaBox &aaBox = m2Data->bounding_box;


        mathfu::vec3 center = mathfu::vec3(
            (aaBox.min.x + aaBox.max.x) / 2,
            (aaBox.min.y + aaBox.max.y) / 2,
            (aaBox.min.z + aaBox.max.z) / 2
        );

        mathfu::vec3 scale = mathfu::vec3(
            aaBox.max.x - center[0],
            aaBox.max.y - center[1],
            aaBox.max.z - center[2]
        );

        bbModelWideBlockVS &blockVS = self->getObject<bbModelWideBlockVS>();
        blockVS.uPlacementMat = m_placementMatrix;
        blockVS.uBBScale = mathfu::vec4_packed(mathfu::vec4(scale, 0.0));
        blockVS.uBBCenter = mathfu::vec4_packed(mathfu::vec4(center, 0.0));
        blockVS.uColor = mathfu::vec4_packed(mathfu::vec4(0.1f, 0.7f, 0.1f, 0.1f));
    });

    boundingBoxMesh = m_api->hDevice->createMesh(meshTemplate);
    occlusionQuery = m_api->hDevice->createQuery(boundingBoxMesh);
}

bool M2Object::checkifBonesAreInRange(M2SkinProfile *skinProfile, M2SkinSection *skinSection) {
    int16_t minBone = 9999;
    int16_t maxBone = 0;

    auto m2File = this->m_m2Geom->getM2Data();
    for (int vertIndex = skinSection->vertexStart; vertIndex < (skinSection->vertexStart + skinSection->vertexCount); ++vertIndex)
    {
        for (int boneInd = 0; boneInd < skinSection->boneInfluences; ++boneInd)
        {
            auto boneIdx = *m2File->bone_lookup_table[skinSection->boneComboIndex + (*skinProfile->bones[vertIndex])[boneInd]];
            minBone = std::min<int16_t>(minBone, boneIdx);
            maxBone = std::max<int16_t>(maxBone, boneIdx);
        }
    }

    if (maxBone >= MAX_MATRIX_NUM)
        return false;

    return true;
}

float wfv_convert(float value, int16_t random) {
    if ( value == 0.0 )
        return 0.0;

    int invertedVal = (int)(float)(1000.0 / fabs(value));

    if ( !invertedVal )
        return 0.0;

    float multiplier = 1.0;

    if ( value <= 0.0 )
        multiplier = -1.0;
    else
        multiplier = 1.0;
    return (float)((float)(int)(random % invertedVal) / (float)invertedVal) * multiplier;
}

HGM2Mesh M2Object::createWaterfallMesh() {
    HGShaderPermutation shaderPermutation = m_api->hDevice->getShader("waterfallShader", nullptr);

    gMeshTemplate meshTemplate(bufferBindings, shaderPermutation);

    auto skinData = m_skinGeom->getSkinData();
    auto m2Data = m_m2Geom->getM2Data();
    auto wfv3Data = m_m2Geom->m_wfv3 != nullptr ? m_m2Geom->m_wfv3 : m_m2Geom->m_wfv1;

    auto m2Batch = skinData->batches.getElement(0);
    auto skinSection = skinData->skinSections[m2Batch->skinSectionIndex];

    int renderFlagIndex = m2Batch->materialIndex;

    meshTemplate.depthWrite = false;
    meshTemplate.depthCulling = true;
    meshTemplate.backFaceCulling = false;
    meshTemplate.triCCW = 1;

    meshTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

    meshTemplate.start = (skinSection->indexStart + (skinSection->Level << 16)) * 2;
    meshTemplate.end = skinSection->indexCount;
    meshTemplate.element = DrawElementMode::TRIANGLES;
    meshTemplate.skybox = m_boolSkybox;

    HGTexture texture[4] = {nullptr,nullptr,nullptr,nullptr};
    meshTemplate.texture.resize(5);
    meshTemplate.textureCount = 5;
    meshTemplate.texture[0] = getTexture(0); //mask
    meshTemplate.texture[1] = getTexture(1); //whiteWater
    meshTemplate.texture[2] = getTexture(2); //noise
    meshTemplate.texture[3] = getTexture(3); //bumpTexture
    meshTemplate.texture[4] = getTexture(4); //normalTex


    meshTemplate.ubo[0] = nullptr;
    meshTemplate.ubo[1] = vertexModelWideUniformBuffer;
    meshTemplate.ubo[2] = m_api->hDevice->createUniformBufferChunk(sizeof(M2::WaterfallData::meshWideBlockVS));

    meshTemplate.ubo[2]->setUpdateHandler([this, skinData, m2Data, wfv3Data](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData){
        auto &meshblockVS = self->getObject<M2::WaterfallData::meshWideBlockVS>();
        meshblockVS.bumpScale = mathfu::vec4(wfv3Data->bumpScale, 0, 0, 0);

        M2MeshBufferUpdater::fillTextureMatrices(*this, 0, m2Data, skinData, meshblockVS.uTextMat);
    });

    meshTemplate.ubo[3] = nullptr;
    meshTemplate.ubo[4] = m_api->hDevice->createUniformBufferChunk(sizeof(M2::WaterfallData::meshWideBlockPS));
    meshTemplate.ubo[4]->setUpdateHandler([this, skinData, m2Data, wfv3Data](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData){
        auto &meshblockPS = self->getObject<M2::WaterfallData::meshWideBlockPS>();
        meshblockPS.baseColor = mathfu::vec4(
            wfv3Data->basecolor.a / 255.0f,
            wfv3Data->basecolor.r / 255.0f,
            wfv3Data->basecolor.g / 255.0f,
            wfv3Data->basecolor.b / 255.0f);

        meshblockPS.values0.x = wfv3Data->values0_x;
        meshblockPS.values0.y = wfv3Data->values0_y;
        meshblockPS.values0.z = wfv3Data->values0_z;
        meshblockPS.values0.w = wfv3Data->values0_w;

        meshblockPS.values1.x = wfv3Data->value1_x;
        meshblockPS.values1.y = wfv3Data->value1_y;
        meshblockPS.values1.w = wfv3Data->value1_w;

        meshblockPS.m_values3.x = wfv3Data->value3_x;
        meshblockPS.m_values3.y = wfv3Data->value3_y;

        meshblockPS.m_values2.x = wfv3Data->flags & 2;
        meshblockPS.m_values2.y = wfv3Data->flags & 1;

        meshblockPS.m_values2.w = wfv3Data->value2_w;
        meshblockPS.m_values3.w = wfv3Data->values3_w;
        meshblockPS.m_values4.y = wfv3Data->values4_y;

        meshblockPS.values1.z =   wfv_convert(meshblockPS.values1.y, (int16_t)((uint64_t)this));
        meshblockPS.m_values2.z = wfv_convert(meshblockPS.m_values2.w, (int16_t)((uint64_t)this));
        meshblockPS.m_values3.z = wfv_convert(wfv3Data->values3_z, (int16_t)((uint64_t)this));
    });

    //Make mesh
    auto hmesh = m_api->hDevice->createM2Mesh(meshTemplate);
    hmesh->setM2Object(this);
    hmesh->setLayer(0);
    hmesh->setPriorityPlane(0);
    hmesh->setQuery(nullptr);

    return hmesh;
}

void M2Object::createMeshes() {
    /* 1. Free previous subMeshArray */
    this->m_meshNaturalArray.clear();
    this->m_meshForcedTranspArray.clear();
    this->m_materialArray.clear();

    createBoundingBoxMesh();

    if (bufferBindings == nullptr)
        return;

    M2SkinProfile* skinProfile = this->m_skinGeom->getSkinData();
    auto m_m2Data = m_m2Geom->getM2Data();

    /* 2. Fill the materialArray */
    std::vector<int> batchesRequiringDynamicVao = {};
    M2Array<M2Batch>* batches = &m_skinGeom->getSkinData()->batches;

    if (m_m2Geom->m_wfv3 == nullptr && m_m2Geom->m_wfv1 == nullptr) {
        for (int i = 0; i < batches->size; i++) {
            auto m2Batch = skinProfile->batches[i];
            auto skinSection = skinProfile->skinSections[m2Batch->skinSectionIndex];
            if (!checkifBonesAreInRange(skinProfile, skinSection)) {
                batchesRequiringDynamicVao.push_back(i);
                continue;
            }
            M2MaterialInst material;
            EGxBlendEnum mainBlendMode;
            HGM2Mesh hmesh = createSingleMesh(m_m2Data, i, 0, bufferBindings, m2Batch, skinSection, material,
                                              mainBlendMode, false);

            if (hmesh == nullptr)
                continue;

            this->m_materialArray.push_back(material);
            this->m_meshNaturalArray.push_back(hmesh);
            M2MeshBufferUpdater::assignUpdateEvents(hmesh, this, m_materialArray[m_materialArray.size() - 1], m_m2Data,
                                                    skinProfile);

            if (mainBlendMode == EGxBlendEnum::GxBlend_Opaque) {
                EGxBlendEnum blendMode = EGxBlendEnum::GxBlend_Alpha;
                M2MaterialInst materialTransp;
                HGM2Mesh hmeshTrans = createSingleMesh(m_m2Data, i, 0, bufferBindings, m2Batch, skinSection,
                                                       materialTransp, blendMode, true);

                this->m_materialArray.push_back(material);
                this->m_meshForcedTranspArray.push_back(hmeshTrans);
                M2MeshBufferUpdater::assignUpdateEvents(hmeshTrans, this, m_materialArray[m_materialArray.size() - 1],
                                                        m_m2Data, skinProfile);

            } else {
                m_meshForcedTranspArray.push_back(nullptr);
            }
        }

        // Create meshes requiring dynamic
        for (int j = 0; j < batchesRequiringDynamicVao.size(); j++) {
            int i = batchesRequiringDynamicVao[j];
            auto m2Batch = skinProfile->batches[i];
            auto skinSection = skinProfile->skinSections[m2Batch->skinSectionIndex];

            std::array<HGVertexBufferDynamic, 4> dynVBOs;
            auto dynVaos = m_m2Geom->createDynamicVao(*m_api->hDevice, dynVBOs, m_skinGeom.get(), skinSection);

            std::array<dynamicVaoMeshFrame, 4> dynamicMeshData;

            //Try to create mesh
            M2MaterialInst testMaterial;
            EGxBlendEnum blendMode;
            auto testMesh = createSingleMesh(m_m2Data, i, 0, dynVaos[0], m2Batch, skinSection, testMaterial, blendMode,
                                             false);
            if (testMesh == nullptr)
                continue;

            for (int k = 0; k < 4; k++) {
                dynamicMeshData[k].batchIndex = i;
                dynamicMeshData[k].m_bindings = dynVaos[k];
                dynamicMeshData[k].m_bufferVBO = dynVBOs[k];

                M2MaterialInst material;
                int correction = skinSection->indexStart + (skinSection->Level << 16);
                dynamicMeshData[k].m_mesh = createSingleMesh(m_m2Data, i, correction, dynVaos[k], m2Batch, skinSection,
                                                             material, blendMode, false);

                this->m_materialArray.push_back(material);
                M2MeshBufferUpdater::assignUpdateEvents(dynamicMeshData[k].m_mesh, this,
                                                        m_materialArray[m_materialArray.size() - 1], m_m2Data,
                                                        skinProfile);
            }

            dynamicMeshes.push_back(dynamicMeshData);
        }
    } else {
        m_meshNaturalArray.push_back(createWaterfallMesh());
    }
}

HGM2Mesh
M2Object::createSingleMesh(const M2Data *m_m2Data, int i, int indexStartCorrection, HGVertexBufferBindings finalBufferBindings, const M2Batch *m2Batch,
                           const M2SkinSection *skinSection, M2MaterialInst &material, EGxBlendEnum &blendMode, bool overrideBlend) {

    if (!prepearMaterial(material, i)) return nullptr;

    M2ShaderCacheRecord cacheRecord{};
    cacheRecord.vertexShader = material.vertexShader;
    cacheRecord.pixelShader  = material.pixelShader;
    cacheRecord.unlit = true;
    cacheRecord.alphaTestOn = true;
    cacheRecord.unFogged = true;
    cacheRecord.unShadowed = true;
    if (skinSection->boneInfluences > 0) {
        cacheRecord.boneInfluences = skinSection->boneInfluences;
    } else {
        cacheRecord.boneInfluences = skinSection->boneCount > 0 ? 1 : 0;
    }
    HGShaderPermutation shaderPermutation = m_api->hDevice->getShader("m2Shader", &cacheRecord);

    gMeshTemplate meshTemplate(finalBufferBindings, shaderPermutation);

    int renderFlagIndex = m2Batch->materialIndex;
    auto renderFlag = m_m2Data->materials[renderFlagIndex];

    meshTemplate.depthWrite = !(renderFlag->flags & 0x10);
    meshTemplate.depthCulling = !(renderFlag->flags & 0x8);
    meshTemplate.backFaceCulling = !(renderFlag->flags & 0x4);
    meshTemplate.triCCW = 1;

    if (overrideBlend) {
        meshTemplate.blendMode = blendMode;
    } else {
        meshTemplate.blendMode = M2BlendingModeToEGxBlendEnum[renderFlag->blending_mode];
        blendMode = meshTemplate.blendMode;
    }

    meshTemplate.start = (skinSection->indexStart + (skinSection->Level << 16) - indexStartCorrection) * 2;
    meshTemplate.end = skinSection->indexCount;
    meshTemplate.element = DrawElementMode::TRIANGLES;
    meshTemplate.skybox = m_boolSkybox;

    HGTexture texture[4] = {nullptr,nullptr,nullptr,nullptr};
    meshTemplate.texture.resize(m2Batch->textureCount);
    meshTemplate.textureCount = m2Batch->textureCount;
    for (int j = 0; j < material.textureCount; j++) {
        meshTemplate.texture[j] = material.textures[j];
    }
    meshTemplate.ubo[0] = nullptr;
    meshTemplate.ubo[1] = vertexModelWideUniformBuffer;
    meshTemplate.ubo[2] = m_api->hDevice->createUniformBufferChunk(sizeof(M2::meshWideBlockVS));

    meshTemplate.ubo[3] = fragmentModelWideUniformBuffer;
    meshTemplate.ubo[4] = m_api->hDevice->createUniformBufferChunk(sizeof(M2::meshWideBlockPS));

    //Make mesh
    auto hmesh = m_api->hDevice->createM2Mesh(meshTemplate);
    hmesh->setM2Object(this);
    hmesh->setLayer(m2Batch->materialLayer);
    hmesh->setPriorityPlane(m2Batch->priorityPlane);
    hmesh->setQuery(nullptr);

    return hmesh;
}

void M2Object::collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes, int renderOrder) {
    //2. Check if .skin file is loaded
    if (m_m2Geom == nullptr) {
        return;
    }

    if (m_skinGeom == nullptr) {
        Cache<SkinGeom> *skinGeomCache = m_api->cacheStorage->getSkinGeomCache();
        if (m_m2Geom->skinFileDataIDs.size() > 0) {
            assert(m_m2Geom->skinFileDataIDs.size() > 0);
            m_skinGeom = skinGeomCache->getFileId(m_m2Geom->skinFileDataIDs[0]);
        } else if (!useFileId){
            assert(m_nameTemplate.size() > 0);
            std::string skinFileName = m_nameTemplate + "00.skin";
            m_skinGeom = skinGeomCache->get(skinFileName);
        }
        return;
    }

    if (m_m2Geom->m_skid > 0) {
        auto skelCache = m_api->cacheStorage->getSkelCache();
        if (m_skelGeom == nullptr) {
            auto skelCache = m_api->cacheStorage->getSkelCache();
            m_skelGeom = skelCache->getFileId(m_m2Geom->m_skid);
            return;
        }
        if (m_skelGeom->getStatus() == FileStatus::FSLoaded && m_parentSkelGeom == nullptr) {
            if (m_skelGeom->m_skpd != nullptr && m_skelGeom->m_skpd->parent_skel_file_id != 0) {
                m_parentSkelGeom = skelCache->getFileId(m_skelGeom->m_skpd->parent_skel_file_id);
            }
        }
    }

    if (!m_loaded) return;

    M2SkinProfile* skinData = this->m_skinGeom->getSkinData();

    int minBatch = m_api->getConfig()->m2MinBatch;
    int maxBatch = std::min(m_api->getConfig()->m2MaxBatch, (const int &) this->m_meshNaturalArray.size());

    for (int i = minBatch; i < maxBatch; i++) {
        float finalTransparency = M2MeshBufferUpdater::calcFinalTransparency(*this, i, skinData);
        if ((finalTransparency < 0.0001) )
            continue;

        HGM2Mesh mesh = this->m_meshNaturalArray[i];
        if (finalTransparency < 0.999 && i < this->m_meshForcedTranspArray.size() && this->m_meshForcedTranspArray[i] != nullptr) {
            mesh = this->m_meshForcedTranspArray[i];
        }

        mesh->setRenderOrder(renderOrder);
        if (mesh->getIsTransparent()) {
            transparentMeshes.push_back(mesh);
        } else {
            opaqueMeshes.push_back(mesh);
        }
    }

    for (auto &dynMesh : dynamicMeshes) {
        HGParticleMesh mesh = dynMesh[m_api->hDevice->getUpdateFrameNumber()].m_mesh;
        mesh->setRenderOrder(renderOrder);
        if (mesh->getIsTransparent()) {
            transparentMeshes.push_back(mesh);
        } else {
            opaqueMeshes.push_back(mesh);
        }
    }

//    transparentMeshes.push_back(boundingBoxMesh);
//    std::cout << "Collected meshes at update frame =" << m_api->hDevice->getUpdateFrameNumber() << std::endl;

//    renderedThisFrame.push_back(occlusionQuery);
}

void M2Object::initAnimationManager() {

    this->m_animationManager = new AnimationManager(m_api, m_boneMasterData, m_m2Geom->exp2 != nullptr);
}

void M2Object::initBoneAnimMatrices() {
    auto &bones = *m_boneMasterData->getSkelData()->m_m2CompBones;
    this->bonesMatrices = std::vector<mathfu::mat4>(bones.size, mathfu::mat4::Identity());;
}
void M2Object::initTextAnimMatrices() {
    textAnimMatrices = std::vector<mathfu::mat4>(m_m2Geom->getM2Data()->texture_transforms.size, mathfu::mat4::Identity());;
}

void M2Object::initSubmeshColors() {
    subMeshColors = std::vector<mathfu::vec4>(m_m2Geom->getM2Data()->colors.size);

}
void M2Object::initTransparencies() {
    transparencies = std::vector<float>(m_m2Geom->getM2Data()->texture_weights.size);
}

void M2Object::initLights() {
    lights = std::vector<M2LightResult>(m_m2Geom->getM2Data()->lights.size);
}
void M2Object::initParticleEmitters() {
//    return;
    particleEmitters = std::vector<ParticleEmitter *>();
//    particleEmitters.reserve(m_m2Geom->getM2Data()->particle_emitters.size);
    for (int i = 0; i < m_m2Geom->getM2Data()->particle_emitters.size; i++) {
        int txacVal = 0;
        if (m_m2Geom->txacMParticle.size() > 0) {
            txacVal = m_m2Geom->txacMParticle[i].value;
        }

        ParticleEmitter *emitter = new ParticleEmitter(m_api, m_m2Geom->getM2Data()->particle_emitters.getElement(i), this, m_m2Geom, txacVal);
        particleEmitters.push_back(emitter);
        if (m_m2Geom->exp2 != nullptr && emitter->getGenerator() != nullptr) {
            emitter->getGenerator()->getAniProp()->zSource = m_m2Geom->exp2->content.getElement(i)->zSource;
        }
    }
}

void M2Object::initRibbonEmitters() {
//    return;
    ribbonEmitters = std::vector<CRibbonEmitter *>();
//    ribbonEmitters.reserve(m_m2Geom->getM2Data()->ribbon_emitters.size);
    for (int i = 0; i < m_m2Geom->getM2Data()->ribbon_emitters.size; i++) {
        M2Ribbon *m2Ribbon = m_m2Geom->getM2Data()->ribbon_emitters.getElement(i);

        std::vector<M2Material> materials(m2Ribbon->materialIndices.size);
        std::vector<int> textureIndicies(m2Ribbon->textureIndices.size);
        for (size_t j = 0; j < materials.size(); j++) {
            materials[j] = *m_m2Geom->getM2Data()->materials[*m2Ribbon->materialIndices[j]];
        }

        for (size_t j = 0; j < textureIndicies.size(); j++) {
            textureIndicies[j] = *m2Ribbon->textureIndices[j];
        }

        auto emitter = new CRibbonEmitter(m_api, this, materials, textureIndicies);
        ribbonEmitters.push_back(emitter);

        CImVector color;
        color.r = 255;
        color.g = 255;
        color.b = 255;
        color.a = 255;
        CRect rect;
        rect.miny = 0.0;
        rect.minx = 0.0;
        rect.maxy = 1.0;
        rect.maxx = 1.0;

        emitter->Initialize(m2Ribbon->edgesPerSecond, m2Ribbon->edgeLifetime, color, &rect, m2Ribbon->textureCols, m2Ribbon->textureRows);
        emitter->SetGravity(m2Ribbon->gravity);
        emitter->SetPriority(m2Ribbon->priorityPlane);
        emitter->SetDataEnabled(0);
    }
}

void M2Object::setModelFileName(std::string modelName) {

    std::string delimiter = ".";
    std::string nameTemplate = modelName.substr(0, modelName.find_last_of(delimiter));
    std::string modelFileName = nameTemplate + ".m2";

    this->m_modelName = modelFileName;
    this->m_nameTemplate= nameTemplate;
}

int M2Object::getModelFileId() {
    return m_modelFileId;
}

void M2Object::setModelFileId(int fileId) {
    useFileId = true;
    m_modelFileId = fileId;
}

void M2Object::setAnimationId(int animationId) {
    if (!m_loaded) return;

    m_animationManager->setAnimationId(animationId, false);
}
void M2Object::resetCurrentAnimation() {
    if (!m_loaded) return;

    m_animationManager->resetCurrentAnimation();
}

M2CameraResult M2Object::updateCamera(double deltaTime, int cameraId) {
    M2CameraResult result;
    m_animationManager->calcCamera(result, cameraId, m_placementMatrix);

    return result;
}
mathfu::vec4 M2Object::getM2SceneAmbientLight() {
    mathfu::vec4 ambientColor = mathfu::vec4(0,0,0,0);
    for (int i = 0; i < lights.size(); ++i) {
        if (lights[i].ambient_intensity > 0) {
            ambientColor += lights[i].ambient_color * lights[i].ambient_intensity;
        }
    }

    return mathfu::vec4(ambientColor.x, ambientColor.y, ambientColor.z, 1.0) ;
};

void M2Object::getAvailableAnimation(std::vector<int> &allAnimationList) {
    auto &sequences = *m_boneMasterData->getSkelData()->m_sequences;

    allAnimationList.resize(0);
    for (int i = 0; i < sequences.size; i++) {
        allAnimationList.push_back(sequences[i]->id);
    }
    if (m_parentSkelGeom != nullptr) {
        auto &bannedAnims = m_m2Geom->blackListAnimations;
        auto &sequences = *m_boneMasterData->getParentSkelData()->m_sequences;

        for (int i = 0; i < sequences.size; i++) {
            bool animationIsBanned = false;
            for (auto const a : bannedAnims) {
                if (a != 0 && a == sequences[i]->id) {
                    animationIsBanned = true;
                    break;
                }
            }
            if (!animationIsBanned) {
                allAnimationList.push_back(sequences[i]->id);
            }
        }
    }

    std::sort( allAnimationList.begin(), allAnimationList.end());
    allAnimationList.erase( unique( allAnimationList.begin(), allAnimationList.end() ), allAnimationList.end());
}


void M2Object::drawParticles(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes, int renderOrder) {
//    return;
//        for (int i = 0; i< std::min((int)particleEmitters.size(), 10); i++) {
    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());
//    int maxBatch = particleEmitters.size();


    for (int i = minParticle; i < maxParticle; i++) {
//    for (int i = 0; i< particleEmitters.size(); i++) {
        particleEmitters[i]->collectMeshes(opaqueMeshes, transparentMeshes, renderOrder);
    }

    for (int i = 0; i < ribbonEmitters.size(); i++) {
        ribbonEmitters[i]->collectMeshes(opaqueMeshes, transparentMeshes, renderOrder);
    }
}

HBlpTexture M2Object::getBlpTextureData(int textureInd) {
    M2Texture* textureDefinition = m_m2Geom->getM2Data()->textures.getElement(textureInd);
    //TODO:! Example of exception: "WORLD\\AZEROTH\\KARAZAHN\\PASSIVEDOODADS\\BURNINGBOOKS\\BOOKSONFIRE.m2"
    HBlpTexture blpData = nullptr;

    if ((textureDefinition== nullptr) || textureDefinition->type == 0) {
        blpData = getHardCodedTexture(textureInd);
    } else if ((textureDefinition != nullptr) && (textureDefinition->type < this->m_replaceTextures.size()) ){
        blpData = this->m_replaceTextures[textureDefinition->type];
    }

    return blpData;
}

HGTexture M2Object::getTexture(int textureInd) {
    std::unordered_map<int, HBlpTexture> &loadedTextureCache = loadedTextures;
    M2Texture* textureDefinition = m_m2Geom->getM2Data()->textures.getElement(textureInd);

    HBlpTexture blpData = getBlpTextureData(textureInd);

    if (blpData == nullptr)
        return nullptr;

    HGTexture hgTexture = m_api->hDevice->createBlpTexture(
        blpData,
        textureDefinition!= nullptr ? ( (textureDefinition->flags & 1) > 0 ) : false,
        textureDefinition!= nullptr ? ( (textureDefinition->flags & 2) > 0 ) : false
    );

    return hgTexture;
}

HBlpTexture M2Object::getHardCodedTexture(int textureInd) {
    M2Texture* textureDefinition = m_m2Geom->getM2Data()->textures.getElement(textureInd);
    auto textureCache = m_api->cacheStorage->getTextureCache();
    HBlpTexture texture;
    if (textureDefinition != nullptr && textureDefinition->filename.size > 0) {
        std::string fileName = textureDefinition->filename.toString();
        texture = textureCache->get(fileName);
    } else if (textureInd < m_m2Geom->textureFileDataIDs.size()) {
        int textureFileDataId = m_m2Geom->textureFileDataIDs[textureInd];
        if (textureFileDataId > 0) {
            texture = textureCache->getFileId(textureFileDataId);
        } else {
            texture = nullptr;
        }
    }

    return texture;
}

void M2Object::createVertexBindings() {
    std::shared_ptr<IDevice> device = m_api->hDevice;

    //2. Create buffer binding and fill it
    bufferBindings = m_m2Geom->getVAO(*device, m_skinGeom.get());

    //3. Create model wide uniform buffer
//    vertexModelWideUniformBuffer = device->createUniformBuffer(sizeof(mathfu::mat4) * (m_m2Geom->m_m2Data->bones.size + 1));
    vertexModelWideUniformBuffer = device->createUniformBufferChunk(sizeof(M2::modelWideBlockVS));
    fragmentModelWideUniformBuffer = device->createUniformBufferChunk(sizeof(M2::modelWideBlockPS));

    vertexModelWideUniformBuffer->setUpdateHandler([this](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData){
        auto &blockVS = self->getObject<M2::modelWideBlockVS>();

        blockVS.uPlacementMat = m_placementMatrix;
        int interCount = (int) std::min(bonesMatrices.size(), (size_t) MAX_MATRIX_NUM);
        std::copy(bonesMatrices.data(), bonesMatrices.data() + interCount, blockVS.uBoneMatrixes);
    });

    fragmentModelWideUniformBuffer->setUpdateHandler([this](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData){
        static mathfu::vec4 diffuseNon(0.0, 0.0, 0.0, 0.0);
        mathfu::vec4 localDiffuse = diffuseNon;

        M2::modelWideBlockPS &blockPS = self->getObject<M2::modelWideBlockPS>();

        blockPS.intLight.uInteriorAmbientColorAndApplyInteriorLight =
            mathfu::vec4_packed(mathfu::vec4(
                m_ambientColorOverride.xyz(),
                m_useLocalDiffuseColor == 1 ? 1.0 : 0
            ));

        blockPS.intLight.uInteriorDirectColorAndApplyExteriorLight =
            mathfu::vec4_packed(mathfu::vec4(
                m_localDiffuseColorV.xyz(),
                m_useLocalDiffuseColor == 1 ? 0.0 : 1
            ));

        blockPS.interiorExteriorBlend =
            mathfu::vec4_packed(mathfu::vec4(
                (m_useLocalDiffuseColor == 1) ? 1.0 : 0.0,
                0,0,0));

        //Lights
        M2MeshBufferUpdater::fillLights(*this, blockPS);
//        blockPS.uViewUp = mathfu::vec4_packed(mathfu::vec4(m_api->getViewUp(), 0.0));
//        blockPS.uSunDirAndFogStart = mathfu::vec4_packed(mathfu::vec4(getSunDir(), m_api->getGlobalFogStart()));
//        blockPS.uSunColorAndFogEnd = mathfu::vec4_packed(mathfu::vec4(localDiffuse.xyz(), m_api->getGlobalFogEnd()));
    });
}

void M2Object::updateDynamicMeshes() {
    auto rootMatInverse = bonesMatrices[0].Inverse();
    auto frameNum = m_api->hDevice->getUpdateFrameNumber();

    for (auto &dynamicMesh: dynamicMeshes) {
        auto &dynMeshData = dynamicMesh[frameNum];

        M2SkinProfile* skinProfile = this->m_skinGeom->getSkinData();
        auto m2Data = m_m2Geom->getM2Data();

        auto m2Batch = skinProfile->batches[dynMeshData.batchIndex];
        auto skinSection = skinProfile->skinSections[m2Batch->skinSectionIndex];

        M2Vertex *overrideVertexes = (M2Vertex *)dynMeshData.m_bufferVBO->getPointerForModification();

        for (int vertIndex = skinSection->vertexStart;
             vertIndex < (skinSection->vertexStart + skinSection->vertexCount); ++vertIndex) {
            auto &overrideVert = overrideVertexes[vertIndex - skinSection->vertexStart];
            overrideVert = *m2Data->vertices[vertIndex];

            mathfu::mat4 matrix = mathfu::mat4::Identity();
            if (overrideVert.bone_indices[0] > 0) {
                matrix = bonesMatrices[overrideVert.bone_indices[0]] * overrideVert.bone_weights[0];
            }

            for (int i = 1; i < MAX_BONES_PER_VERTEX; i++) {
                if (overrideVert.bone_indices[i] > 0) {
                    matrix += (bonesMatrices[overrideVert.bone_indices[i]] * overrideVert.bone_weights[i]);
                }
            }
            overrideVert.pos =
                mathfu::vec3_packed(rootMatInverse * matrix * mathfu::vec4(mathfu::vec3(overrideVert.pos), 1.0).xyz());


            overrideVert.bone_indices[0] = 0;
            overrideVert.bone_indices[1] = 0;
            overrideVert.bone_indices[2] = 0;
            overrideVert.bone_indices[3] = 0;

//            overrideVert.bone_weights[0] = 1;
//            overrideVert.bone_weights[1] = 0;
//            overrideVert.bone_weights[2] = 0;
//            overrideVert.bone_weights[3] = 0;
        }

        dynMeshData.m_bufferVBO->save(skinSection->vertexCount*sizeof(M2Vertex));
//        std::cout << "Saved " << skinSection->vertexCount << " vertices " << "at update frame =" << frameNum << std::endl;
    }
}
void M2Object::setReplaceTextures(std::vector<HBlpTexture> &replaceTextures) {
    m_replaceTextures = replaceTextures;

    if (m_loaded) {
        createMeshes(); // recreate meshes
    }
}
void M2Object::setMeshIds(std::vector<uint8_t> &meshIds) {
    m_meshIds = meshIds;

    if (m_loaded) {
        createMeshes(); // recreate meshes
    }
}

void M2Object::setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {
    m_particleColorReplacement = particleColorReplacement;
    particleColorReplacementIsSet = true;
    std::cout << "particleColorReplacementIsSet = " << particleColorReplacementIsSet << std::endl;
}

bool M2Object::getReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {
    if (particleColorReplacementIsSet) {
        particleColorReplacement = m_particleColorReplacement;
        return true;
    }

    return false;
}

void M2Object::resetReplaceParticleColor() {
    particleColorReplacementIsSet = false;
}

int M2Object::getCurrentAnimationIndex() {
    return m_animationManager->getCurrentAnimationIndex();
}
