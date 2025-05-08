//
// Created by deamon on 22.06.17.
//

#include <locale>
#include <iomanip>
#include <unordered_set>
#include <cmath>
#include "m2Object.h"

#include <fstream>

#include "mathfu/matrix.h"
#include "m2Helpers/M2MeshBufferUpdater.h"
#include "../../../gapi/interface/FrameContext.h"
#include "../../../gapi/interface/sortLambda.h"

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
    Combiners_Mod_Mod_Depth = 36
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

extern const std::array<EGxBlendEnum,8> M2BlendingModeToEGxBlendEnum;
const std::array<EGxBlendEnum,8> M2BlendingModeToEGxBlendEnum =
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
        { +M2PixelShader::Combiners_Mod_Mod_Depth,                     +M2VertexShader::Diffuse_EdgeFade_T1_T2, 1, 1 },
}};

int getVertexShaderId(int textureCount, int16_t shaderId) {
    int result;
    if ( shaderId >= 0 ) {
        if ( textureCount == 1 ) {
            if ( (shaderId & 0x80u) == 0 ) {
                return (shaderId & 0x4000) != 0 ? 10 : 0;
            } else {
                result = 1;
            }
        }
        else if ( (shaderId & 0x80u) == 0 )
        {
            if ( (shaderId & 8) != 0 )
            {
                return 3;
            }
            else
            {
                result = 7;
                if ( (shaderId & 0x4000) != 0 )
                    return 2;
            }
        }
        else if ( (shaderId & 8) != 0 )
        {
            return 5;
        }
        else
        {
            return 4;
        }
    } else if ( shaderId < 0 )
    {
        int vertexShaderId = shaderId & 0x7FFF;
        if ( (unsigned int)vertexShaderId >= M2ShaderTable.size()) {
            std::cout << "Wrong shaderId for vertex shader";
            assert(false);
        }
        result = (unsigned int)M2ShaderTable[(shaderId & 0x7FFF)].vertex;
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
        if ( (shaderId & 0x70) != 0 ) {
            switch (shaderId & 7) {
                case 0 :
                    result = +M2PixelShader::Combiners_Mod_Opaque;
                    break;
                case 1 :
                case 2 :
                case 5 :
                    result = +M2PixelShader::Combiners_Mod_Mod;
                    break;
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
                case 1:
                case 2:
                case 5:
                    result = +M2PixelShader::Combiners_Opaque_Mod;
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


void M2Object::createAABB() {
    const M2Data *m2Data = m_m2Geom->getM2Data();

    //Debug: calc bounding box from verticies
    if (false)
    {
        auto min = mathfu::vec3(9999, 9999, 9999);
        auto max = mathfu::vec3(-9999, -9999, -9999);

//        auto m2SkinProfile = m_skinGeom->getSkinData();
//        for (int batchIndex = 0; batchIndex < m2SkinProfile->batches.size; batchIndex++) {
//            M2Batch *textMaterial = m2SkinProfile->batches.getElement(batchIndex);
//            M2SkinSection *skinSection = m2SkinProfile->skinSections.getElement(textMaterial->skinSectionIndex);
//
//            int vertexIndexStart = skinSection->indexStart + (skinSection->Level << 16);
//
//            for (int i = 0; i < skinSection->indexCount; i++) {
//                auto *vertex = m2Data->vertices.getElement(vertexIndexStart+i);
//
//                min = mathfu::vec3(
//                    mathfu::vec3(
//                        std::min(min.x, vertex->pos.x),
//                        std::min(min.y, vertex->pos.y),
//                        std::min(min.z, vertex->pos.z)
//                    )
//                );
//
//                max = mathfu::vec3(
//                    mathfu::vec3(
//                        std::max(max.x, vertex->pos.x),
//                        std::max(max.y, vertex->pos.y),
//                        std::max(max.z, vertex->pos.z)
//                    )
//                );
//            }
//        }
        for (int i = 0; i < m2Data->vertices.size; i++) {
            auto *vertex = m2Data->vertices.getElement(i);
            min = mathfu::vec3(
                    mathfu::vec3(
                        std::min(min.x, vertex->pos.x),
                        std::min(min.y, vertex->pos.y),
                        std::min(min.z, vertex->pos.z)
                    )
                );

                max = mathfu::vec3(
                    mathfu::vec3(
                        std::max(max.x, vertex->pos.x),
                        std::max(max.y, vertex->pos.y),
                        std::max(max.z, vertex->pos.z)
                    )
                );
        }

        //TODO: undo this :D

        mathfu::vec4 minVec = mathfu::vec4(min.x - 10, min.y - 10, min.z - 10, 1);
        mathfu::vec4 maxVec = mathfu::vec4(max.x + 10, max.y + 10, max.z + 10, 1);
        if (min.x > max.x) {
            minVec = {0,0,0,1};
            maxVec = {0,0,0,1};
        }

        CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, minVec, maxVec);

        setAABB(worldAABB);
    }


    if (true)
    {
        C3Vector min = m2Data->bounding_box.min;
        C3Vector max = m2Data->bounding_box.max;
        mathfu::vec4 minVec = mathfu::vec4(min.x, min.y, min.z, 1);
        mathfu::vec4 maxVec = mathfu::vec4(max.x, max.y, max.z, 1);

        CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix, minVec, maxVec);

        //this.diameter = vec3.distance(worldAABB[0],worldAABB[1]);
        setAABB(worldAABB);
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
    status->m_hasAABB = true;
}

void M2Object::setSize(float newSize) {

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

void M2Object::createPlacementMatrix(const SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat) {
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
    m_placementMatrixChanged = true;

    m_localUpVector = (invertPlacementMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();

    m_scale = placementMatrix.GetColumn(0).Length();
}

void M2Object::createPlacementMatrix(const SMDoodadDef &def) {
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
    m_placementMatrixChanged = true;

    m_localUpVector = (placementInvertMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();
    m_localRightVector = (placementInvertMatrix * mathfu::vec4(1,0,0,0)).xyz().Normalized();
    m_scale = placementMatrix.GetColumn(0).Length();
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
    m_placementMatrixChanged = true;

    m_localUpVector = (placementInvertMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();
    m_localRightVector = (placementInvertMatrix * mathfu::vec4(1,0,0,0)).xyz().Normalized();
    m_scale = placementMatrix.GetColumn(0).Length();
}

void M2Object::updatePlacementMatrixFromParentAttachment(M2Object *parent, int attachment, float scale) {
    if (!parent->status->m_loaded) return;
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
    m_placementMatrixChanged = true;

    m_localUpVector = (placementInvertMatrix * mathfu::vec4(0,0,1,0)).xyz().Normalized();
    m_localRightVector = (placementInvertMatrix * mathfu::vec4(1,0,0,0)).xyz().Normalized();
}


void M2Object::calcDistance(const mathfu::vec3 &cameraPos) {
    const CAaBox &aabb = this->getAABB();
    mathfu::vec3 closestPoint = mathfu::vec3::Max(mathfu::vec3(aabb.min),
                                                   mathfu::vec3::Min(mathfu::vec3(aabb.max), cameraPos));
    m_currentDistance = (closestPoint - cameraPos).Length();
}

float M2Object::getCurrentDistance() {
    return m_currentDistance;
}
float M2Object::getHeight(){
    const auto &aabb = this->getAABB();
    return aabb.max.z - aabb.min.z;
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

void M2Object::setInteriorDirectColor(const mathfu::vec3 &interiorDirectColor) {
    if (interiorDirectColor != this->m_interiorDirectColor) {
        m_modelWideDataChanged = true;
    }

    this->m_interiorDirectColor = interiorDirectColor;
}
void M2Object::setLoadParams (int skinNum, std::vector<uint8_t> meshIds, std::vector<HBlpTexture> replaceTextures) {
    this->m_skinNum = skinNum;
    this->m_meshIds = meshIds;
    this->m_replaceTextures = replaceTextures;
    this->status = m2Factory->getObjectById<2>(this->getObjectId());
    *this->status = M2LoadedStatus();
}

void M2Object::startLoading() {
    if (!status->m_loading) {
        status->m_loading = true;

        Cache<M2Geom> *m2GeomCache = m_api->cacheStorage->getM2GeomCache();
        if (!useFileId) {
            m_m2Geom = m2GeomCache->get(m_modelName);
        } else {
            m_m2Geom = m2GeomCache->getFileId(m_modelFileId);
        }
    }
}

void M2Object::sortMaterials(mathfu::mat4 &modelViewMat) {
    if (!status->m_loaded) return;

    const M2Data * m2File = this->m_m2Geom->getM2Data();
    const M2SkinProfile * skinData = this->m_skinGeom->getSkinData();

    if (m_m2Geom->m_wfv3 == nullptr && m_m2Geom->m_wfv1 == nullptr) {
        for (int i = 0; i < this->m_meshArray.size(); i++) {
            //Update info for sorting
            M2MeshBufferUpdater::updateSortData(std::get<0>(this->m_meshArray[i]), *this,
                                                std::get<1>(this->m_meshArray[i]), m2File,
                                                skinData, modelViewMat);
        }
        for (int i = 0; i < this->m_meshForcedTranspArray.size(); i++) {
            //Update info for sorting
            if (std::get<0>(this->m_meshForcedTranspArray[i]) != nullptr) {
                M2MeshBufferUpdater::updateSortData(std::get<0>(this->m_meshForcedTranspArray[i]), *this,
                                                    std::get<1>(this->m_meshForcedTranspArray[i]), m2File,
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

void M2Object::doLoadMainFile(){
    if (!this->status->m_loaded && !this->status->m_loading) {
        this->startLoading();
    }

    if (m_m2Geom == nullptr) return;
    if (m_m2Geom->getStatus() != FileStatus::FSLoaded) return;
    this->createAABB();
}
bool M2Object::isFailedToLoadMainFile() {
    return m_m2Geom != nullptr && m_m2Geom->getStatus() == FileStatus::FSRejected;
}
bool M2Object::isFailedToLoadGeomFile() {
    return m_skinGeom != nullptr && m_skinGeom->getStatus() == FileStatus::FSRejected;
}

template<typename N>
void printTrack(std::ofstream &outFile, const M2Track<N> &track, int animIndex) {
    const auto *transTrackTime = track.timestamps.getElement(animIndex);
    const auto *transTrackVal = track.values.getElement(animIndex);

    if (transTrackTime != nullptr && transTrackVal != nullptr) {
        outFile << "  Times = ";
        printArray(outFile, *transTrackTime);
        outFile << "  Val = ";
        printArray(outFile, *transTrackVal);
    } else {
        outFile << "  Times = []" << std::endl;
        outFile << "  Val = []" << std::endl;
    }
}
template<typename N>
void printArray(std::ofstream &outFile, const M2Array<N> &array) {
    if (array.size > 0) {
        outFile << "[ ";
        for (int k = 0; k < array.size; k++) {
            const auto &elem = *array.getElement(k);
            if constexpr (std::is_same<N, Quat16>::value) {
                auto quat = convertHelper<Quat16, mathfu::quat>(elem);
                auto vec = quat.vector();
                outFile << "[ "<< vec.x << " " << vec.y << " " << vec.z << " " << quat.scalar() << " ]";
            } else if constexpr (std::is_same<N, C3Vector>::value) {
                outFile << "[ " << elem.x << " " << elem.y << " " << elem.z << " ] ";
            } else {
                outFile << elem << " ";
            }
        }
        outFile << "]" << std::endl;
    } else {
        outFile << "[]" << std::endl;
    }
}

void M2Object::dumpBoneAnimations() {
    std::ofstream outFile(std::to_string(this->getModelFileId())+".txt");
    outFile << std::scientific << std::setprecision(6);
    auto const &skelData = m_boneMasterData->getSkelData();
    for (int i = 0; i < skelData->m_sequences->size; i++) {
        const auto &sequence = skelData->m_sequences->getElement(i);
        outFile << "Sequence_" << i << "." << " id=" << sequence->id<< " variation=" << sequence->variationIndex << std::endl;
        for (int j = 0; j < skelData->m_m2CompBones->size; j++) {
            const auto &bone = skelData->m_m2CompBones->getElement(j);

            outFile << "Bone_" << j << std::endl;

            outFile << "Translation" << std::endl;
            printTrack(outFile, bone->translation, i);
            outFile << "Rotation" << std::endl;
            printTrack(outFile, bone->rotation, i);
            outFile << "Scale" << std::endl;
            printTrack(outFile, bone->scaling, i);
        }
        outFile << std::endl << std::endl;
    }
}

void M2Object::doLoadGeom(const HMapSceneBufferCreate &sceneRenderer){
    //0. If loading procedures were already done - exit
    if (this->status->m_loaded) return;

    //1. Check if .m2 files is loaded
    if (m_m2Geom == nullptr) return;
    if (m_m2Geom->getStatus() != FileStatus::FSLoaded) return;


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
    if (m_skinGeom->getStatus() != FileStatus::FSLoaded) return;

    if (m_m2Geom->m_skid > 0) {
        auto skelCache = m_api->cacheStorage->getSkelCache();
        if (m_skelGeom == nullptr) {
            auto skelCache = m_api->cacheStorage->getSkelCache();
            m_skelGeom = skelCache->getFileId(m_m2Geom->m_skid);
            return;
        }
        if (m_skelGeom->getStatus() != FileStatus::FSLoaded ) {
            return;
        }
        if (m_parentSkelGeom == nullptr) {
            if (m_skelGeom->m_skpd != nullptr && m_skelGeom->m_skpd->parent_skel_file_id != 0) {
                m_parentSkelGeom = skelCache->getFileId(m_skelGeom->m_skpd->parent_skel_file_id);
                return;
            }
        }
        if (m_parentSkelGeom != nullptr && m_parentSkelGeom->getStatus() != FileStatus::FSLoaded)
            return;
    }

    //3. Do post load procedures
//    m_skinGeom->fixData(m_m2Geom->getM2Data());
    m_boneMasterData = std::make_shared<CBoneMasterData>(m_m2Geom, m_skelGeom, m_parentSkelGeom);

    this->createVertexBindings(sceneRenderer);
    this->createMeshes(sceneRenderer);

    this->initAnimationManager();
    this->initBoneAnimMatrices();
    this->initTextAnimMatrices();
    this->initSubmeshColors();
    this->initTransparencies();
    this->initLights();
    this->initParticleEmitters(sceneRenderer);
    this->initRibbonEmitters(sceneRenderer);

    m_sceneRendererWeak = sceneRenderer;
    buildCpuBoneSubset();

    this->status->m_loaded = true;
    this->status->m_geomLoaded = true;
    this->status->m_loading = false;

//Dump to bone animations to txt
    {
        // dumpBoneAnimations();
    }

    for ( auto &item : m_postLoadEvents) {
        item(this);
    }
    m_postLoadEvents.clear();

    return;
}

static const mathfu::mat4 particleCoordinatesFix =
    mathfu::mat4(
        0,1,0,0,
        -1,0,0,0,
        0,0,1,0,
        0,0,0,1
    );

void M2Object::buildCpuBoneSubset() {
    std::unordered_set<int> boneSet;

    auto const m2Data = m_m2Geom->getM2Data();

    // Transparent sorting reads bonesMatrices[centerBoneIndex] (M2MeshBufferUpdater::updateSortData)
    auto skinData = m_skinGeom->getSkinData();
    if (skinData != nullptr) {
        for (int i = 0; i < skinData->skinSections.size; i++) {
            boneSet.insert((int)skinData->skinSections.getElement(i)->centerBoneIndex);
        }
    }

    // Lights attach to bones (calcLights)
    for (int i = 0; i < m2Data->lights.size; i++) {
        int bone = m2Data->lights.getElement(i)->bone;
        if (bone >= 0) boneSet.insert(bone);
    }

    // Attachment points (updatePlacementMatrixFromParentAttachment reads the parent's bone)
    for (int i = 0; i < m2Data->attachments.size; i++) {
        boneSet.insert((int)m2Data->attachments.getElement(i)->bone);
    }

    // While particle/ribbon sims run on CPU (or as a fallback), their transforms
    // read the attachment bone matrix
    for (int i = 0; i < m2Data->particle_emitters.size; i++) {
        boneSet.insert((int)m2Data->particle_emitters.getElement(i)->old.bone);
    }
    for (int i = 0; i < m2Data->ribbon_emitters.size; i++) {
        boneSet.insert((int)m2Data->ribbon_emitters.getElement(i)->boneIndex);
    }

    m_cpuBoneSubset.assign(boneSet.begin(), boneSet.end());
}

void M2Object::pushGpuParticleBindFields() {
    if (m_gpuAnimData == nullptr) return;
    for (int i = 0; i < (int)particleEmitters.size(); i++) {
        GpuParticleEmitterBindInfo bindInfo;
        if (m_gpuAnimData->getParticleEmitterBindInfo(i, bindInfo)) {
            particleEmitters[i]->setGpuSimData(bindInfo.stateIndex, bindInfo.capacity);
            particleEmitters[i]->setGpuBindFields(bindInfo);
        }
    }
}

void M2Object::tryInitGpuAnimData() {
    if (m_gpuAnimDataTried) return;
    m_gpuAnimDataTried = true;

    // TEMP diagnostic: report exactly why the GPU anim path is not taken
#ifdef GPU_ANIM_LOGGING
    #define GPU_ANIM_BAIL(reason) do { \
        std::cout << "[GPU M2 anim] skip \"" << m_modelName << "\" (id " << (uintptr_t)this->getObjectId() \
                  << "): " << reason << std::endl; \
        return; \
    } while (0)
#else
    #define GPU_ANIM_BAIL(reason) do {return;} while(0)
#endif

    if (!m_api->getConfig()->useGpuAnimation) GPU_ANIM_BAIL("useGpuAnimation off");
    if (m_animationManager == nullptr || m_boneMasterData == nullptr) GPU_ANIM_BAIL("no animationManager/boneMasterData");
    if (m_modelWideData == nullptr || m_modelWideData->m_bonesData == nullptr) GPU_ANIM_BAIL("no modelWideData/bonesData");
    // Waterfall meshes and legacy dynamic-VAO (>256 bone) batches stay on the CPU path
    if (m_m2Geom->m_wfv3 != nullptr || m_m2Geom->m_wfv1 != nullptr) GPU_ANIM_BAIL("waterfall model");
    if (!dynamicMeshes.empty()) GPU_ANIM_BAIL("has dynamic meshes");

    auto sceneRenderer = m_sceneRendererWeak.lock();
    if (sceneRenderer == nullptr || !sceneRenderer->supportsM2GpuAnimation()) GPU_ANIM_BAIL("renderer does not support GPU anim (not bindless Vulkan / shader missing)");

    // Particle emitters: capture the CPU RNG streams so the GPU sim continues them
    std::vector<M2GpuEmitterSeeds> emitterSeeds(particleEmitters.size());
    std::vector<int32_t> emitterRim(particleEmitters.size());
    for (int i = 0; i < (int)particleEmitters.size(); i++) {
        particleEmitters[i]->getSeedStates(emitterSeeds[i]);
        emitterRim[i] = particleEmitters[i]->getRandomizedTextureIndexMask();
    }

    // Ribbons: the per-ribbon GPU index buffer chunks (created at load)
    std::vector<std::shared_ptr<IBuffer>> ribbonGpuIndexBuffers(ribbonEmitters.size());
    for (int i = 0; i < (int)ribbonEmitters.size(); i++) {
        ribbonGpuIndexBuffers[i] = ribbonEmitters[i]->getGpuIndexBuffer();
    }

    // The static track data is shared per source model (keyed on m_m2Geom): the
    // renderer invokes the pack builder only when no track set exists for the
    // model yet, so instances after the first neither rebuild nor re-upload it.
    m_gpuAnimData = sceneRenderer->createM2GpuAnimData(m_m2Geom.get(),
        [this]() -> M2GpuTrackPack {
            return buildM2GpuTrackPack(*m_boneMasterData, m_m2Geom->getM2Data(),
                                       m_m2Geom->exp2);
        },
        m_modelWideData,
        emitterSeeds, emitterRim,
        ribbonGpuIndexBuffers);
    if (m_gpuAnimData == nullptr) GPU_ANIM_BAIL("createM2GpuAnimData returned null");

    #undef GPU_ANIM_BAIL
    // std::cout << "[GPU M2 anim] enabled for \"" << m_modelName << "\" (id " << (uintptr_t)this->getObjectId()
    //           << "), stateIndex: " << m_gpuAnimData->getStateIndex() << std::endl;

    // Track-data generation baseline for .anim streaming detection (syncGpuAnimTrackData)
    m_gpuTrackDataGeneration = m_gpuAnimData->getTrackSet()->getDataGeneration();

    // Wire up the emitters that run on GPU (others stay on the CPU sim)
    pushGpuParticleBindFields();

    // Wire up the ribbons
    for (int i = 0; i < (int)ribbonEmitters.size(); i++) {
        GpuRibbonEmitterBindInfo bindInfo;
        if (m_gpuAnimData->getRibbonBindInfo(i, bindInfo)) {
            bindInfo.objectId = static_cast<uint32_t>(this->getObjectId());
            ribbonEmitters[i]->setGpuSimData(bindInfo.stateIndex);
            ribbonEmitters[i]->setGpuBindFields(bindInfo);
        }
    }

    // Push the runtime ParticleColor replacement if it was set before GPU-init
    if (particleColorReplacementIsSet) {
        m_gpuAnimData->updateParticleColorReplacements(m_particleColorReplacement, true);
        // validity flipped the colorReplOffset in the bind info — re-push
        pushGpuParticleBindFields();
    }
}

// .anim streaming: lazy sequences become resident when the CPU sequencing gate
// (updateSequencing -> loadLowPriority) finishes loading them; the flag 0x20 flip
// is the completion signal. The track data is shared per model: the first instance
// to notice the growth rebuilds the pack and re-uploads the shared track pools
// (inside syncTrackData, under its mutex); the rest observe the bumped generation.
void M2Object::syncGpuAnimTrackData() {
    auto trackSet = m_gpuAnimData->getTrackSet();

    int32_t loaded = countGpuLoadedSequences(*m_boneMasterData);
    trackSet->syncTrackData(loaded, [this]() -> M2GpuTrackPack {
        return buildM2GpuTrackPack(*m_boneMasterData, m_m2Geom->getM2Data(),
                                   m_m2Geom->exp2);
    });

    // Emitter UBOs carry value-pool offsets (gpuBind fields) that may have moved
    // with the replaced chunks — re-push them whenever the shared data was
    // re-uploaded, no matter which instance of the model triggered it
    uint32_t generation = trackSet->getDataGeneration();
    if (generation != m_gpuTrackDataGeneration) {
        m_gpuTrackDataGeneration = generation;
        pushGpuParticleBindFields();
    }

    // std::cout << "[GPU M2 anim] \"" << m_modelName << "\" (id " << (uintptr_t)this->getObjectId()
    //           << "): streamed track data, loaded sequences: " << loaded << std::endl;
}

//deltaTime = miliseconds
void M2Object::update(double deltaTime, mathfu::vec3 &cameraPos, mathfu::mat4 &viewMat) {
    if (!this->status->m_loaded) return;

    m_postLoadEvents.clear();

    if (m_boolSkybox && m_overrideSkyModelMat) {
        m_placementMatrix.GetColumn(3) = mathfu::vec4(cameraPos, 1.0);
        m_placementMatrixChanged = true;
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

    if (m_api->getConfig()->useGpuAnimation && m_gpuAnimData == nullptr) {
        tryInitGpuAnimData();
    }

    if (m_gpuAnimData != nullptr && m_api->getConfig()->useGpuAnimation) {
        // Stream newly resident .anim sequences into the GPU track pools
        // (the CPU sequencing gate below is what triggers their load)
        syncGpuAnimTrackData();

        // GPU animation path: sequencing stays on CPU, bone/track evaluation runs
        // in the m2Animation compute pass. CPU keeps only the cheap scalar evals and
        // the small bone subset needed for sorting/lights/CPU emitters.
        if (m_animationManager->updateSequencing(animDeltaTime, deltaTime)) {
            m_animationManager->evaluateForGpuPath(m_placementMatrix, modelViewMat,
                                                   this->bonesMatrices, this->textAnimMatrices,
                                                   this->subMeshColors, this->transparencies,
                                                   this->lights, this->particleEmitters,
                                                   this->ribbonEmitters, m_cpuBoneSubset);

            GpuM2AnimState &animState = m_gpuAnimData->getStateForWrite();
            mathfu::mat4 invModelViewMat = modelViewMat.Inverse();
            static_assert(sizeof(animState.modelViewMat) == sizeof(mathfu::mat4));
            memcpy(animState.modelViewMat, &modelViewMat, sizeof(mathfu::mat4));
            memcpy(animState.invModelViewMat, &invModelViewMat, sizeof(mathfu::mat4));
            m_animationManager->fillGpuAnimState(animState);
            m_gpuAnimStateFrame = FrameContext::getCurrentProcessingFrameNumber();
        }
        // else: deferred .anim loading gate — skip evaluation this frame (same as CPU)
    } else {
        this->m_animationManager->update(
            animDeltaTime,
            deltaTime,
            cameraInlocalPos,
            this->m_localUpVector,
            this->m_localRightVector,
            m_placementMatrix,
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
    }

    if (m_animationManager->isNeedUpdateBB()) {
        auto bounds = m_animationManager->getAnimatinonBB();

        // Empty/un-extended animation bounds come back as the classic min=FLT_MAX/max=-FLT_MAX
        // accumulator sentinel. Transforming that through the placement matrix overflows to
        // +/-Infinity, which then poisons WMO group AABB merging and GPU frustum culling permanently.
        bool boundsValid = bounds.extent.min.x <= bounds.extent.max.x &&
                            bounds.extent.min.y <= bounds.extent.max.y &&
                            bounds.extent.min.z <= bounds.extent.max.z &&
                            (mathfu::vec3(bounds.extent.max) - mathfu::vec3(bounds.extent.min)).LengthSquared() > 0.001f;

        if (boundsValid) {
            CAaBox worldAABB = MathHelper::transformAABBWithMat4(m_placementMatrix,
                                                                 mathfu::vec4(mathfu::vec3(bounds.extent.min), 1.0f),
                                                                 mathfu::vec4(mathfu::vec3(bounds.extent.max), 1.0f));
            setAABB(worldAABB);
        }
    }


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

        if (particleEmitters[i]->isGpuSimActive() && isGpuAnimActive()) {
            // GPU-simulated emitter: only the sort distance stays on CPU
            // (the bone it uses is part of m_cpuBoneSubset)
            particleEmitters[i]->updateGpuSortDistance(transformMat, viewMat);
        } else {
            particleEmitters[i]->Update(deltaTime * 0.001 , transformMat, viewMatInv.TranslationVector3D(), nullptr, viewMat);
        }
    }

    this->sortMaterials(modelViewMat);

    //Ribbon Emitters
    mathfu::vec3 nullPos(0,0,0);
    for (int i = 0; i < ribbonEmitters.size(); i++) {
        // GPU-simulated ribbons run in ribbonSimulate.comp.slang (bones read from SSBO)
        if (ribbonEmitters[i]->isGpuSimActive() && isGpuAnimActive()) continue;

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

    M2SkinProfile * skinData = this->m_skinGeom->getSkinData();
    for (int i = 0; i < this->m_meshArray.size(); i++) {
        int currentM2BatchIndex = std::get<1>(this->m_meshArray[i]);

        float finalTransparency = M2MeshBufferUpdater::calcFinalTransparency(*this, currentM2BatchIndex, skinData);
        m_finalTransparencies[i] = finalTransparency;
    }
}

void M2Object::collectLights(std::vector<LocalLight> &pointLights) {
    //Fill lights
    {
        auto const m2Data = m_m2Geom->getM2Data();
        pointLights.reserve(pointLights.size() + lights.size());
        for (int i = 0; i < lights.size(); i++) {
            //1 == point light
            if (m2Data->lights[i]->type != 1) continue;

            auto const &m2Light = lights[i];
            // if (!m2Light.visibility) continue;

            float detlMult = 1.0f;
            if (i < m_m2Geom->detl_count) {
                detlMult = halfToFloat(m_m2Geom->detl[i].diffuseColorMultiplier);
            }

            auto &pointLight =  pointLights.emplace_back();

            auto attenuation_start = m_scale * m2Light.attenuation_start;
            auto attenuation_end = m_scale * m2Light.attenuation_end;

            if (attenuation_end < attenuation_start)
                attenuation_end = attenuation_start + 1.0;

            pointLight.attenuation = mathfu::vec4(
                attenuation_start,
                attenuation_end,
                1.0f/(attenuation_end - attenuation_start),
                0
            );
            pointLight.innerColor = m2Light.diffuse_color * m2Light.diffuse_intensity * detlMult;
            pointLight.outerColor = m2Light.diffuse_color * m2Light.diffuse_intensity * detlMult;
            pointLight.position = m2Light.position;
            pointLight.blendParams = mathfu::vec4(0,0,0,0);
        }
    }
}

void M2Object::fitParticleAndRibbonBuffersToSize(const HMapSceneBufferCreate &sceneRenderer) {
    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());

    for (int i = minParticle; i < maxParticle; i++) {
        particleEmitters[i]->fitBuffersToSize(sceneRenderer);
    }
    for (int i = 0; i < ribbonEmitters.size(); i++) {
        ribbonEmitters[i]->fitBuffersToSize(sceneRenderer);
    }
}

void M2Object::uploadBuffers(mathfu::mat4 &viewMat, const HFrameDependantData &frameDependantData) {
    if (!this->status->m_loaded) return;

//    mathfu::mat4 modelViewMat = viewMat * m_placementMatrix;

    const M2Data * m2File = this->m_m2Geom->getM2Data();
    const M2SkinProfile * skinData = this->m_skinGeom->getSkinData();

    auto const dataIsChanged = m_animationManager->getCombinedChangedData();

    //Update materials`
    if (m_firstUpdate || m_placementMatrixChanged) {
        auto &placementMatrix = m_modelWideData->m_placementMatrix->getObject();
        placementMatrix.uPlacementMat = m_placementMatrix;
        placementMatrix.invPlacementMat = m_placementInvertMatrix;
        m_modelWideData->m_placementMatrix->save();
        m_placementMatrixChanged = false;
    }

    // On the GPU animation path the bone matrices are computed by the m2Animation
    // compute pass straight into the bone SSBO — no CPU upload.
    const bool gpuBonesActive = (m_gpuAnimData != nullptr) && m_api->getConfig()->useGpuAnimation;
    if (gpuBonesActive) {
        // The dirty-flag system does not track CPU-evaluated bones on this path,
        // so force one upload when switching back to the CPU path
        m_forceBoneUploadAfterGpu = true;
    } else if (m_firstUpdate || m_forceBoneUploadAfterGpu || (!bonesMatrices.empty() && dataIsChanged[EAnimDataTypeToInt(EAnimDataType::bonesMatrices)])) {
        auto &bonesData = m_modelWideData->m_bonesData->getObject();
        int interCount = (int) std::min(bonesMatrices.size(), (size_t) MAX_MATRIX_NUM);
        std::copy(bonesMatrices.data(), bonesMatrices.data() + interCount, bonesData.uBoneMatrixes);

        m_modelWideData->m_bonesData->save();
        m_forceBoneUploadAfterGpu = false;
    }
    if (m_firstUpdate || (!subMeshColors.empty() && dataIsChanged[EAnimDataTypeToInt(EAnimDataType::subMeshColors)])) {
        auto &m2Colors = m_modelWideData->m_colors->getObject();
        int m2ColorsCnt = (int) std::min(subMeshColors.size(), (size_t) MAX_M2COLORS_NUM);

        std::copy(subMeshColors.data(), subMeshColors.data() + m2ColorsCnt, m2Colors.colors);
        m_modelWideData->m_colors->save();
    }

    if (m_firstUpdate || (!transparencies.empty() && dataIsChanged[EAnimDataTypeToInt(EAnimDataType::transparencies)])) {
        auto &textureWeights = m_modelWideData->m_textureWeights->getObject();
        int textureWeightsCnt = (int) std::min(transparencies.size(), (size_t) MAX_TEXTURE_WEIGHT_NUM);

        std::copy(transparencies.data(), transparencies.data() + textureWeightsCnt, textureWeights.textureWeight);
        m_modelWideData->m_textureWeights->save();
    }

    if (m_firstUpdate || (!textAnimMatrices.empty() && dataIsChanged[EAnimDataTypeToInt(EAnimDataType::textAnimMatrices)])) {
        auto &textureMatrices = m_modelWideData->m_textureMatrices->getObject();
        int textureMatricesCnt = (int) std::min(textAnimMatrices.size(), (size_t) MAX_TEXTURE_MATRIX_NUM);

        std::copy(textAnimMatrices.data(), textAnimMatrices.data() + textureMatricesCnt, textureMatrices.textureMatrix);
        m_modelWideData->m_textureMatrices->save();
    }

    if (m_firstUpdate || m_modelWideDataChanged || m_setInteriorSunDir)
    {
        auto &modelFragmentData = m_modelWideData->m_modelFragmentData->getObject();

        modelFragmentData.intLight.uInteriorAmbientColorAndInteriorExteriorBlend =
            mathfu::vec4_packed(mathfu::vec4(
                m_interiorAmbientColor,
                m_interiorExteriorBlend
            ));

        modelFragmentData.intLight.uInteriorGroundAmbientColor =
            mathfu::vec4_packed(mathfu::vec4(
                m_interiorAmbientGroundColor,
                .0f
            ));
        modelFragmentData.intLight.uInteriorHorizontAmbientColor =
            mathfu::vec4_packed(mathfu::vec4(
                m_interiorAmbientHorizontColor,
                0.0f
            ));

        modelFragmentData.intLight.uInteriorDirectColor =
            mathfu::vec4_packed(mathfu::vec4(
                m_interiorDirectColor,
                0.0f
            ));
        modelFragmentData.intLight.uPersonalInteriorSunDirAndApplyPersonalSunDir =
            mathfu::vec4_packed(mathfu::vec4(
                (viewMat * mathfu::vec4(m_interiorSunDir, 0.0)).xyz(),
                m_setInteriorSunDir ? 1.0f : 0.f
            ));

        modelFragmentData.modelAlpha = m_alpha;
        modelFragmentData.objectId = static_cast<uint32_t>(this->getObjectId());

        m_modelWideData->m_modelFragmentData->save();
        m_modelWideDataChanged = false;
    }

    //Manually update vertices for dynamics
    updateDynamicMeshes();

    m_firstUpdate = false;
}
void M2Object::uploadGeneratorBuffers(mathfu::mat4 &viewMat, const HFrameDependantData &frameDependantData) {
    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());

    for (int i = minParticle; i < maxParticle; i++) {
        particleEmitters[i]->prepearAndUpdateBuffers(viewMat);
    }

    for (int i = 0; i < ribbonEmitters.size(); i++) {
        ribbonEmitters[i]->updateBuffers();
    }
}

void M2Object::appendGpuParticleStateIndices(std::vector<uint32_t> &out) const {
    if (m_gpuAnimData == nullptr) return;

    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());

    for (int i = minParticle; i < maxParticle; i++) {
        int32_t idx = particleEmitters[i]->getGpuStateIndex();
        if (idx >= 0) {
            out.push_back((uint32_t)idx);
        }
    }
}

void M2Object::appendGpuRibbonStateIndices(std::vector<uint32_t> &out) const {
    if (m_gpuAnimData == nullptr) return;
    if (!m_api->getConfig()->renderRibbons) return;

    for (int i = 0; i < (int)ribbonEmitters.size(); i++) {
        int32_t idx = ribbonEmitters[i]->getGpuStateIndex();
        if (idx >= 0) {
            out.push_back((uint32_t)idx);
        }
    }
}

bool M2Object::isMainDataLoaded() const {
    if (m_m2Geom == nullptr) return false;
    if (m_m2Geom->getStatus() != FileStatus::FSLoaded) return false;

    return true;
}

const bool M2Object::checkFrustumCulling (const mathfu::vec4 &cameraPos, const MathHelper::FrustumCullingData &frustumData) {
    if (!this->status->m_hasAABB) {
        if (!this->isMainDataLoaded()) return false;

        if (m_m2Geom != nullptr) {
            this->createAABB();
        } else {
            return false;
        }
    }

    if (m_alwaysDraw) {
        return true;
    }
    if (m_boolSkybox ) {
        return true;
    }

    const CAaBox &aabb = this->getAABB();

    //1. Check if camera position is inside Bounding Box
    if (
        cameraPos[0] > aabb.min.x && cameraPos[0] < aabb.max.x &&
        cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y &&
        cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
    ) {
        return true;
    }

    //2. Check aabb is inside camera frustum
    bool result = MathHelper::checkFrustum(frustumData, aabb);
    return result;
}

void M2Object::drawBBInternal(const CAaBox &bb, mathfu::vec3 &color, mathfu::mat4 &placementMatrix) {
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
    if (!this->status->m_loaded) return;

    mathfu::mat4 defMat = mathfu::mat4::Identity();
    drawBBInternal(getAABB(), color, defMat);

}

bool M2Object::prepearMaterial(M2MaterialTemplate &materialTemplate, int batchIndex) {
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

    auto textureCount = m2Batch->textureCount;

    if (m_api->getConfig()->useWotlkLogic) {
        std::string vertexShader;
        std::string pixelShader;
        getShaderNames(m2Batch, vertexShader, pixelShader);
        //TODO: this is hack!!!
        if (pixelShader.empty()) {
            materialTemplate.pixelShader = 0;
        } else {
            materialTemplate.pixelShader = pixelShaderTable.at(pixelShader);
        }
    } else {
        //Legion logic
        materialTemplate.pixelShader = getPixelShaderId(m2Batch->textureCount, m2Batch->shader_id);
        materialTemplate.vertexShader = getVertexShaderId(m2Batch->textureCount, m2Batch->shader_id);
    }

    //Hack for Midnight, which has wrongly constructed assets
    if (m_modelFileId > 5000000 && textureCount == 1) {
        auto blpText = getHardCodedTexture(*m2File->texture_lookup_table[m2Batch->textureComboIndex]);
        if (blpText && blpText->getFileDataId() == 5930925) {
            return false;
        }
    }

    for (int j = 0; j < std::min<int>(textureCount, 4); j++) {
        auto m2TextureIndex = *m2File->texture_lookup_table[m2Batch->textureComboIndex + j];
        materialTemplate.textures[j] = getTexture(m2TextureIndex);
    }
//materialTemplate.textures[0]->getTexture()->
    materialTemplate.batchIndex = batchIndex;

    return true;
}

void M2Object::createBoundingBoxMesh(const HMapSceneBufferCreate &sceneRenderer) {

    return;
    //Create bounding box mesh
//    HGShaderPermutation boundingBoxshaderPermutation = m_api->hDevice->getShader("drawBBShader", "drawBBShader", );

    //TODO:
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = false;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum ::GxBlend_Alpha;

    gMeshTemplate meshTemplate(/*m_api->hDevice->getBBVertexBinding()*/ nullptr);

//    meshTemplate.colorMask = 0;
    meshTemplate.start = 0;
    meshTemplate.end = 36;

//    std::shared_ptr<IBufferChunk<bbModelWideBlockVS>> bbBlockVS = m_api->hDevice->createUniformBufferChunk(sizeof(bbModelWideBlockVS));
    std::shared_ptr<IBufferChunk<bbModelWideBlockVS>> bbBlockVS = nullptr;

    auto l_m2Geom = m_m2Geom;

    //Bunding box material */
    /*
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

    bbModelWideBlockVS &blockVS = data;
    blockVS.uPlacementMat = m_placementMatrix;
    blockVS.uBBScale = mathfu::vec4_packed(mathfu::vec4(scale, 0.0));
    blockVS.uBBCenter = mathfu::vec4_packed(mathfu::vec4(center, 0.0));
    blockVS.uColor = mathfu::vec4_packed(mathfu::vec4(0.1f, 0.7f, 0.1f, 0.1f));
    */
    //TODO:
//    boundingBoxMesh = sceneRenderer->createSortableMesh(meshTemplate);
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

HGM2Mesh M2Object::createWaterfallMesh(const HMapSceneBufferCreate &sceneRenderer, const HGVertexBufferBindings &finalBufferBindings) {
    gMeshTemplate meshTemplate(bufferBindings);

    auto skinData = m_skinGeom->getSkinData();
    auto m2Data = m_m2Geom->getM2Data();
    auto wfv3Data = m_m2Geom->m_wfv3 != nullptr ? m_m2Geom->m_wfv3 : m_m2Geom->m_wfv1;

    auto m2Batch = skinData->batches.getElement(0);
    auto skinSection = skinData->skinSections[m2Batch->skinSectionIndex];

    int renderFlagIndex = m2Batch->materialIndex;

#ifdef DEBUG_MESH_NAMES
        meshTemplate.name = "M2 WaterFall, FileDataId = " + std::to_string(m_modelFileId);
#endif

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = false;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

    pipelineTemplate.stencilTestEnable = false;
    pipelineTemplate.stencilWrite = true;
    pipelineTemplate.stencilWriteVal = ObjStencilValues::M2_STENCIL_VAL;

    meshTemplate.start = (skinSection->indexStart + (skinSection->Level << 16)) * 2;
    meshTemplate.end = skinSection->indexCount;

    M2WaterfallMaterialTemplate m2WaterfallMaterialTemplate;

    m2WaterfallMaterialTemplate.textures[0] = getTexture(0); //mask
    m2WaterfallMaterialTemplate.textures[1] = getTexture(1); //whiteWater
    m2WaterfallMaterialTemplate.textures[2] = getTexture(2); //noise
    m2WaterfallMaterialTemplate.textures[3] = getTexture(3); //bumpTexture
    m2WaterfallMaterialTemplate.textures[4] = getTexture(4); //normalTex

    auto waterfallMaterial = sceneRenderer->createM2WaterfallMaterial(m_modelWideData, pipelineTemplate, m2WaterfallMaterialTemplate);

    {
        auto &waterfallCommon = waterfallMaterial->m_waterfallCommon->getObject();
        waterfallCommon.bumpScale = wfv3Data->bumpScale;

        std::array<int, 2> textureMatrixIndexes = {-1, -1};
        M2MeshBufferUpdater::getTextureMatrixIndexes(*this, 0, 0, m2Data, skinData, textureMatrixIndexes);
        waterfallCommon.textureMatIndex1 = textureMatrixIndexes[0];
        waterfallCommon.textureMatIndex2 = textureMatrixIndexes[1];

        waterfallCommon.baseColor = mathfu::vec4(
            wfv3Data->basecolor.a / 255.0f,
            wfv3Data->basecolor.r / 255.0f,
            wfv3Data->basecolor.g / 255.0f,
            wfv3Data->basecolor.b / 255.0f);

        waterfallCommon.values0.x = wfv3Data->values0_x;
        waterfallCommon.values0.y = wfv3Data->values0_y;
        waterfallCommon.values0.z = wfv3Data->values0_z;
        waterfallCommon.values0.w = wfv3Data->values0_w;

        waterfallCommon.values1.x = wfv3Data->value1_x;
        waterfallCommon.values1.y = wfv3Data->value1_y;
        waterfallCommon.values1.w = wfv3Data->value1_w;

        waterfallCommon.m_values3.x = wfv3Data->value3_x;
        waterfallCommon.m_values3.y = wfv3Data->value3_y;

        waterfallCommon.m_values2.x = wfv3Data->flags & 2;
        waterfallCommon.m_values2.y = wfv3Data->flags & 1;

        waterfallCommon.m_values2.w = wfv3Data->value2_w;
        waterfallCommon.m_values3.w = wfv3Data->values3_w;
        waterfallCommon.m_values4.y = wfv3Data->values4_y;

        waterfallCommon.values1.z =   wfv_convert(waterfallCommon.values1.y, (int16_t)((uint64_t)this));
        waterfallCommon.m_values2.z = wfv_convert(waterfallCommon.m_values2.w, (int16_t)((uint64_t)this));
        waterfallCommon.m_values3.z = wfv_convert(wfv3Data->values3_z, (int16_t)((uint64_t)this));

        waterfallMaterial->m_waterfallCommon->save();
    }

    //Make mesh
    auto hmesh = sceneRenderer->createM2WaterfallMesh(meshTemplate, waterfallMaterial, m2Batch->materialLayer, m2Batch->priorityPlane);

    return hmesh;
}

bool isProjectiveTexture(M2Batch * batch, int version) {
    return (batch->flags & 0x4) > 0 || (version > 273 && batch->flags2 & 0x2) > 0; // something is wrong with these flags2
}

void M2Object::createMeshes(const HMapSceneBufferCreate &sceneRenderer) {
    ZoneScoped;

    /* 1. Free previous subMeshArray */
    this->m_meshArray.clear();
    this->m_meshForcedTranspArray.clear();
    this->m_materialArray.clear();

    createBoundingBoxMesh(sceneRenderer);

    if (bufferBindings == nullptr)
        return;

    M2SkinProfile* skinProfile = this->m_skinGeom->getSkinData();
    auto m_m2Data = m_m2Geom->getM2Data();

    /* 2. Fill the materialArray */
    std::vector<int> batchesRequiringDynamicVao = {};
    const auto &batches = m_skinGeom->getSkinData()->batches;

    m_materialArray.resize(std::max<int32_t>(batches.size, 0));
    m_forcedTranspMaterialArray.resize(std::max<int32_t>(batches.size, 0));
    m_projectiveMaterialArray.resize(std::max<int32_t>(batches.size, 0));

    if (m_m2Geom->m_wfv3 == nullptr && m_m2Geom->m_wfv1 == nullptr) {
        int m2Version = m_m2Geom->m_m2Data->version;

        //Create materials
        for (int batchIndex = 0; batchIndex < batches.size; batchIndex++) {
            auto m2Batch = batches[batchIndex];
            EGxBlendEnum mainBlendMode = EGxBlendEnum::GxBlend_Opaque;
            const EGxBlendEnum forcedTranspBlend = EGxBlendEnum::GxBlend_Alpha;

            this->m_materialArray[batchIndex] = createM2Material(sceneRenderer, batchIndex, mainBlendMode, false);
            this->m_forcedTranspMaterialArray[batchIndex] = createM2Material(sceneRenderer, batchIndex, forcedTranspBlend, true);

            if (isProjectiveTexture(m2Batch, m2Version)) {
                this->m_projectiveMaterialArray[batchIndex] = createM2ProjectiveMaterial(sceneRenderer, batchIndex);
            }
        }
        //Create meshes
        for (int batchIndex = 0; batchIndex < batches.size; batchIndex++) {
            auto m2Batch = batches[batchIndex];
            auto skinSection = skinProfile->skinSections[m2Batch->skinSectionIndex];
            if (!checkifBonesAreInRange(skinProfile, skinSection)) {
                batchesRequiringDynamicVao.push_back(batchIndex);
                continue;
            }

            if (isProjectiveTexture(m2Batch, m2Version)) {
                auto mesh = createProjectiveMesh(sceneRenderer, m_projectiveMaterialArray[batchIndex], skinSection, m2Batch);
                this->m_meshProjectiveArray.emplace_back(mesh, batchIndex);

                continue;
            }

            if (m_materialArray[batchIndex] == nullptr)
                continue;

            HGM2Mesh hMesh = createSingleMesh(sceneRenderer,  0, bufferBindings, m_materialArray[batchIndex], skinSection, m2Batch);

            if (hMesh == nullptr)
                continue;

            this->m_meshArray.emplace_back(hMesh, batchIndex);

            if (getBlendMode(batchIndex) == EGxBlendEnum::GxBlend_Opaque || getBlendMode(batchIndex) == EGxBlendEnum::GxBlend_AlphaKey) {
                EGxBlendEnum blendMode = EGxBlendEnum::GxBlend_Alpha;
                HGM2Mesh hMeshTrans = createSingleMesh(sceneRenderer,  0, bufferBindings, m_forcedTranspMaterialArray[batchIndex], skinSection, m2Batch);

                this->m_meshForcedTranspArray.emplace_back(hMeshTrans, batchIndex);
            } else {
                m_meshForcedTranspArray.emplace_back(nullptr, batchIndex);
            }
        }

        // Create meshes requiring dynamic
        for (int j = 0; j < batchesRequiringDynamicVao.size(); j++) {
            int batchIndex = batchesRequiringDynamicVao[j];
            auto m2Batch = skinProfile->batches[batchIndex];
            auto skinSection = skinProfile->skinSections[m2Batch->skinSectionIndex];

            std::array<HGVertexBufferDynamic, IDevice::MAX_FRAMES_IN_FLIGHT> dynVBOs;
            auto dynVaos = m_m2Geom->createDynamicVao(sceneRenderer, dynVBOs, m_skinGeom.get(), skinSection);

            std::array<dynamicVaoMeshFrame, IDevice::MAX_FRAMES_IN_FLIGHT> dynamicMeshData;

            //Try to create mesh
            M2MaterialInst testMaterial;
            EGxBlendEnum blendMode;
            auto testMesh = createSingleMesh(sceneRenderer, 0, dynVaos[0],
                                             m_materialArray[batchIndex], skinSection, m2Batch);
            if (testMesh == nullptr)
                continue;

            for (int k = 0; k < IDevice::MAX_FRAMES_IN_FLIGHT; k++) {
                dynamicMeshData[k].batchIndex = batchIndex;
                dynamicMeshData[k].m_bindings = dynVaos[k];
                dynamicMeshData[k].m_bufferVBO = dynVBOs[k];

                M2MaterialInst material;
                int correction = skinSection->indexStart + (skinSection->Level << 16);
                dynamicMeshData[k].m_mesh = createSingleMesh(sceneRenderer, correction, dynVaos[k],
                                                             m_materialArray[batchIndex], skinSection, m2Batch);
            }

            dynamicMeshes.push_back(dynamicMeshData);
        }
    } else {
//        std::cout << "Waterfall mesh detected " << m_modelFileId << std::endl;
        m_meshArray.push_back({createWaterfallMesh(sceneRenderer, bufferBindings), 0});
    }

    constexpr float defaultTranspVal = 1.0f;
    m_finalTransparencies.resize(m_meshArray.size(), defaultTranspVal);
}

EGxBlendEnum M2Object::getBlendMode(int batchIndex) {
    const auto &batches = m_skinGeom->getSkinData()->batches;
    auto const &m2Data = m_m2Geom->getM2Data();

    auto m2Batch = batches[batchIndex];

    int materialIndex = m2Batch->materialIndex;
    auto renderFlag = m2Data->materials[materialIndex];
    return M2BlendingModeToEGxBlendEnum[renderFlag->blending_mode];
}


std::shared_ptr<IM2Material> M2Object::createM2Material(const HMapSceneBufferCreate &sceneRenderer, int batchIndex,
                                                        const EGxBlendEnum blendMode, bool overrideBlend) {
    M2MaterialTemplate materialTemplate;

    auto m_m2Data = m_m2Geom->getM2Data();
    const auto &batches = m_skinGeom->getSkinData()->batches;
    auto m2Batch = batches[batchIndex];

    //Do not create projective material using this function
    int m2Version = m_m2Geom->m_m2Data->version;
    if (isProjectiveTexture(m2Batch, m2Version)) return nullptr;
    if (!prepearMaterial(materialTemplate, batchIndex)) return nullptr;



    int materialIndex = m2Batch->materialIndex;
    auto renderFlag = m_m2Data->materials[materialIndex];

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = !(renderFlag->flags & 0x10);
    pipelineTemplate.depthCulling = !(renderFlag->flags & 0x8);
    pipelineTemplate.backFaceCulling = !(renderFlag->flags & 0x4);
    if (overrideBlend) {
        pipelineTemplate.blendMode = blendMode;
        if (blendMode > EGxBlendEnum::GxBlend_AlphaKey)
            pipelineTemplate.depthWrite = false;
    } else {
        pipelineTemplate.blendMode = M2BlendingModeToEGxBlendEnum[renderFlag->blending_mode];
    }
    pipelineTemplate.stencilTestEnable = false;
    pipelineTemplate.stencilWrite = true;
    pipelineTemplate.stencilWriteVal = ObjStencilValues::M2_STENCIL_VAL;

    auto m2Material = sceneRenderer->createM2Material(m_modelWideData, pipelineTemplate, materialTemplate);

    //Update material
    M2MeshBufferUpdater::updateMaterialData(m2Material, this, m_skinGeom->getSkinData());

    return m2Material;
}

std::shared_ptr<IM2ProjectiveMaterial> M2Object::createM2ProjectiveMaterial(const HMapSceneBufferCreate &sceneRenderer, int batchIndex) {
    M2MaterialTemplate materialTemplate;

    auto m_m2Data = m_m2Geom->getM2Data();
    const auto &batches = m_skinGeom->getSkinData()->batches;
    auto m2Batch = batches[batchIndex];

    //Create only projective material using this function
    int m2Version = m_m2Geom->m_m2Data->version;
    if (!isProjectiveTexture(m2Batch, m2Version)) return nullptr;
    if (!prepearMaterial(materialTemplate, batchIndex)) return nullptr;

    int materialIndex = m2Batch->materialIndex;
    auto renderFlag = m_m2Data->materials[materialIndex];

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = false;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = true;
    pipelineTemplate.triCCW = false;
    pipelineTemplate.blendMode = M2BlendingModeToEGxBlendEnum[renderFlag->blending_mode];

    pipelineTemplate.stencilTestEnable = true;
    pipelineTemplate.stencilWrite = false;
    pipelineTemplate.stencilWriteVal = ObjStencilValues::M2_STENCIL_VAL;

    auto m2ProjectiveMaterial = sceneRenderer->createM2ProjectiveMaterial(m_modelWideData, pipelineTemplate, materialTemplate);

    uint8_t pixelShader =
        (m2Batch->textureCount == 1 && renderFlag->blending_mode == 5) ?
         4  : //Add_Single_Texture
        (m2Batch->textureCount - 1) & 3; // one of [Opaque_Single_Texture, Two_Texture, Three_Texture]

    //Update material
    M2MeshBufferUpdater::updateProjectiveMaterialData(batchIndex, static_cast<uint8_t>(pipelineTemplate.blendMode),
        0, pixelShader,
        m2ProjectiveMaterial, this, m_m2Data, m_skinGeom->getSkinData());


    return m2ProjectiveMaterial;
}

HGM2Mesh
M2Object::createSingleMesh(const HMapSceneBufferCreate &sceneRenderer, int indexStartCorrection,
                           const HGVertexBufferBindings &finalBufferBindings,
                           const std::shared_ptr<IM2Material> &m2Material,
                           const M2SkinSection *skinSection,
                           const M2Batch *m2Batch) {
    gMeshTemplate meshTemplate(finalBufferBindings);
    meshTemplate.meshType = MeshType::eM2Mesh;

#ifdef DEBUG_MESH_NAMES
    meshTemplate.name = std::string("M2,") +
                        " FileDataId = " + std::to_string(m_modelFileId) +
                        " batchFlags = " + std::to_string(m2Batch->flags) +
                        " priorityPlane = " + std::to_string(m2Batch->priorityPlane)
    ;
#endif

    meshTemplate.start = (skinSection->indexStart + (skinSection->Level << 16) - indexStartCorrection) * 2;
    meshTemplate.end = skinSection->indexCount;

    auto m2Mesh = sceneRenderer->createM2Mesh(meshTemplate, m2Material, m2Batch->materialLayer, m2Batch->priorityPlane);

    return m2Mesh;
}

HGM2Mesh
M2Object::createProjectiveMesh(const HMapSceneBufferCreate &sceneRenderer,
                               const std::shared_ptr<IM2ProjectiveMaterial> &m2Material,
                               const M2SkinSection *skinSection,
                               const M2Batch *m2Batch) {
    gMeshTemplate meshTemplate(nullptr);
    meshTemplate.meshType = MeshType::eM2Mesh;
#ifdef DEBUG_MESH_NAMES
    meshTemplate.name = std::string("M2 Projective,") +
                        " FileDataId = " + std::to_string(m_modelFileId) +
                        " batchFlags = " + std::to_string(m2Batch->flags) +
                        " priorityPlane = " + std::to_string(m2Batch->priorityPlane)
    ;
#endif



    meshTemplate.start = (skinSection->indexStart + (skinSection->Level << 16)) * 2;
    meshTemplate.end = skinSection->indexCount;

    auto m2Mesh = sceneRenderer->createM2ProjectiveMesh(meshTemplate, m2Material, m2Batch->materialLayer, m2Batch->priorityPlane);

    return m2Mesh;
}

void M2Object::forEachVisibleMesh(const std::function<void(const HGM2Mesh &mesh)> &visitor) {
    if (!this->status->m_loaded) return;

    auto *config = m_api->getConfig();
    if (!config->renderM2) return;

    M2SkinProfile* skinData = this->m_skinGeom->getSkinData();

    int minBatch = config->m2MinBatch;
    int maxBatch = std::min(config->m2MaxBatch, (const int &) this->m_meshArray.size());
    bool discardInvisible = config->discardInvisibleMeshes;

    for (int i = 0; i < this->m_meshArray.size(); i++) {
        int currentM2BatchIndex = std::get<1>(this->m_meshArray[i]);
        if (currentM2BatchIndex < minBatch || currentM2BatchIndex > maxBatch ) continue;

        float finalTransparency = m_finalTransparencies[i];
        bool meshIsInvisible = finalTransparency < 0.0001;
        if (discardInvisible && meshIsInvisible)
            continue;

        // Borrowed pointer — avoids a shared_ptr copy (atomic refcounts) per mesh
        const HGM2Mesh *mesh = &std::get<0>(this->m_meshArray[i]);
        if (!meshIsInvisible && finalTransparency < 0.999f && i < this->m_meshForcedTranspArray.size() &&
            std::get<0>(this->m_meshForcedTranspArray[i]) != nullptr) {
            mesh = &std::get<0>(this->m_meshForcedTranspArray[i]);
        }

        visitor(*mesh);
    }

    const int frame = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;
    for (auto &dynMesh: dynamicMeshes) {
        auto const &dynMeshFrame = dynMesh[frame];

        int currentM2BatchIndex = dynMeshFrame.batchIndex;
        if (currentM2BatchIndex < minBatch || currentM2BatchIndex > maxBatch ) continue;

        float finalTransparency = M2MeshBufferUpdater::calcFinalTransparency(*this, currentM2BatchIndex, skinData);
        if ((finalTransparency < 0.0001))
            continue;

        visitor(dynMeshFrame.m_mesh);
    }
}

void M2Object::forEachVisibleMeshSorted(const std::function<void(const HGSortableMesh &mesh, bool hasDynamicDrawParams)> &visitor) {
    if (!this->status->m_loaded) return;

    auto *config = m_api->getConfig();
    if (!config->renderM2) return;

    auto &scratch = m_sortedTranspMeshScratch;
    scratch.clear();

    auto collectMesh = [&](const HGSortableMesh &mesh, bool hasDynamicDrawParams) {
        if (mesh->getIsTransparent()) {
            scratch.push_back({mesh, hasDynamicDrawParams});
        } else {
            // Opaque meshes are re-sorted for batching by the renderer — serve in any order
            visitor(mesh, hasDynamicDrawParams);
        }
    };

    // Static and dynamic-VAO meshes
    forEachVisibleMesh([&](const HGM2Mesh &mesh) {
        collectMesh(mesh, false);
    });

    // Particle and ribbon meshes are sorted together with the usual transparent meshes
    forEachParticleEmitter([&](ParticleEmitter *emitter) {
        emitter->forEachMesh([&](const HGParticleMesh &mesh) {
            collectMesh(mesh, true);
        });
    });

    if (config->renderRibbons) {
        forEachRibbonEmitter([&](CRibbonEmitter *ribbonEmitter) {
            ribbonEmitter->forEachMesh([&](const HGParticleMesh &mesh) {
                collectMesh(mesh, true);
            });
        });
    }

    // Intra-M2 transparent sort. The M2 objects themselves are ordered by the
    // renderer using per-object bounding-box distances ("boxes" rendering).
    if (scratch.size() > 1) {
        std::sort(scratch.begin(), scratch.end(),
                  [](const SortedTranspMeshEntry &a, const SortedTranspMeshEntry &b) {
                      return SortMeshes(a.mesh, b.mesh);
                  });
    }

    for (const auto &entry : scratch) {
        visitor(entry.mesh, entry.hasDynamicDrawParams);
    }
}

void M2Object::collectMeshes(COpaqueMeshCollector &opaqueMeshCollector, transp_vec<HGSortableMesh> &transparentMeshes) {
    if (!this->status->m_loaded) return;

    bool isWaterFallMesh = m_m2Geom->m_wfv3 != nullptr && m_m2Geom->m_wfv1 != nullptr;

    forEachVisibleMesh([&](const HGM2Mesh &mesh) {
        if (mesh->getIsTransparent()) {
            transparentMeshes.emplace_back() = mesh;
        } else {
            if (!isWaterFallMesh) {
                opaqueMeshCollector.addM2Mesh(mesh);
            } else {
                opaqueMeshCollector.addMesh(mesh);
            }
        }
    });

    collectProjectiveMeshes(opaqueMeshCollector);

    if (m_api->getConfig()->drawM2BB) {
        transparentMeshes.emplace_back(boundingBoxMesh);
    }
//    std::cout << "Collected meshes at update frame =" << m_api->hDevice->getUpdateFrameNumber() << std::endl;
}

void M2Object::collectProjectiveMeshes(COpaqueMeshCollector &opaqueMeshCollector) {
    if (!this->status->m_loaded) return;
    if (!m_api->getConfig()->renderM2 || !m_api->getConfig()->renderM2Decals) return;

    M2SkinProfile* skinData = this->m_skinGeom->getSkinData();

    for (int i = 0; i < this->m_meshProjectiveArray.size(); i++) {
        int currentM2BatchIndex = std::get<1>(this->m_meshProjectiveArray[i]);

        float finalTransparency = M2MeshBufferUpdater::calcFinalTransparency(*this, currentM2BatchIndex, skinData);
        bool meshIsInvisible = (finalTransparency < 0.0001);

        if (m_api->getConfig()->discardInvisibleMeshes && meshIsInvisible)
            continue;

        HGM2Mesh mesh = std::get<0>(this->m_meshProjectiveArray[i]);
        opaqueMeshCollector.addProjectiveMesh(mesh);
    }
}

void M2Object::forEachRibbonEmitter(const std::function<void(CRibbonEmitter *emitter)> &visitor) {
    if (!this->status->m_loaded) return;

    for (auto &ribbonEmitter : ribbonEmitters) {
        visitor(ribbonEmitter.get());
    }
}

void M2Object::initAnimationManager() {
    ZoneScoped;
    this->m_animationManager = std::make_unique<AnimationManager>(m_api, m_boneMasterData, m_m2Geom->exp2 != nullptr);
}

void M2Object::initBoneAnimMatrices() {
    ZoneScoped;
    auto &bones = *m_boneMasterData->getSkelData()->m_m2CompBones;
    this->bonesMatrices = std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>>(bones.size, mathfu::mat4::Identity());;
}
void M2Object::initTextAnimMatrices() {
    ZoneScoped;
    textAnimMatrices = std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>>(m_m2Geom->getM2Data()->texture_transforms.size, mathfu::mat4::Identity());;
}

void M2Object::initSubmeshColors() {
    ZoneScoped;
    subMeshColors = std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>>(m_m2Geom->getM2Data()->colors.size);

}
void M2Object::initTransparencies() {
    transparencies = std::vector<float>(m_m2Geom->getM2Data()->texture_weights.size);
}

void M2Object::initLights() {
    ZoneScoped;
    lights = std::vector<M2LightResult>(m_m2Geom->getM2Data()->lights.size);
}
void M2Object::initParticleEmitters(const HMapSceneBufferCreate &sceneRenderer) {
    ZoneScoped;

    particleEmitters.clear();
    particleEmitters.reserve(m_m2Geom->getM2Data()->particle_emitters.size);
    for (int i = 0; i < m_m2Geom->getM2Data()->particle_emitters.size; i++) {
        int txacVal = 0;
        if (m_m2Geom->txacMParticle.size() > 0) {
            txacVal = m_m2Geom->txacMParticle[i].value;
        }

        Exp2Record *exp2 = nullptr;
        if (m_m2Geom->exp2 != nullptr) {
            exp2 = m_m2Geom->exp2->content.getElement(i);
        }
        auto emitter = std::make_unique<ParticleEmitter>(m_api, sceneRenderer,m_m2Geom->getM2Data()->particle_emitters.getElement(i), exp2, this, m_m2Geom, txacVal);
        if (m_m2Geom->exp2 != nullptr && emitter->getGenerator() != nullptr) {
            auto exp2Rec = m_m2Geom->exp2->content.getElement(i);
            emitter->getGenerator()->getAniProp()->zSource = exp2Rec->zSource;
        }

        particleEmitters.push_back(std::move(emitter));
    }
}

void M2Object::initRibbonEmitters(const HMapSceneBufferCreate &sceneRenderer) {
    ZoneScoped;

    ribbonEmitters = std::vector<std::unique_ptr<CRibbonEmitter>>();
//    ribbonEmitters.reserve(m_m2Geom->getM2Data()->ribbon_emitters.size);
    auto m2Data = m_m2Geom->getM2Data();
    for (int i = 0; i < m2Data->ribbon_emitters.size; i++) {
        M2Ribbon *m2Ribbon = m2Data->ribbon_emitters.getElement(i);

        std::vector<M2Material> materials(m2Ribbon->materialIndices.size);
        std::vector<int> textureIndicies(m2Ribbon->textureIndices.size);
        for (size_t j = 0; j < materials.size(); j++) {
            materials[j] = *m2Data->materials[*m2Ribbon->materialIndices[j]];
        }

        for (size_t j = 0; j < textureIndicies.size(); j++) {
            textureIndicies[j] = *m2Ribbon->textureIndices[j];
        }

        int textureTransformLookup = (m2Data->global_flags.flag_unk_0x20000 != 0) ? m2Ribbon->textureTransformLookupIndex : -1;

        auto emitter = std::make_unique<CRibbonEmitter>(m_api, sceneRenderer, m_modelWideData,
                                          this, materials, textureIndicies, textureTransformLookup);

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
        // Pull-model GPU mesh set (created only when the renderer supports GPU ribbons)
        emitter->createGpuMeshes(sceneRenderer);

        ribbonEmitters.push_back(std::move(emitter));
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
    if (!status->m_loaded) return;

    m_animationManager->setAnimationId(animationId, false);
}
void M2Object::resetCurrentAnimation() {
    if (!status->m_loaded) return;

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
void M2Object::getMeshIds(std::vector<int> &meshIdList) {
//    this->m_skin
    std::unordered_set<int> meshIdSet;

    auto skinData = m_skinGeom->getSkinData();
    M2Array<M2Batch>& batches = skinData->batches;


    for (int i = 0; i < batches.size; i++) {
        auto m2Batch = batches[i];
        auto skinSection = skinData->skinSections[m2Batch->skinSectionIndex];

        meshIdSet.insert(skinSection->skinSectionId);
    }

    meshIdList = std::vector<int>(meshIdSet.begin(), meshIdSet.end());
}
mathfu::mat4 M2Object::getTextureTransformByLookup(int textureTrasformlookup) {
    if (textureTrasformlookup < this->m_m2Geom->getM2Data()->texture_transforms_lookup_table.size) {
        auto textureTransformIndex = *this->m_m2Geom->getM2Data()->texture_transforms_lookup_table.getElement(textureTrasformlookup);
        if (textureTransformIndex >= 0 && textureTransformIndex < this->textAnimMatrices.size()) {
            return this->textAnimMatrices[textureTransformIndex];
        }
    }

    return mathfu::mat4::Identity();
}
int32_t M2Object::getTextureTransformIndexByLookup(int textureTrasformlookup) {
    if (textureTrasformlookup < this->m_m2Geom->getM2Data()->texture_transforms_lookup_table.size) {
        auto textureTransformIndex = *this->m_m2Geom->getM2Data()->texture_transforms_lookup_table.getElement(textureTrasformlookup);
        if (textureTransformIndex >= 0 && textureTransformIndex < this->textAnimMatrices.size()) {
            return textureTransformIndex;
        }
    }

    return -1;
}

void M2Object::forEachParticleEmitter(const std::function<void(ParticleEmitter *emitter)> &visitor) {
    int minParticle = m_api->getConfig()->minParticle;
    int maxParticle = std::min(m_api->getConfig()->maxParticle, (const int &) particleEmitters.size());

    for (int i = minParticle; i < maxParticle; i++) {
        //Respect PGD1 chunk
        if (m_m2Geom->particleGeosetData != nullptr) {
            auto geoset = *m_m2Geom->particleGeosetData->pgd.getElement(i);
            auto meshGroup = (geoset / 100);
            if ((meshGroup < this->m_meshIds.size()) && (geoset > 0) &&
                (m_meshIds[meshGroup] != (geoset % 100))) {
                continue;
            }
        }

        visitor(particleEmitters[i].get());
    }
}

void M2Object::drawParticles(COpaqueMeshCollector &opaqueMeshCollector, transp_vec<HGSortableMesh> &transparentMeshes) {
    if (!this->status->m_loaded) return;

    forEachParticleEmitter([&](ParticleEmitter *emitter) {
        emitter->collectMeshes(opaqueMeshCollector, transparentMeshes);
    });

    if (m_api->getConfig()->renderRibbons) {
        for (int i = 0; i < ribbonEmitters.size(); i++) {
            ribbonEmitters[i]->collectMeshes(opaqueMeshCollector, transparentMeshes);
        }
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

HGSamplableTexture M2Object::getTexture(int textureInd) {
    M2Texture* textureDefinition = m_m2Geom->getM2Data()->textures.getElement(textureInd);

    HBlpTexture blpData = getBlpTextureData(textureInd);

    if (blpData == nullptr)
        return nullptr;

    HGSamplableTexture hgTexture = m_api->hDevice->createBlpTexture(
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

void M2Object::createVertexBindings(const HMapSceneBufferCreate &sceneRenderer) {
    ZoneScoped;
    HGDevice device = m_api->hDevice;

    //2. Create buffer binding and fill it
    bufferBindings = m_m2Geom->getVAO(sceneRenderer, m_skinGeom.get());

    //3. Create model wide uniform buffer
    m_modelWideData = sceneRenderer->createM2ModelMat(
        m_boneMasterData->getSkelData()->m_m2CompBones->size,
        m_m2Geom->m_m2Data->colors.size,
        m_m2Geom->m_m2Data->texture_weights.size,
        m_m2Geom->m_m2Data->texture_transforms.size,
        static_cast<uint32_t>(this->getObjectId())
    );
}

void M2Object::updateDynamicMeshes() {
    if (dynamicMeshes.empty()) return;

    auto rootMatInverse = bonesMatrices[0].Inverse();
    auto frameNum = FrameContext::getCurrentProcessingFrameNumber() % IDevice::MAX_FRAMES_IN_FLIGHT;


    for (auto &dynamicMesh: dynamicMeshes) {
        auto &dynMeshData = dynamicMesh[frameNum];

        M2SkinProfile* skinProfile = this->m_skinGeom->getSkinData();
        auto m2Data = m_m2Geom->getM2Data();

        auto m2Batch = skinProfile->batches[dynMeshData.batchIndex];
        auto skinSection = skinProfile->skinSections[m2Batch->skinSectionIndex];

        M2Vertex *overrideVertexes = (M2Vertex *)dynMeshData.m_bufferVBO->getPointer();

        for (int vertIndex = skinSection->vertexStart;
             vertIndex < (skinSection->vertexStart + skinSection->vertexCount); ++vertIndex) {
            auto &overrideVert = overrideVertexes[vertIndex - skinSection->vertexStart];
            auto const &originalVert = *m2Data->vertices[vertIndex];

            mathfu::mat4 matrix = mathfu::mat4::Identity();
            if (originalVert.bone_indices[0] > 0) {
                matrix = bonesMatrices[originalVert.bone_indices[0]] * originalVert.bone_weights[0];
            }

            for (int i = 1; i < MAX_BONES_PER_VERTEX; i++) {
                if (overrideVert.bone_indices[i] > 0) {
                    matrix += (bonesMatrices[originalVert.bone_indices[i]] * originalVert.bone_weights[i]);
                }
            }

            overrideVert = originalVert;
            overrideVert.pos =
                mathfu::vec3_packed(rootMatInverse * matrix * mathfu::vec4(mathfu::vec3(originalVert.pos), 1.0).xyz());
            overrideVert.bone_indices[0] = 0;
            overrideVert.bone_indices[1] = 0;
            overrideVert.bone_indices[2] = 0;
            overrideVert.bone_indices[3] = 0;
        }
        dynMeshData.m_bufferVBO->save(skinSection->vertexCount*sizeof(M2Vertex));

        //TODO:
//        std::cout << "Saved " << skinSection->vertexCount << " vertices " << "at update frame =" << frameNum << std::endl;
    }
}
void M2Object::setReplaceTextures(const HMapSceneBufferCreate &sceneRenderer, const std::vector<HBlpTexture> &replaceTextures) {
    m_replaceTextures = replaceTextures;

    if (status->m_loaded) {
        createMeshes(sceneRenderer); // recreate meshes
    }
}
void M2Object::setMeshIds(const HMapSceneBufferCreate &sceneRenderer, const std::vector<uint8_t> &meshIds) {
    m_meshIds = meshIds;

    if (status->m_loaded) {
        createMeshes(sceneRenderer); // recreate meshes
    }
}

void M2Object::setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {
    m_particleColorReplacement = particleColorReplacement;
    particleColorReplacementIsSet = true;
    std::cout << "particleColorReplacementIsSet = " << particleColorReplacementIsSet << std::endl;
    if (m_gpuAnimData != nullptr) {
        m_gpuAnimData->updateParticleColorReplacements(m_particleColorReplacement, true);
        pushGpuParticleBindFields();
    }
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
    if (m_gpuAnimData != nullptr) {
        m_gpuAnimData->updateParticleColorReplacements(m_particleColorReplacement, false);
        pushGpuParticleBindFields();
    }
}

int M2Object::getCurrentAnimationIndex() {
    return m_animationManager->getCurrentAnimationIndex();
}

std::shared_ptr<EntityFactory<10000, M2ObjId, M2Object, CAaBox, M2LoadedStatus>> m2Factory =
    std::make_shared<EntityFactory<10000, M2ObjId, M2Object, CAaBox, M2LoadedStatus>>();