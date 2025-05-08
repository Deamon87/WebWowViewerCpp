//
// Created by Deamon on 3/31/2024.
//

#ifndef AWEBWOWVIEWERCPP_WDTLIGHTFILE_H
#define AWEBWOWVIEWERCPP_WDTLIGHTFILE_H

#include "../../include/sharedFile.h"
#include "PersistentFile.h"
#include "helper/ChunkFileReader.h"
#include "header/wdtFileHeader.h"

class WdtLightFile : public PersistentFile {
public:
    WdtLightFile(const std::string &fileName) : PersistentFile(fileName){};
    WdtLightFile(int fileDataId) : PersistentFile(fileDataId){};
    ~WdtLightFile() {
//		std::cout << "destructor for WdtLightFile was called" << std::endl;
    }

    void process(HFileContent wdtFile) override;
public:
    PointerChecker<MapPointLight2> mapPointLights2 = (mapPointLights2Len);
    int mapPointLights2Len = 0;

    PointerChecker<MapPointLight3> mapPointLights3 = (mapPointLights3Len);
    int mapPointLights3Len = 0;

    PointerChecker<MapSpotLight> mapSpotLights = (mapSpotLightLen);
    int mapSpotLightLen = 0;

    PointerChecker<MapLightTextureAnimation> mapTextureLightAttenuation = (mapTextureLightAttenuationLen);
    int mapTextureLightAttenuationLen = 0;

    std::vector<uint32_t> textureFileDataIds;

private:
    HFileContent m_wdtLightFile;
    static chunkDef<WdtLightFile> wdtLightFileTable;
};


#endif //AWEBWOWVIEWERCPP_WDTLIGHTFILE_H
