//
// Created by Deamon on 9/5/2017.
//

#ifndef WOWVIEWERLIB_IWMOAPI_H
#define WOWVIEWERLIB_IWMOAPI_H

#include <functional>
#include "../engineClassList.h"
#include "m2/m2Object.h"
#include "lights/CWmoNewLight.h"

struct PortalInfo_t {
    std::vector<mathfu::vec3> sortedVericles;
    CAaBox aaBox;
};

struct WmoGroupResult {
    M2Range topBottom;
    int groupIndex;
    int WMOGroupID;
    std::vector<int> bspLeafList;
    int nodeId;
};

/*
//0
MapObjDiffuse {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjDiffuse);
}

//1
MapObjSpecular {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjSpecular);
}

//2
MapObjMetal {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjMetal);
}

//3
MapObjEnv {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnv);
}

//4
MapObjOpaque {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjOpaque);
}

//5
Effect(MapObjEnvMetal {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnvMetal);
}

//6
Effect(MapObjComposite) //aka MapObjTwoLayerDiffuse
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjComposite); //aka MapObjTwoLayerDiffuse
}

//7
Effect(MapObjTwoLayerEnvMetal)
{
    VertexShader(MapObjDiffuse_Comp_Refl);
    PixelShader(MapObjTwoLayerEnvMetal);
}

//8
Effect(TwoLayerTerrain)
{
    VertexShader(MapObjDiffuse_Comp_Terrain);
    PixelShader(MapObjTwoLayerTerrain);
}

//9
Effect(MapObjDiffuseEmissive)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjDiffuseEmissive);
}

//10
Effect(waterWindow)
{
e    //unk
}

//11
Effect(MapObjMaskedEnvMetal)
{
    VertexShader(MapObjDiffuse_T1_Env_T2);
    PixelShader(MapObjMaskedEnvMetal);
}

//12
Effect(MapObjEnvMetalEmissive)
{
    VertexShader(MapObjDiffuse_T1_Env_T2);
    PixelShader(MapObjEnvMetalEmissive);
}

//13
Effect(TwoLayerDiffuseOpaque)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjTwoLayerDiffuseOpaque);
}

//14
Effect(submarineWindow)
{
    //unk
}

//15
Effect(TwoLayerDiffuseEmissive)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjTwoLayerDiffuseEmissive);
}

//16
Effect(MapObjDiffuseTerrain)
{
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjDiffuse);
}

*/

enum class WmoVertexShader : int {
    None = -1,
    MapObjDiffuse_T1 = 0,
    MapObjDiffuse_T1_Refl = 1,
    MapObjDiffuse_T1_Env_T2 = 2,
    MapObjSpecular_T1 = 3,
    MapObjDiffuse_Comp = 4,
    MapObjDiffuse_Comp_Refl = 5,
    MapObjDiffuse_Comp_Terrain = 6,
    MapObjDiffuse_CompAlpha = 7,
    MapObjParallax = 8,

};

enum class WmoPixelShader : int {
    None = -1,
    MapObjDiffuse = 0,
    MapObjSpecular = 1,
    MapObjMetal = 2,
    MapObjEnv = 3,
    MapObjOpaque = 4,
    MapObjEnvMetal = 5,
    MapObjTwoLayerDiffuse = 6, //MapObjComposite
    MapObjTwoLayerEnvMetal = 7,
    MapObjTwoLayerTerrain = 8,
    MapObjDiffuseEmissive = 9,
    MapObjMaskedEnvMetal = 10,
    MapObjEnvMetalEmissive = 11,
    MapObjTwoLayerDiffuseOpaque = 12,
    MapObjTwoLayerDiffuseEmissive = 13,
    MapObjAdditiveMaskedEnvMetal = 14,
    MapObjTwoLayerDiffuseMod2x = 15,
    MapObjTwoLayerDiffuseMod2xNA = 16,
    MapObjTwoLayerDiffuseAlpha = 17,
    MapObjLod = 18,
    MapObjParallax = 19,
    MapObjDFShader = 20
};

inline constexpr const int operator+(WmoPixelShader const val) { return static_cast<const int>(val); };

inline constexpr const int operator+(WmoVertexShader const val) { return static_cast<const int>(val); };

