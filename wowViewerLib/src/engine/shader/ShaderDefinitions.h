#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H
#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H

#include <string>
#include <iostream>
#include <fstream>

template <typename T>
inline constexpr const uint32_t operator+ (T const val) { return static_cast<const uint32_t>(val); };

struct adtLodShader {
    enum class Attribute {
        aHeight = 0, aIndex = 1, adtLodShaderAttributeEnd
    };
};

struct adtShader {
    enum class Attribute {
        aHeight = 0, aColor = 1, aVertexLighting = 2, aNormal = 3, aIndex = 4, adtShaderAttributeEnd
    };
};


struct drawBBShader {
    enum class Attribute {
        aPosition = 0, drawBBShaderAttributeEnd
    };
};

struct drawDepthShader {
    enum class Attribute {
        position = 0, texture = 1, drawDepthShaderAttributeEnd
    };
};

struct drawFrustumShader {
    enum class Attribute {
        aPosition = 0, drawFrustumShaderAttributeEnd
    };
};

struct drawLinesShader {
 enum class Attribute { 
 aPosition = 0, drawLinesShaderAttributeEnd
 };
 };

struct drawPoints {
 enum class Attribute { 
 aPosition = 0, drawPointsAttributeEnd
 };
 };

struct drawPortalShader {
 enum class Attribute { 
 aPosition = 0, drawPortalShaderAttributeEnd
 };
 };

struct m2ParticleShader {
    enum class Attribute {
        aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, m2ParticleShaderAttributeEnd
    };
};


struct m2Shader {
    enum class Attribute {
        aPosition = 0,
        aNormal = 1,
        bones = 2,
        boneWeights = 3,
        aTexCoord = 4,
        aTexCoord2 = 5,
        aPlacementMat = 6,
        m2ShaderAttributeEnd
    };
};

struct readDepthBufferShader {
    enum class Attribute {
        position = 0, texture = 1, readDepthBufferShaderAttributeEnd
    };
};

struct renderFrameBufferShader {
    enum class Attribute {
        a_position = 0, renderFrameBufferShaderAttributeEnd
    };
};

struct textureCompositionShader {
    enum class Attribute {
        aTextCoord = 0, textureCompositionShaderAttributeEnd
    };
};

struct wmoShader {
    enum class Attribute {
        aPosition = 0,
        aNormal = 1,
        aTexCoord = 2,
        aTexCoord2 = 3,
        aTexCoord3 = 4,
        aColor = 5,
        aColor2 = 6,
        wmoShaderAttributeEnd
    };
};

std::string loadShader(std::string shaderName);

#endif //WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H