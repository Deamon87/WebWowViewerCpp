//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_BLPTEXTURE_H
#define WOWVIEWERLIB_BLPTEXTURE_H

#include "../persistance/header/blpFileHeader.h"
#include "../../include/wowScene.h"
#include <vector>
enum class TextureFormat {
    None,
    S3TC_RGBA_DXT1,
    S3TC_RGB_DXT1,
    S3TC_RGBA_DXT3,
    S3TC_RGBA_DXT5,
    BGRA,
    RGBA,
    PalARGB1555DitherFloydSteinberg,
    PalARGB4444DitherFloydSteinberg,
    PalARGB2565DitherFloydSteinberg
};

struct mipmapStruct_t {
    std::vector<uint8_t> texture;
    int32_t width;
    int32_t height;
};
typedef std::vector<mipmapStruct_t> MipmapsVector;

class BlpTexture {
public:
    BlpTexture(std::string fileName){};
    BlpTexture(int fileDataId){};

    std::string getTextureName() { return m_textureName; };
    void process(HFileContent blpFile, const std::string &fileName);
    bool getIsLoaded() { return m_isLoaded; };
    const MipmapsVector& getMipmapsVector() {
        return m_mipmaps;
    }

    TextureFormat getTextureFormat() {
        return m_textureFormat;
    }
private:
    bool m_isLoaded = false;
    std::string m_textureName;

    MipmapsVector m_mipmaps;
    TextureFormat m_textureFormat;
};


#endif //WOWVIEWERLIB_BLPTEXTURE_H