const int MAX_WMO_SHADERS = 24;
static const struct {
    int vertexShader;
    int pixelShader;
} wmoMaterialShader[MAX_WMO_SHADERS] = {
    //MapObjDiffuse = 0
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjDiffuse,
    },
    //MapObjSpecular = 1
    {
        +WmoVertexShader::MapObjSpecular_T1,
        +WmoPixelShader::MapObjSpecular,
    },
    //MapObjMetal = 2
    {
        +WmoVertexShader::MapObjSpecular_T1,
        +WmoPixelShader::MapObjMetal,
    },
    //MapObjEnv = 3
    {
        +WmoVertexShader::MapObjDiffuse_T1_Refl,
        +WmoPixelShader::MapObjEnv,
    },
    //MapObjOpaque = 4
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjOpaque,
    },
    //MapObjEnvMetal = 5
    {
        +WmoVertexShader::MapObjDiffuse_T1_Refl,
        +WmoPixelShader::MapObjEnvMetal,
    },
    //MapObjTwoLayerDiffuse = 6
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuse,
    },
    //MapObjTwoLayerEnvMetal = 7
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjTwoLayerEnvMetal,
    },
    //TwoLayerTerrain = 8
    {
        +WmoVertexShader::MapObjDiffuse_Comp_Terrain,
        +WmoPixelShader::MapObjTwoLayerTerrain,
    },
    //MapObjDiffuseEmissive = 9
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjDiffuseEmissive,
    },
    //waterWindow = 10
    {
        +WmoVertexShader::None,
        +WmoPixelShader::None,
    },
    //MapObjMaskedEnvMetal = 11
    {
        +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        +WmoPixelShader::MapObjMaskedEnvMetal,
    },
    //MapObjEnvMetalEmissive = 12
    {
        +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        +WmoPixelShader::MapObjEnvMetalEmissive,
    },
    //TwoLayerDiffuseOpaque = 13
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuseOpaque,
    },
    //submarineWindow = 14
    {
        +WmoVertexShader::None,
        +WmoPixelShader::None,
    },
    //TwoLayerDiffuseEmissive = 15
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuseEmissive,
    },
    //MapObjDiffuseTerrain = 16
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjDiffuse,
    },
    //17
    {
        +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        +WmoPixelShader::MapObjAdditiveMaskedEnvMetal,
    },
    //18
    {
        +WmoVertexShader::MapObjDiffuse_CompAlpha,
        +WmoPixelShader::MapObjTwoLayerDiffuseMod2x,
    },
    //19
    {
        +WmoVertexShader::MapObjDiffuse_Comp,
        +WmoPixelShader::MapObjTwoLayerDiffuseMod2xNA,
    },
    //20
    {
        +WmoVertexShader::MapObjDiffuse_CompAlpha,
        +WmoPixelShader::MapObjTwoLayerDiffuseAlpha,
    },
    //21
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjLod,
    },
    //22
    {
        +WmoVertexShader::MapObjParallax,
        +WmoPixelShader::MapObjParallax,
    },
    //23
    {
        +WmoVertexShader::MapObjDiffuse_T1,
        +WmoPixelShader::MapObjDFShader,
    },
};

class IWmoApi {
public:
    virtual std::shared_ptr<M2Object> getDoodad(int index) = 0;
    virtual SMOHeader *getWmoHeader() = 0;
    virtual mathfu::vec3 getAmbientColor() = 0;
    virtual PointerChecker<SMOMaterial> &getMaterials() = 0;
    virtual std::shared_ptr<IWMOMaterial> getMaterialInstance(int index, const HMapSceneBufferCreate &sceneRenderer) = 0;
    virtual bool isLoaded() = 0;
    virtual std::function<void (WmoGroupGeom& wmoGroupGeom)> getAttenFunction() = 0;
    virtual PointerChecker<SMOLight> &getLightArray() = 0;

    virtual std::vector<PortalInfo_t> &getPortalInfos() = 0;
    virtual int getActiveDoodadSet() = 0;
    virtual std::shared_ptr<CWmoNewLight> getNewLight(int index) = 0;


    virtual HGSamplableTexture getTexture(int textureId, bool isSpec) = 0;
    virtual void updateBB() = 0;
    virtual void postWmoGroupObjectLoad(int groupId, int lod) = 0;
    virtual std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> getPlacementBuffer() = 0;
};
#endif //WOWVIEWERLIB_IWMOAPI_H
