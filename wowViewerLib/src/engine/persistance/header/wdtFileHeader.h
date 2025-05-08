//
// Created by deamon on 28.11.17.
//

#ifndef WEBWOWVIEWERCPP_WDTFILEHEADER_H_H
#define WEBWOWVIEWERCPP_WDTFILEHEADER_H_H

#include "commonFileStructs.h"

struct MPHDFlags {
    //0x1
    uint32_t wdt_uses_global_map_obj : 1;
    //0x2
    uint32_t adt_has_mccv : 1;
    //0x4
    uint32_t adt_has_big_alpha : 1;
    //0x8
    uint32_t adt_has_doodadrefs_sorted_by_size_cat : 1;
    //0x10
    uint32_t adt_has_mclv : 1; //adds second color: ADT.MCNK.MCLV
    //0x20
    uint32_t adt_has_upside_down_ground : 1; //Flips the ground display upside down to create a ceiling
    //0x40
    uint32_t unk_0x0040 : 1;
    //0x80
    uint32_t adt_has_height_texturing : 1; //shader = 6. Decides whether to influence alpha maps by _h+MTXP: (without with)
    //0x100
    uint32_t unk_0x0100 : 1; //implicitly sets 0x8000
    //0x200
    uint32_t wdt_has_maid : 1;
    //0x400
    uint32_t unk_0x0400 : 1;
    //0x800
    uint32_t unk_0x0800 : 1;
    //0x1000
    uint32_t unk_0x1000 : 1;
    //0x2000
    uint32_t unk_0x2000 : 1;
    //0x4000
    uint32_t unk_0x4000 : 1;
    //0x4000
    uint32_t unk_0x8000 : 1; //implicitly set for map ids 0, 1, 571, 870, 1116 (continents). Affects the rendering of _lod.adt

};

struct MPHD {
    MPHDFlags flags;
    uint32_t lgtFileDataID;
    uint32_t occFileDataID;
    uint32_t fogsFileDataID;
    uint32_t mpvFileDataID;
    uint32_t texFileDataID;
    uint32_t wdlFileDataID;
    uint32_t pd4FileDataID;
};

struct MAIN {
    uint32_t Flag_HasADT : 1;
    uint32_t Flag_AllWater : 1;
    uint32_t Flag_Loaded : 1;
    uint32_t asyncId;    // only set during runtime.
};

PACK(
struct MapPointLight2
{
/*0x00*/  uint32_t lightIndex;
/*0x04*/  CImVector color;
/*0x08*/  C3Vector position;
/*0x14*/  float attenuationStart;
/*0x18*/  float attenuationEnd;
/*0x1C*/  float intensity;
/*0x20*/  C3Vector rotation;
/*0x2C*/  uint16_t tileX;
/*0x2E*/  uint16_t tileY;
/*0x30*/  int16_t mlta_index;   //Index into MLTA
/*0x32*/  int16_t textureIndex; //Index into MTEX
});

PACK(
struct MapPointLight3
{
/*0x00*/  uint32_t lightIndex;
/*0x04*/  CImVector color;
/*0x08*/  C3Vector position;
/*0x14*/  float attenuationStart;
/*0x18*/  float attenuationEnd;
/*0x1C*/  float intensity;
/*0x20*/  C3Vector rotation;        //Should be rotation, but rotation doesn't make sense for point light. Probably unused?
/*0x2C*/  uint16_t tileX;
/*0x2E*/  uint16_t tileY;
/*0x30*/  int16_t mlta_index;   //Index into MLTA
/*0x32*/  int16_t textureIndex; //Index into MTEX
/*0x34*/  uint16_t flags;
/*0x36*/  uint16_t scale;        //Some packed value
});

PACK(
struct MapSpotLight
{
    uint32_t id;
    CArgb color;
    C3Vector position;
    float attenuationStart; // When to start the attenuation of the light, must be <= attenuationEnd or glitches
    float attenuationEnd;
    float intensity;
    C3Vector rotation; // radians
    float falloff;
    float innerAngle;
    float outerAngle; // radians
    uint16_t tileX;
    uint16_t tileY;
    uint16_t mlta_index; //Index into MTLA
    uint16_t textureIndex; //Index into MTEX
}) ;



#endif //WEBWOWVIEWERCPP_WDTFILEHEADER_H_H
