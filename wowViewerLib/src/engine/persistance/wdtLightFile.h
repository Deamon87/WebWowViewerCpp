//
// Created by Deamon on 3/31/2024.
//

#ifndef AWEBWOWVIEWERCPP_WDTLIGHTFILE_H
#define AWEBWOWVIEWERCPP_WDTLIGHTFILE_H

#include "../../include/sharedFile.h"
#include "PersistentFile.h"
#include "helper/ChunkFileReader.h"

class WdtLightFile : public PersistentFile {
public:
    WdtLightFile(std::string fileName){};
    WdtLightFile(int fileDataId){};
    ~WdtLightFile() {
//		std::cout << "destructor for WdtLightFile was called" << std::endl;
    }

    void process(HFileContent wdtFile, const std::string &fileName) override;
public:
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
/*0x36*/  uint16_t unk1;        //Some packed value
    });

    PACK(
    struct MapSpotLight
    {
        uint32_t id;
        CArgb color;
        C3Vector position;
        float rangeAttenuationStart;// When to start the attenuation of the light, must be <= rangeAttenuationEnd or glitches
        float rangeAttenuationEnd;
        float intensity;
        C3Vector rotation; // radians
        float falloffExponent;
        float innerRadius;
        float outerRadius; // radians
        uint16_t tile_x;
        uint16_t tile_y;
        int16_t unk[2]; // -1 mostly, may be an MLTA ID though.
    }) ;

    PACK(
    struct MapLightTextureAttenuation{
        float flickerIntensity;
        float flickerSpeed;
        int flickerMode;                // 0 = off, 1 = sine curve, 2 = noise curve, 3 = noise step curve
    });

    PointerChecker<MapPointLight3> mapPointLights3 = (mapPointLights3Len);
    int mapPointLights3Len = 0;

    PointerChecker<MapSpotLight> mapSpotLights3 = (mapSpotLightLen);
    int mapSpotLightLen = 0;

    PointerChecker<MapLightTextureAttenuation> mapTextureLightAttenuation = (mapTextureLightAttenuationLen);
    int mapTextureLightAttenuationLen = 0;

    std::vector<uint32_t> textureFileDataIds;

private:
    HFileContent m_wdtLightFile;
    static chunkDef<WdtLightFile> wdtLightFileTable;
};


#endif //AWEBWOWVIEWERCPP_WDTLIGHTFILE_H
