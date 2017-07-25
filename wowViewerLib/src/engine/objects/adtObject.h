//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_ADTOBJECT_H
#define WEBWOWVIEWERCPP_ADTOBJECT_H

class IWoWInnerApi;
class AdtObject;

#include <vector>
#include "../persistance/ChunkFileReader.h"
#include "../persistance/adtFile.h"
#include "../opengl/header.h"
#include "../wowInnerApi.h"

struct mcnkStruct_t {
    MCVT *mcvt;
    MCCV *mccv;
    SMNormal *mcnr;
    SMLayer *mcly;
    uint8_t *mcal;
};

class AdtObject {
public:
    AdtObject() : alphaTextures(){
        m_api = nullptr;

    }
    AdtObject(IWoWInnerApi *api) : m_api(api), alphaTextures()  {}
    void process(std::vector<unsigned char> &adtFile);
private:
    static chunkDef<AdtObject> adtObjectTable;

    void createVBO();
    void createTriangleStrip();
    void loadAlphaTextures(int limit);


    IWoWInnerApi *m_api;
    int alphaTexturesLoaded = 0;
    std::vector<char> alphaTextures;
    bool loaded;

    std::vector<int16_t> strips;
    std::vector<int> stripOffsets;

    int indexOffset = 0;
    int heightOffset = 0;

    GLuint combinedVbo = 0;
    GLuint stripVBO = 0;

private:
    SMMapHeader* mhdr;

    struct {
        SMChunkInfo chunkInfo[16][16];
    } *mcins;

    char *textureNamesField;
    int textureNamesFieldLen;

    char *doodadNamesField;
    int doodadNamesFieldLen;

    char *wmoNamesField;
    int wmoNamesFieldLen;

    int mcnkRead = -1;
    SMChunk mapTile[16*16];
    mcnkStruct_t mcnkStructs[16*16];

    void draw(std::vector<bool> &drawChunks);
};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
