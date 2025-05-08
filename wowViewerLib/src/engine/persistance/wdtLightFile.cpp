//
// Created by Deamon on 3/31/2024.
//

#include "wdtLightFile.h"
chunkDef<WdtLightFile> WdtLightFile::wdtLightFileTable = {
    [](WdtLightFile &file, ChunkData &chunkData) {},
    {
        {
            'MPL3',
            {
                [](WdtLightFile &file, ChunkData &chunkData) {
                    debuglog("Entered MPL3");
                    file.mapPointLights3Len = (chunkData.chunkLen / sizeof(MapPointLight3));
                    chunkData.readValues(file.mapPointLights3, file.mapPointLights3Len);
                }
            }
        },
        {
            'MPL2',
            {
                [](WdtLightFile &file, ChunkData &chunkData) {
                    debuglog("Entered MPL2");
                    file.mapPointLights2Len = (chunkData.chunkLen / sizeof(MapPointLight2));
                    chunkData.readValues(file.mapPointLights2, file.mapPointLights2Len);
                }
            }
        },
        {
            'MSLT',
            {
                [](WdtLightFile &file, ChunkData &chunkData) {
                    debuglog("Entered MSLT");
                    file.mapSpotLightLen = (chunkData.chunkLen / sizeof(MapSpotLight));
                    chunkData.readValues(file.mapSpotLights, file.mapSpotLightLen);
                }
            }
        },
        {
            'MLTA',
            {
                [](WdtLightFile &file, ChunkData &chunkData) {
                    debuglog("Entered MLTA");
                    file.mapTextureLightAttenuationLen = (chunkData.chunkLen / sizeof(MapLightTextureAnimation));
                    chunkData.readValues(file.mapTextureLightAttenuation, file.mapTextureLightAttenuationLen);
                }
            }
        },
        {
            'MTEX',
            {
                [](WdtLightFile &file, ChunkData &chunkData) {
                    debuglog("Entered MTEX");
                    file.textureFileDataIds =
                        std::vector<uint32_t>(
                            (unsigned long) (chunkData.chunkLen / 4));

                    for (int i = 0; i < file.textureFileDataIds.size(); i++) {
                        chunkData.readValue(file.textureFileDataIds[i]);
                    }
                }
            }
        },
    }
};


void WdtLightFile::process(HFileContent wdtLightFile) {
    m_wdtLightFile = wdtLightFile;

    CChunkFileReader reader(*m_wdtLightFile.get(), getFileNameOrDataId());
    reader.processFile(*this, &WdtLightFile::wdtLightFileTable);

    fsStatus = FileStatus::FSLoaded;
}
