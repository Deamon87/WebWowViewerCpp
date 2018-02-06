#include <string>
#include "shadersStructures.h"

constexpr const char* const adtShaderString =
 #include <../glsl/adtShader.glsl>
;

struct adtShader {
 enum class Attribute { 
 aHeight = 0, aColor = 1, aNormal = 2, aIndex = 3, adtShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (adtShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem adtShaderAttributes [] = {
 {"aHeight", +adtShader::Attribute::aHeight},
 {"aColor", +adtShader::Attribute::aColor},
 {"aNormal", +adtShader::Attribute::aNormal},
 {"aIndex", +adtShader::Attribute::aIndex},
 };
constexpr const static shaderDefinition adtShaderDef = { 
adtShaderString,
4,
adtShaderAttributes
};

constexpr const char* const drawBBShaderString =
 #include <../glsl/drawBBShader.glsl>
;

struct drawBBShader {
 enum class Attribute { 
 aPosition = 0, drawBBShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (drawBBShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem drawBBShaderAttributes [] = {
 {"aPosition", +drawBBShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawBBShaderDef = { 
drawBBShaderString,
1,
drawBBShaderAttributes
};

constexpr const char* const drawDepthShaderString =
 #include <../glsl/drawDepthShader.glsl>
;

struct drawDepthShader {
 enum class Attribute { 
 position = 0, texture = 1, drawDepthShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (drawDepthShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem drawDepthShaderAttributes [] = {
 {"position", +drawDepthShader::Attribute::position},
 {"texture", +drawDepthShader::Attribute::texture},
 };
constexpr const static shaderDefinition drawDepthShaderDef = { 
drawDepthShaderString,
2,
drawDepthShaderAttributes
};

constexpr const char* const drawFrustumShaderString =
 #include <../glsl/drawFrustumShader.glsl>
;

struct drawFrustumShader {
 enum class Attribute { 
 aPosition = 0, drawFrustumShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (drawFrustumShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem drawFrustumShaderAttributes [] = {
 {"aPosition", +drawFrustumShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawFrustumShaderDef = { 
drawFrustumShaderString,
1,
drawFrustumShaderAttributes
};

constexpr const char* const drawLinesShaderString =
 #include <../glsl/drawLinesShader.glsl>
;

struct drawLinesShader {
 enum class Attribute { 
 aPosition = 0, drawLinesShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (drawLinesShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem drawLinesShaderAttributes [] = {
 {"aPosition", +drawLinesShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawLinesShaderDef = { 
drawLinesShaderString,
1,
drawLinesShaderAttributes
};

constexpr const char* const drawPointsString =
 #include <../glsl/drawPoints.glsl>
;

struct drawPoints {
 enum class Attribute { 
 aPosition = 0, drawPointsAttributeEnd
 };
 };
inline constexpr const int operator+ (drawPoints::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem drawPointsAttributes [] = {
 {"aPosition", +drawPoints::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawPointsDef = { 
drawPointsString,
1,
drawPointsAttributes
};

constexpr const char* const drawPortalShaderString =
 #include <../glsl/drawPortalShader.glsl>
;

struct drawPortalShader {
 enum class Attribute { 
 aPosition = 0, drawPortalShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (drawPortalShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem drawPortalShaderAttributes [] = {
 {"aPosition", +drawPortalShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawPortalShaderDef = { 
drawPortalShaderString,
1,
drawPortalShaderAttributes
};

constexpr const char* const m2ParticleShaderString =
 #include <../glsl/m2ParticleShader.glsl>
;

struct m2ParticleShader {
 enum class Attribute { 
 aPosition = 0, aColor = 1, aTexcoord0 = 2, aTexcoord1 = 3, aTexcoord2 = 4, m2ParticleShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (m2ParticleShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem m2ParticleShaderAttributes [] = {
 {"aPosition", +m2ParticleShader::Attribute::aPosition},
 {"aColor", +m2ParticleShader::Attribute::aColor},
 {"aTexcoord0", +m2ParticleShader::Attribute::aTexcoord0},
 {"aTexcoord1", +m2ParticleShader::Attribute::aTexcoord1},
 {"aTexcoord2", +m2ParticleShader::Attribute::aTexcoord2},
 };
constexpr const static shaderDefinition m2ParticleShaderDef = { 
m2ParticleShaderString,
5,
m2ParticleShaderAttributes
};

constexpr const char* const m2ShaderString =
 #include <../glsl/m2Shader.glsl>
;

struct m2Shader {
 enum class Attribute { 
 aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, aDiffuseColor = 6, aPlacementMat = 7, m2ShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (m2Shader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem m2ShaderAttributes [] = {
 {"aPosition", +m2Shader::Attribute::aPosition},
 {"aNormal", +m2Shader::Attribute::aNormal},
 {"bones", +m2Shader::Attribute::bones},
 {"boneWeights", +m2Shader::Attribute::boneWeights},
 {"aTexCoord", +m2Shader::Attribute::aTexCoord},
 {"aTexCoord2", +m2Shader::Attribute::aTexCoord2},
 {"aDiffuseColor", +m2Shader::Attribute::aDiffuseColor},
 {"aPlacementMat", +m2Shader::Attribute::aPlacementMat},
 };
constexpr const static shaderDefinition m2ShaderDef = { 
m2ShaderString,
8,
m2ShaderAttributes
};

constexpr const char* const readDepthBufferShaderString =
 #include <../glsl/readDepthBufferShader.glsl>
;

struct readDepthBufferShader {
 enum class Attribute { 
 position = 0, texture = 1, readDepthBufferShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (readDepthBufferShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem readDepthBufferShaderAttributes [] = {
 {"position", +readDepthBufferShader::Attribute::position},
 {"texture", +readDepthBufferShader::Attribute::texture},
 };
constexpr const static shaderDefinition readDepthBufferShaderDef = { 
readDepthBufferShaderString,
2,
readDepthBufferShaderAttributes
};

constexpr const char* const renderFrameBufferShaderString =
 #include <../glsl/renderFrameBufferShader.glsl>
;

struct renderFrameBufferShader {
 enum class Attribute { 
 a_position = 0, renderFrameBufferShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (renderFrameBufferShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem renderFrameBufferShaderAttributes [] = {
 {"a_position", +renderFrameBufferShader::Attribute::a_position},
 };
constexpr const static shaderDefinition renderFrameBufferShaderDef = { 
renderFrameBufferShaderString,
1,
renderFrameBufferShaderAttributes
};

constexpr const char* const textureCompositionShaderString =
 #include <../glsl/textureCompositionShader.glsl>
;

struct textureCompositionShader {
 enum class Attribute { 
 aTextCoord = 0, textureCompositionShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (textureCompositionShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem textureCompositionShaderAttributes [] = {
 {"aTextCoord", +textureCompositionShader::Attribute::aTextCoord},
 };
constexpr const static shaderDefinition textureCompositionShaderDef = { 
textureCompositionShaderString,
1,
textureCompositionShaderAttributes
};

constexpr const char* const wmoShaderString =
 #include <../glsl/wmoShader.glsl>
;

struct wmoShader {
 enum class Attribute { 
 aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aTexCoord3 = 4, aColor = 5, aColor2 = 6, wmoShaderAttributeEnd
 };
 };
inline constexpr const int operator+ (wmoShader::Attribute const val) { return static_cast<const int>(val); };
constexpr shaderItem wmoShaderAttributes [] = {
 {"aPosition", +wmoShader::Attribute::aPosition},
 {"aNormal", +wmoShader::Attribute::aNormal},
 {"aTexCoord", +wmoShader::Attribute::aTexCoord},
 {"aTexCoord2", +wmoShader::Attribute::aTexCoord2},
 {"aTexCoord3", +wmoShader::Attribute::aTexCoord3},
 {"aColor", +wmoShader::Attribute::aColor},
 {"aColor2", +wmoShader::Attribute::aColor2},
 };
constexpr const static shaderDefinition wmoShaderDef = { 
wmoShaderString,
7,
wmoShaderAttributes
};

constexpr innerType map[] = {
  { "adtShader", adtShaderDef },
 { "drawBBShader", drawBBShaderDef },
 { "drawDepthShader", drawDepthShaderDef },
 { "drawFrustumShader", drawFrustumShaderDef },
 { "drawLinesShader", drawLinesShaderDef },
 { "drawPoints", drawPointsDef },
 { "drawPortalShader", drawPortalShaderDef },
 { "m2ParticleShader", m2ParticleShaderDef },
 { "m2Shader", m2ShaderDef },
 { "readDepthBufferShader", readDepthBufferShaderDef },
 { "renderFrameBufferShader", renderFrameBufferShaderDef },
 { "textureCompositionShader", textureCompositionShaderDef },
 { "wmoShader", wmoShaderDef },
 
 };