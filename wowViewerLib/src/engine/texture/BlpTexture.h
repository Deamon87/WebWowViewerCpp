//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_BLPTEXTURE_H
#define WOWVIEWERLIB_BLPTEXTURE_H

#include "../persistance/header/blpFileHeader.h"
#include "../../include/sharedFile.h"
#include "../persistance/PersistentFile.h"
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
typedef std::shared_ptr<std::vector<mipmapStruct_t>> HMipmapsVector;

class BlpTexture : public PersistentFile{
public:
    explicit BlpTexture(std::string fileName){ m_textureName = fileName;};
    explicit BlpTexture(int fileDataId){m_textureName = std::to_string(fileDataId);};

    std::string getTextureName() { return m_textureName; };
    void process(HFileContent blpFile, const std::string &fileName) override;
    const HMipmapsVector& getMipmapsVector() {
        return m_mipmaps;
    }

    TextureFormat getTextureFormat() {
        return m_textureFormat;
    }
private:
    std::string m_textureName;

    HMipmapsVector m_mipmaps;
    TextureFormat m_textureFormat = TextureFormat::None;
};


#endif //WOWVIEWERLIB_BLPTEXTURE_H
