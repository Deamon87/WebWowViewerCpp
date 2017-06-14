#include <string>
#include "shadersStructures.h"

constexpr const char* const adtShaderString =
 #include <adtShader.glsl>
;

struct adtShader {
 enum class Attribute { 
 aHeight = 0, aNormal = 1, aIndex = 2, adtShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (adtShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem adtShaderAttributes [] = {
 {"aHeight", +adtShader::Attribute::aHeight},
 {"aNormal", +adtShader::Attribute::aNormal},
 {"aIndex", +adtShader::Attribute::aIndex},
 };
constexpr const static shaderDefinition adtShaderDef = { 
adtShaderString,
3,
adtShaderAttributes
};

constexpr const char* const drawBBShaderString =
 #include <drawBBShader.glsl>
;

struct drawBBShader {
 enum class Attribute { 
 aPosition = 0, drawBBShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (drawBBShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem drawBBShaderAttributes [] = {
 {"aPosition", +drawBBShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawBBShaderDef = { 
drawBBShaderString,
1,
drawBBShaderAttributes
};

constexpr const char* const drawDepthShaderString =
 #include <drawDepthShader.glsl>
;

struct drawDepthShader {
 enum class Attribute { 
 position = 0, texture = 1, drawDepthShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (drawDepthShader::Attribute const val) { return static_cast<unsigned>(val); };
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
 #include <drawFrustumShader.glsl>
;

struct drawFrustumShader {
 enum class Attribute { 
 aPosition = 0, drawFrustumShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (drawFrustumShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem drawFrustumShaderAttributes [] = {
 {"aPosition", +drawFrustumShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawFrustumShaderDef = { 
drawFrustumShaderString,
1,
drawFrustumShaderAttributes
};

constexpr const char* const drawLinesShaderString =
 #include <drawLinesShader.glsl>
;

struct drawLinesShader {
 enum class Attribute { 
 aPosition = 0, drawLinesShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (drawLinesShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem drawLinesShaderAttributes [] = {
 {"aPosition", +drawLinesShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawLinesShaderDef = { 
drawLinesShaderString,
1,
drawLinesShaderAttributes
};

constexpr const char* const drawPortalShaderString =
 #include <drawPortalShader.glsl>
;

struct drawPortalShader {
 enum class Attribute { 
 aPosition = 0, drawPortalShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (drawPortalShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem drawPortalShaderAttributes [] = {
 {"aPosition", +drawPortalShader::Attribute::aPosition},
 };
constexpr const static shaderDefinition drawPortalShaderDef = { 
drawPortalShaderString,
1,
drawPortalShaderAttributes
};

constexpr const char* const m2ShaderString =
 #include <m2Shader.glsl>
;

struct m2Shader {
 enum class Attribute { 
 aPosition = 0, aNormal = 1, bones = 2, boneWeights = 3, aTexCoord = 4, aTexCoord2 = 5, aDiffuseColor = 6, aPlacementMat = 7, m2ShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (m2Shader::Attribute const val) { return static_cast<unsigned>(val); };
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
 #include <readDepthBufferShader.glsl>
;

struct readDepthBufferShader {
 enum class Attribute { 
 position = 0, texture = 1, readDepthBufferShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (readDepthBufferShader::Attribute const val) { return static_cast<unsigned>(val); };
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
 #include <renderFrameBufferShader.glsl>
;

struct renderFrameBufferShader {
 enum class Attribute { 
 a_position = 0, renderFrameBufferShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (renderFrameBufferShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem renderFrameBufferShaderAttributes [] = {
 {"a_position", +renderFrameBufferShader::Attribute::a_position},
 };
constexpr const static shaderDefinition renderFrameBufferShaderDef = { 
renderFrameBufferShaderString,
1,
renderFrameBufferShaderAttributes
};

constexpr const char* const textureCompositionShaderString =
 #include <textureCompositionShader.glsl>
;

struct textureCompositionShader {
 enum class Attribute { 
 aTextCoord = 0, textureCompositionShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (textureCompositionShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem textureCompositionShaderAttributes [] = {
 {"aTextCoord", +textureCompositionShader::Attribute::aTextCoord},
 };
constexpr const static shaderDefinition textureCompositionShaderDef = { 
textureCompositionShaderString,
1,
textureCompositionShaderAttributes
};

constexpr const char* const wmoShaderString =
 #include <wmoShader.glsl>
;

struct wmoShader {
 enum class Attribute { 
 aPosition = 0, aNormal = 1, aTexCoord = 2, aTexCoord2 = 3, aColor = 4, aColor2 = 5, wmoShaderAttributeEnd
 };
 };
inline constexpr unsigned operator+ (wmoShader::Attribute const val) { return static_cast<unsigned>(val); };
constexpr shaderItem wmoShaderAttributes [] = {
 {"aPosition", +wmoShader::Attribute::aPosition},
 {"aNormal", +wmoShader::Attribute::aNormal},
 {"aTexCoord", +wmoShader::Attribute::aTexCoord},
 {"aTexCoord2", +wmoShader::Attribute::aTexCoord2},
 {"aColor", +wmoShader::Attribute::aColor},
 {"aColor2", +wmoShader::Attribute::aColor2},
 };
constexpr const static shaderDefinition wmoShaderDef = { 
wmoShaderString,
6,
wmoShaderAttributes
};

constexpr innerType map[] = {
  { "adtShader", adtShaderDef },
 { "drawBBShader", drawBBShaderDef },
 { "drawDepthShader", drawDepthShaderDef },
 { "drawFrustumShader", drawFrustumShaderDef },
 { "drawLinesShader", drawLinesShaderDef },
 { "drawPortalShader", drawPortalShaderDef },
 { "m2Shader", m2ShaderDef },
 { "readDepthBufferShader", readDepthBufferShaderDef },
 { "renderFrameBufferShader", renderFrameBufferShaderDef },
 { "textureCompositionShader", textureCompositionShaderDef },
 { "wmoShader", wmoShaderDef },
 
 };