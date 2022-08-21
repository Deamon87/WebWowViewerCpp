//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include <algorithm>
#include <fstream>
#include "../../engine/opengl/header.h"
#include "GDeviceGL20.h"
#include "../../engine/algorithms/hashString.h"
#include "shaders/GM2ShaderPermutationGL20.h"
#include "meshes/GM2MeshGL20.h"
#include "GOcclusionQueryGL20.h"
#include "meshes/GParticleMeshGL20.h"
#include "shaders/GM2ParticleShaderPermutationGL20.h"
#include "shaders/GAdtShaderPermutationGL20.h"
#include "shaders/GWMOShaderPermutationGL20.h"
#include "../../engine/stringTrim.h"
#include "buffers/GVertexBufferDynamicGL20.h"


BlendModeDesc blendModes[(int)EGxBlendEnum::GxBlend_MAX] = {
        /*GxBlend_Opaque*/           {false,GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
        /*GxBlend_AlphaKey*/         {false,GL_ONE,GL_ZERO,GL_ONE,GL_ZERO},
        /*GxBlend_Alpha*/            {true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA},
        /*GxBlend_Add*/              {true,GL_SRC_ALPHA,GL_ONE,GL_ZERO,GL_ONE},
        /*GxBlend_Mod*/              {true,GL_DST_COLOR,GL_ZERO,GL_DST_ALPHA,GL_ZERO},
        /*GxBlend_Mod2x*/            {true,GL_DST_COLOR,GL_SRC_COLOR,GL_DST_ALPHA,GL_SRC_ALPHA},
        /*GxBlend_ModAdd*/           {true,GL_DST_COLOR,GL_ONE,GL_DST_ALPHA,GL_ONE},
        /*GxBlend_InvSrcAlphaAdd*/   {true,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE},
        /*GxBlend_InvSrcAlphaOpaque*/{true,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO},
        /*GxBlend_SrcAlphaOpaque*/   {true,GL_SRC_ALPHA,GL_ZERO,GL_SRC_ALPHA,GL_ZERO},
        /*GxBlend_NoAlphaAdd*/       {true,GL_ONE,GL_ONE,GL_ZERO,GL_ONE},
        /*GxBlend_ConstantAlpha*/    {true,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA,GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA},
        /*GxBlend_Screen*/           {true,GL_ONE_MINUS_DST_COLOR,GL_ONE,GL_ONE,GL_ZERO},
        /*GxBlend_BlendAdd*/         {true,GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA}
};

//std::string source_to_string(KHR_debug::Source source) {
//    if(source == KHR_debug::Source::API) {
//        return "GL_DEBUG_SOURCE_API";
//    } else if(source == KHR_debug::Source::WINDOW_SYSTEM) {
//        return "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
//    } else if(source == KHR_debug::Source::SHADER_COMPILER) {
//        return "GL_DEBUG_SOURCE_SHADER_COMPILER";
//    } else if(source == KHR_debug::Source::THIRD_PARTY) {
//        return "GL_DEBUG_SOURCE_THIRD_PARTY";
//    } else if(source == KHR_debug::Source::APPLICATION) {
//        return "GL_DEBUG_SOURCE_APPLICATION";
//    } else if(source == KHR_debug::Source::OTHER) {
//        return "GL_DEBUG_SOURCE_OTHER";
//    } else {
//        return "INVALID_SOURCE_ENUM";
//    }
//}

void debug_func(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
//        if (type == GL_DEBUG_TYPE_ERROR) {
//            fprintf(stdout, "source: %u, type: %u, id: %u, severity: %u, msg: %s\n",
//                    source,
//                    type,
//                    id,
//                    severity,
//                    std::string(message, message + length).c_str());
//            fflush(stdout);
//        }
}

void GDeviceGL20::bindIndexBuffer(IIndexBuffer *buffer) {
    GIndexBufferGL20 * gBuffer = (GIndexBufferGL20 *) buffer;
    if (gBuffer == nullptr ) {
        if (m_lastBindIndexBuffer != nullptr) {
            ((GIndexBufferGL20 * )m_lastBindIndexBuffer)->unbind();
            m_lastBindIndexBuffer = nullptr;
        }
    } else if (gBuffer != m_lastBindIndexBuffer) {
        gBuffer->bind();
        m_lastBindIndexBuffer = gBuffer;
    }
}

void GDeviceGL20::bindVertexBuffer(IVertexBuffer *buffer)  {
//    GVertexBufferGL20 *gbuffer = (IVertexBuffer *) buffer;

    if (buffer == nullptr) {
        if (m_lastBindVertexBuffer != nullptr) {
            m_lastBindVertexBuffer->unbind();
            m_lastBindVertexBuffer = nullptr;
        }
    }  else if (buffer != m_lastBindVertexBuffer) {
        buffer->bind();
        m_lastBindVertexBuffer = buffer;
    }
}

void GDeviceGL20::bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length)  {
    if (m_shaderPermutation != nullptr) {
        m_shaderPermutation->bindUniformBuffer(buffer, slot, offset, length);
    }
}

void GDeviceGL20::bindVertexBufferBindings(IVertexBufferBindings *buffer) {
    GVertexBufferBindingsGL20 *gbuffer = ((GVertexBufferBindingsGL20 *) buffer);

    if (buffer == nullptr) {
       if (m_vertexBufferBindings != nullptr) {
//           m_vertexBufferBindings->unbind();
       }
        ( (GVertexBufferBindingsGL20 *)m_defaultVao.get())->bind();
        m_vertexBufferBindings = (GVertexBufferBindingsGL20 *)(m_defaultVao.get());

        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
    } else {
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;

        if (gbuffer != m_vertexBufferBindings) {
            gbuffer->bind();
            m_vertexBufferBindings = (GVertexBufferBindingsGL20 *) buffer;
        }
    }
}

std::shared_ptr<IShaderPermutation> GDeviceGL20::getShader(std::string shaderName, void *permutationDescriptor) {
    const char * cstr = shaderName.c_str();
    size_t hash = CalculateFNV(cstr);
    if (m_shaderPermutCache.count(hash) > 0) {
        HGShaderPermutation ptr = m_shaderPermutCache.at(hash);
        return ptr;
    }

    std::shared_ptr<IShaderPermutation> sharedPtr;
    IShaderPermutation *iPremutation = nullptr;
    if (shaderName == "m2Shader") {
        M2ShaderCacheRecord *cacheRecord = (M2ShaderCacheRecord * )permutationDescriptor;
        if (cacheRecord != nullptr) {


            auto i = m2ShaderCache.find(*cacheRecord);
            if (i != m2ShaderCache.end()) {
                if (!i->second.expired()) {
                    return i->second.lock();
                } else {
                    m2ShaderCache.erase(i);
                }
            }
        }

        iPremutation = new GM2ShaderPermutationGL20(shaderName, this, *cacheRecord);
        sharedPtr.reset(iPremutation);

        std::weak_ptr<IShaderPermutation> weakPtr(sharedPtr);
        m2ShaderCache[*cacheRecord] = weakPtr;
    } else if (shaderName == "m2ParticleShader") {
        iPremutation = new GM2ParticleShaderPermutationGL20(shaderName, this);
        sharedPtr.reset(iPremutation);
        m_shaderPermutCache[hash] = sharedPtr;
    } else if (shaderName == "wmoShader") {
        WMOShaderCacheRecord *cacheRecord = (WMOShaderCacheRecord * )permutationDescriptor;
        if (cacheRecord != nullptr) {


            auto i = wmoShaderCache.find(*cacheRecord);
            if (i != wmoShaderCache.end()) {
                if (!i->second.expired()) {
                    return i->second.lock();
                } else {
                    wmoShaderCache.erase(i);
                }
            }
        }

        iPremutation = new GWMOShaderPermutationGL20(shaderName, this, *cacheRecord);
        sharedPtr.reset(iPremutation);

        std::weak_ptr<IShaderPermutation> weakPtr(sharedPtr);
        wmoShaderCache[*cacheRecord] = weakPtr;
    } else if (shaderName == "adtShader") {
        iPremutation = new GAdtShaderPermutationGL20(shaderName, this);
        sharedPtr.reset(iPremutation);
        m_shaderPermutCache[hash] = sharedPtr;
    } else {
        iPremutation = new GShaderPermutationGL20(shaderName, this);
        sharedPtr.reset(iPremutation);
        m_shaderPermutCache[hash] = sharedPtr;
    }

    GShaderPermutationGL20 * gShaderPermutation = (GShaderPermutationGL20 *)sharedPtr.get();

    gShaderPermutation->compileShader("", "");
    if (shaderName == "ribbonShader") {
        glUseProgram(gShaderPermutation->m_programBuffer);
        glUniform1i(gShaderPermutation->getUnf("uTexture"), 0);
        glUseProgram(0);
    }

    return sharedPtr;
}

HGUniformBuffer GDeviceGL20::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBufferGL20> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferGL20(*this, size));

    std::weak_ptr<GUniformBufferGL20> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);

    return h_uniformBuffer;
}

void GDeviceGL20::drawMeshes(std::vector<HGMesh> &meshes) {
    //Collect meshes into batches and create new array for performace
//    int meshesSize = meshes.size();
//    for (int i = 0 ; i < meshesSize - 1; i++) {
//        HGMesh &a = meshes[i];
//        HGMesh &b = meshes[i+1];
//
//        if (a->getIsTransparent() || b->getIsTransparent()) continue;
//
//        if (
//            a->m_bindings == b->m_bindings &&
//            a->m_start == b->m_start &&
//            a->m_end == b->m_end &&
//
//            a->m_textureCount == a->m_textureCount)
//    }

    int j = 0;
//    for (auto &hgMesh : meshes) {
//        this->drawMesh(hgMesh);
//        j++;
//    }
}

void GDeviceGL20::updateBuffers(std::vector<std::vector<HGUniformBufferChunk>*> &bufferChunks, std::vector<HFrameDepedantData> &frameDepedantDataVec) {
    int fullSize = 0;

    for (int i = 0; i < bufferChunks.size(); i++) {
        auto &bufferVec = bufferChunks[i];
        for (auto &bufferChunk : *bufferVec) {
            fullSize += bufferChunk->getSize();
            if (uniformBufferOffsetAlign > 0) {
                int offsetDiff = fullSize % uniformBufferOffsetAlign;
                if (offsetDiff != 0) {
                    int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

                    fullSize += bytesToAdd;
                }
            }
        }
    }
    if (fullSize > aggregationBufferForUpload.size()) {
        aggregationBufferForUpload.resize(fullSize);
    }

    //2. Create buffers and update them
    int currentSize = 0;
    int buffersIndex = 0;

    HGUniformBuffer bufferForUpload = m_UBOFrames[getUpdateFrameNumber()].m_uniformBufferForUpload;
    if (bufferForUpload == nullptr) {
        bufferForUpload = createUniformBuffer(10*1024*1024);
        bufferForUpload->createBuffer();
        m_UBOFrames[getUpdateFrameNumber()].m_uniformBufferForUpload = bufferForUpload;
    }

    auto bufferForUploadGL = ((GUniformBufferGL20 *) bufferForUpload.get());
    //Buffer identifier was changed, so we need to update shader UBO descriptor

    char *pointerForUpload = static_cast<char *>(&aggregationBufferForUpload[0]);
    for (int i = 0; i < bufferChunks.size(); i++) {
        auto &bufferVec = bufferChunks[i];
        for (const auto &bufferChunk : *bufferVec) {
            bufferChunk->setOffset(currentSize);
            bufferChunk->setPointer(&pointerForUpload[currentSize]);
            currentSize += bufferChunk->getSize();

            if (uniformBufferOffsetAlign > 0) {
                int offsetDiff = currentSize % uniformBufferOffsetAlign;
                if (offsetDiff != 0) {
                    int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

                    currentSize += bytesToAdd;
                }
            }
        }
    }

    for (int i = 0; i < bufferChunks.size(); i++) {
        auto &bufferVec = bufferChunks[i];
        auto frameDepData = frameDepedantDataVec[i];
        for (const auto &bufferChunk : *bufferVec) {
            bufferChunk->update(frameDepData);
        }
    }

    if (currentSize > 0) {
        ((GUniformBufferGL20 *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], fullSize);
    }
}

void GDeviceGL20::drawMesh(HGMesh hIMesh, HGUniformBufferChunk matrixChunk) {
    GMeshGL20 * hmesh = (GMeshGL20 *) hIMesh.get();
    if (hmesh->m_end <= 0) return;

    GOcclusionQueryGL20 * gOcclusionQuery = nullptr;
    GM2MeshGL20 * gm2Mesh = nullptr;
    if (hmesh->m_meshType == MeshType::eOccludingQuery) {
        gOcclusionQuery = (GOcclusionQueryGL20 *)(hmesh);
    }
    if (hmesh->m_meshType == MeshType::eM2Mesh) {
        gm2Mesh = (GM2MeshGL20 *)(hmesh);
    }

    HGUniformBuffer bufferForUpload = m_UBOFrames[getDrawFrameNumber()].m_uniformBufferForUpload;

    bindProgram(hmesh->m_shader.get());
    bindVertexBufferBindings(hmesh->m_bindings.get());
    for (int i = 0; i < 5; i++) {

        IUniformBufferChunk *uniformChunk = nullptr;
        if (i == 0) {
            uniformChunk = matrixChunk.get();
        } else {
            uniformChunk = hmesh->m_UniformBuffer[i].get();
        }

        if (uniformChunk != nullptr) {
            bindUniformBuffer(bufferForUpload.get(), i, uniformChunk->getOffset(), uniformChunk->getSize());
        }
    }

    for (int i = 0; i < hmesh->m_textureCount; i++) {
        if (hmesh->m_texture[i] != nullptr && hmesh->m_texture[i]->getIsLoaded()) {
            bindTexture(hmesh->m_texture[i].get(), i);
        } else {
            bindTexture(m_blackPixelTexture.get(), i);
        }
    }

//        GLint current_vao;
//        GLint current_array_buffer;
//        GLint current_element_buffer;
//        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
//        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_array_buffer);
//        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &current_element_buffer);
//
//        std::cout << "current_vao = " << current_vao << std::endl <<
//            " current_array_buffer = " << current_array_buffer << std::endl <<
//            " current_element_buffer = " << current_element_buffer << std::endl <<
//            " hmesh->m_bindings->m_buffer = " << *(GLint *) hmesh->m_bindings->m_buffer << std::endl <<
//            " hmesh->m_bindings->m_bindings[0].vertexBuffer->pIdentifierBuffer = "
//                  << *(GLint *) hmesh->m_bindings->m_bindings[0].vertexBuffer->pIdentifierBuffer << std::endl <<
//            " hmesh->m_bindings->m_indexBuffer->buffer  = "
//                << *(GLint *) hmesh->m_bindings->m_indexBuffer->buffer << std::endl;
//                ;

    if (m_lastDepthWrite != hmesh->m_depthWrite) {
        if (hmesh->m_depthWrite > 0) {
            glDepthMask(GL_TRUE);
        } else {
            glDepthMask(GL_FALSE);
        }

        m_lastDepthWrite = hmesh->m_depthWrite;
    }
    if (m_lastDepthCulling != hmesh->m_depthCulling) {
        if (hmesh->m_depthCulling > 0) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        m_lastDepthCulling = hmesh->m_depthCulling;
    }
    if (m_backFaceCulling != hmesh->m_backFaceCulling) {
        if (hmesh->m_backFaceCulling > 0) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }

        m_backFaceCulling = hmesh->m_backFaceCulling;
    }

    if (m_triCCW != hmesh->m_triCCW) {
        if (hmesh->m_triCCW) {
            glFrontFace(GL_CCW);
        } else {
            glFrontFace(GL_CW);
        }

        m_triCCW = hmesh->m_triCCW;
    }

    if (m_lastColorMask != hmesh->m_colorMask) {
        glColorMask(
            (hmesh->m_colorMask & 0x1) > 0 ? GL_TRUE : GL_FALSE,
            (hmesh->m_colorMask & 0x2) > 0 ? GL_TRUE : GL_FALSE,
            (hmesh->m_colorMask & 0x4) > 0 ? GL_TRUE : GL_FALSE,
            (hmesh->m_colorMask & 0x8) > 0 ? GL_TRUE : GL_FALSE
        );

        m_lastColorMask = hmesh->m_colorMask;
    }

    if (m_lastBlendMode != hmesh->m_blendMode) {
        BlendModeDesc &selectedBlendMode = blendModes[(char)hmesh->m_blendMode];

        if ((m_lastBlendMode == EGxBlendEnum::GxBlend_UNDEFINED) ||
            (blendModes[(char)m_lastBlendMode].blendModeEnable != selectedBlendMode.blendModeEnable )) {
            if (selectedBlendMode.blendModeEnable) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
        }

        glBlendFuncSeparate(
            selectedBlendMode.SrcColor,
            selectedBlendMode.DestColor,
            selectedBlendMode.SrcAlpha,
            selectedBlendMode.DestAlpha
        );
        m_lastBlendMode = hmesh->m_blendMode;
    }

    if (m_isInSkyBoxDepthMode != hmesh->getIsSkyBox()) {
        if (hmesh->getIsSkyBox()) {
            glDepthRange(0.998f, 1.0f);
        } else {
            glDepthRange(0, 0.996f); //default
        }
        m_isInSkyBoxDepthMode = hmesh->getIsSkyBox();
    }

    if (gOcclusionQuery != nullptr) {
        gOcclusionQuery->beginQuery();
    }
    if (gm2Mesh != nullptr && gm2Mesh->m_query != nullptr) {
        ((GOcclusionQueryGL20 *)gm2Mesh->m_query.get())->beginConditionalRendering();
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
    glDrawElements(hmesh->m_element, hmesh->m_end, GL_UNSIGNED_SHORT, (const void *) (hmesh->m_start ));
#pragma clang diagnostic pop

    if (gm2Mesh != nullptr && gm2Mesh->m_query != nullptr) {
        ((GOcclusionQueryGL20 *)gm2Mesh->m_query.get())->endConditionalRendering();
    }

    if (gOcclusionQuery != nullptr) {
        gOcclusionQuery->endQuery();
    }
}

HGVertexBuffer GDeviceGL20::createVertexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GVertexBufferGL20> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferGL20(*this));

    return h_vertexBuffer;
}
HGVertexBufferDynamic GDeviceGL20::createVertexBufferDynamic(size_t size) {
    std::shared_ptr<GVertexBufferDynamicGL20> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferDynamicGL20(*this, size));

    return h_vertexBuffer;
};

HGIndexBuffer GDeviceGL20::createIndexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GIndexBufferGL20> h_indexBuffer;
    h_indexBuffer.reset(new GIndexBufferGL20(*this));

    return h_indexBuffer;
}

HGVertexBufferBindings GDeviceGL20::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindingsGL20> h_vertexBufferBindings;
    h_vertexBufferBindings.reset(new GVertexBufferBindingsGL20(*this));

    return h_vertexBufferBindings;
}

HGMesh GDeviceGL20::createMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GMeshGL20> h_mesh;
    h_mesh.reset(new GMeshGL20(*this, meshTemplate));

    return h_mesh;
}

HGM2Mesh GDeviceGL20::createM2Mesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2MeshGL20> h_mesh;
    h_mesh.reset(new GM2MeshGL20(*this, meshTemplate));

    return h_mesh;
}

HGParticleMesh GDeviceGL20::createParticleMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GParticleMeshGL20> h_mesh;
    h_mesh.reset(new GParticleMeshGL20(*this, meshTemplate));

    return h_mesh;
}

void GDeviceGL20::bindTexture(ITexture *iTexture, int slot) {
    GTextureGL20 * texture = (GTextureGL20 *) iTexture;
    if (texture == nullptr) {
        auto blackPixel = ((GTextureGL20 *) m_blackPixelTexture.get());
        if (m_lastTexture[slot] != nullptr && m_lastTexture[slot] != blackPixel) {
            glActiveTexture(GL_TEXTURE0 + slot);
            blackPixel->bind();
            m_lastTexture[slot] = blackPixel;
        }
    }  else if (texture != m_lastTexture[slot]) {
        glActiveTexture(GL_TEXTURE0 + slot);
        texture->bind();
        m_lastTexture[slot] = texture;
    }
}

HGTexture GDeviceGL20::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
    BlpCacheRecord blpCacheRecord;
    blpCacheRecord.texture = texture;
    blpCacheRecord.wrapX = xWrapTex;
    blpCacheRecord.wrapY = yWrapTex;

    auto i = loadedTextureCache.find(blpCacheRecord);
    if (i != loadedTextureCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            loadedTextureCache.erase(i);
        }
    }

    std::shared_ptr<GBlpTextureGL20> hgTexture;
    hgTexture.reset(new GBlpTextureGL20(*this, texture, xWrapTex, yWrapTex));

    std::weak_ptr<GTextureGL20> weakPtr(hgTexture);
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGTexture GDeviceGL20::createTexture(bool xWrapTex, bool yWrapTex) {
    std::shared_ptr<GTextureGL20> hgTexture;
    hgTexture.reset(new GTextureGL20(*this, xWrapTex, yWrapTex));
    return hgTexture;
}

void GDeviceGL20::bindProgram(IShaderPermutation *iProgram) {
    GShaderPermutationGL20 *program = (GShaderPermutationGL20 *)iProgram;

    if (program == nullptr) {
        if (m_shaderPermutation != nullptr) {
            m_shaderPermutation->unbindProgram();
            m_shaderPermutation = nullptr;
        }
        m_vertexBufferBindings = nullptr;
    } else if (program != m_shaderPermutation) {
        program->bindProgram();
        m_shaderPermutation = program;

//        m_vertexBufferBindings = nullptr;
//        m_vertexUniformBuffer[0] = nullptr;
//        m_vertexUniformBuffer[1] = nullptr;
//        m_vertexUniformBuffer[2] = nullptr;
//        m_fragmentUniformBuffer[0] = nullptr;
//        m_fragmentUniformBuffer[1] = nullptr;
//        m_fragmentUniformBuffer[2] = nullptr;
    }
}

GDeviceGL20::GDeviceGL20() {

}

void GDeviceGL20::  initialize() {
    unsigned int ff = 0xFFFFFFFF;
    unsigned int zero = 0;
    m_blackPixelTexture = createTexture(false, false);
    m_blackPixelTexture->loadData(1,1,&zero, ITextureFormat::itRGBA);

    m_whitePixelTexture = createTexture(false, false);
    m_whitePixelTexture->loadData(1,1,&ff, ITextureFormat::itRGBA);

    m_defaultVao = this->createVertexBufferBindings();

    maxUniformBufferSize = 0;
    uniformBufferOffsetAlign = 0;

//    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
//    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffsetAlign);
    if (getIsAnisFiltrationSupported()) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_anisotropicLevel);
    }

    //From https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
    static const float vertices[] = {
        -1, -1, -1, //0
        1, -1, -1,  //1
        1, -1, 1,   //2
        -1, -1, 1,  //3
        -1, 1, 1,   //4
        1, 1, 1,    //5
        1, 1, -1,   //6
        -1, 1, -1  //7
    };
    static const uint16_t elements[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        7, 6, 6, 1, 1, 0, 0, 7,
        3, 2, 2, 5, 5, 4, 4, 3,
        6, 5, 5, 2, 2, 1, 1, 6,
        0, 3, 3, 4, 4, 7, 7, 0
    };

    static const uint16_t vertexElements[] = {
        0, 1, 2, 0, 2, 3, // back
        2, 3, 4, 2, 4, 5, // top
        0, 1, 6, 0, 6, 7, // bottom
        0, 3, 4, 0, 4, 7, // left
        1, 2, 5, 1, 5, 6, // right
        4, 5, 6, 4, 6, 7  // front
    };


    HGVertexBuffer vertexBuffer =  this->createVertexBuffer();
    vertexBuffer->uploadData((void *) &vertices[0], sizeof(vertices));

    HGIndexBuffer lineIndexBuffer = this->createIndexBuffer();
    lineIndexBuffer->uploadData((void *) &elements[0], sizeof(elements));

    HGIndexBuffer vertexIndexBuffer = this->createIndexBuffer();
    vertexIndexBuffer->uploadData((void *) &vertexElements[0], sizeof(vertexElements));

    GBufferBinding bufferBinding = {0, 3, GBindingType::GFLOAT, false, 0, 0 };

    GVertexBufferBinding binding={};
    binding.bindings.push_back(bufferBinding);
    binding.vertexBuffer = vertexBuffer;

    m_vertexBBBindings = this->createVertexBufferBindings();

    m_vertexBBBindings->setIndexBuffer(vertexIndexBuffer);
    m_vertexBBBindings->addVertexBufferBinding(binding);
    m_vertexBBBindings->save();

    m_lineBBBindings = this->createVertexBufferBindings();

    m_lineBBBindings->setIndexBuffer(lineIndexBuffer);
    m_lineBBBindings->addVertexBufferBinding(binding);
    m_lineBBBindings->save();

    aggregationBufferForUpload = std::vector<char>(maxUniformBufferSize);

    std::cout << "maxUniformBufferSize = " << maxUniformBufferSize << std::endl;

//    glEnable(GL_DEBUG_OUTPUT);
//    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
//    glDebugMessageCallback(debug_func, NULL);
}

HGOcclusionQuery GDeviceGL20::createQuery(HGMesh boundingBoxMesh) {
    std::shared_ptr<GOcclusionQueryGL20> hgOcclusionQuery;
    hgOcclusionQuery.reset(new GOcclusionQueryGL20(*this, boundingBoxMesh));

    return hgOcclusionQuery;
}

void GDeviceGL20::reset() {
    m_triCCW = -1;
    m_lastDepthWrite = -1;
    m_lastDepthCulling = -1;
    m_backFaceCulling = -1;
    m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    m_lastBindIndexBuffer = nullptr;
    m_lastBindVertexBuffer = nullptr;
    m_vertexBufferBindings = nullptr;

    m_lastTexture[0] = nullptr;
    m_lastTexture[1] = nullptr;
    m_lastTexture[2] = nullptr;
    m_lastTexture[3] = nullptr;

    bindUniformBuffer(nullptr, 0, 0, 0);
    bindUniformBuffer(nullptr, 1, 0, 0);
    bindUniformBuffer(nullptr, 2, 0, 0);
    bindUniformBuffer(nullptr, 3, 0, 0);
    bindUniformBuffer(nullptr, 4, 0, 0);

    bindVertexBufferBindings(nullptr);
    bindIndexBuffer(nullptr);
    bindVertexBuffer(nullptr);

    m_shaderPermutation = nullptr;

    ITexture *iTexture = m_blackPixelTexture.get();
    bindTexture(iTexture, 0);
    bindTexture(iTexture, 1);
    bindTexture(iTexture, 2);
    bindTexture(iTexture, 3);
    bindTexture(iTexture, 4);
    bindTexture(iTexture, 5);
    bindTexture(iTexture, 6);
}

unsigned int GDeviceGL20::getUpdateFrameNumber() {
    return (m_frameNumber + 1) & 3;
//    return 0;
}
unsigned int GDeviceGL20::getCullingFrameNumber() {
    return (m_frameNumber + 3) & 3;
//    return 0;
}
unsigned int GDeviceGL20::getOcclusionFrameNumber() {
    return (m_frameNumber + 2) & 3;
}
unsigned int GDeviceGL20::getDrawFrameNumber() {
    return m_frameNumber & 3;
}

void GDeviceGL20::increaseFrameNumber() {
    m_frameNumber++;
}

HGVertexBufferBindings GDeviceGL20::getBBLinearBinding() {
    return m_lineBBBindings;
}

HGVertexBufferBindings GDeviceGL20::getBBVertexBinding() {
    return m_vertexBBBindings;
}

HGPUFence GDeviceGL20::createFence() {
    return nullptr;
}

void GDeviceGL20::uploadTextureForMeshes(std::vector<HGMesh> &meshes) {
    std::vector<HGTexture> textures;
    textures.reserve(meshes.size() * 3);

    int texturesLoaded = 0;

    for (const auto &hmesh : meshes) {
        GMeshGL20 * mesh = (GMeshGL20 *) hmesh.get();
        for (int i = 0; i < mesh->m_textureCount; i++) {
            textures.push_back(mesh->m_texture[i]);
        }
    }

    std::sort(textures.begin(), textures.end());
    textures.erase( unique( textures.begin(), textures.end() ), textures.end() );

    for (const auto &texture : textures) {
        if (texture == nullptr) continue;
        if (texture->postLoad()) texturesLoaded++;
        if (texturesLoaded > 4) return;
    }
}

#ifdef __ANDROID_API__
#include "../../engine/androidLogSupport.h"
#endif
#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}
std::string GDeviceGL20::loadShader(std::string fileName, IShaderType shaderType) {
    std::string fullPath;
    trim(fileName);

    fullPath = "./glsl/glsl20/" + fileName;
    switch (shaderType) {
        case IShaderType::gVertexShader :
            fullPath += ".vert";
            break;

        case IShaderType::gFragmentShader :
            fullPath += ".frag";
            break;
    }

    ShaderContentCacheRecord hashRecord;
    hashRecord.fileName = fullPath;
    hashRecord.shaderType = shaderType;


    auto i = shaderCache.find(hashRecord);
    if (i != shaderCache.end()) {
        return i->second;
    }

#ifdef __ANDROID_API__
        //TODO: pass this stuff here
    AAssetManager *mgr = g_assetMgr;
    if (g_assetMgr == nullptr) {
        std::cout << "g_assetMgr == nullptr";
    }
    std::string filename = fullPath;
    //Trim dot and slash at the start
    if (filename[0] == '.')
        filename = filename.substr(1, filename.length()-1);
    if (filename[0] == '/')
        filename = filename.substr(1, filename.length()-1);

    std::cout << "AAssetManager_open" << std::endl;
    std::cout << "Opening assest at \"" << filename << "\n" << std::endl;
    AAsset* asset = AAssetManager_open(mgr, filename.c_str(), AASSET_MODE_STREAMING);
    std::cout << "AAssetManager_opened" << std::endl;
    if (asset == nullptr) {
        std::cout << "asset == nullptr" << std::endl;
    }

    char buf[BUFSIZ];
    std::vector<char> outBuf;

    std::cout << "buffers created" << std::endl;
    int nb_read = 0;
    while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0) {
        std::cout << "file read" << std::endl;
        for (int i = 0; i < nb_read; i++) {
            outBuf.push_back(buf[i]);
        }
    }
        //std::copy(&buf[0], &buf[nb_read-1]+1, outPutIterator);
    std::cout << "file fully read" << std::endl;
    AAsset_close(asset);
    std::cout << "asset closed" << std::endl;
    logExecution
    std::string result = std::string(outBuf.begin(), outBuf.end());
    logExecution
#else
    std::ifstream t(fullPath);

    std::string result = std::string((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());
#endif
    logExecution

    //Hack fix for bones
    {
        auto start = result.find("[bones");
        while(start != std::string::npos) {
            auto end = result.find("]", start);

            result = result.substr(0, start) + "[int(" + result.substr(start+1, end-start-1)+")"+ result.substr(end);
            start = result.find("[bones");
        }
    }


    shaderCache[hashRecord] = result;
    return result;
}

float GDeviceGL20::getAnisLevel() {
    return m_anisotropicLevel;
}

void GDeviceGL20::clearScreen() {
    if (m_isInvertZ) {
        glClearDepthf(0.0f);
    } else {
        glClearDepthf(1.0f);
    }

    glDisable(GL_DEPTH_TEST);
    if (m_isInvertZ) {
        glDepthFunc(GL_GEQUAL);
    } else {
        glDepthFunc(GL_LEQUAL);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    //Note: the scissor dimensions are taken from call to setViewPortDimensions
    glEnable(GL_SCISSOR_TEST);
//    glClearColor(0.0, 0.0, 0.0, 0.0);
//    glClearColor(0.25, 0.06, 0.015, 0.0);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], 1);
    //glClearColor(fogColor[0], fogColor[1], fogColor[2], 1);
//    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glDisable(GL_SCISSOR_TEST);

}

void GDeviceGL20::setClearScreenColor(float r, float g, float b) {
    clearColor[0] = r;
    clearColor[1] = g;
    clearColor[2] = b;
}

void GDeviceGL20::beginFrame() {

}

void GDeviceGL20::commitFrame() {

}

void GDeviceGL20::setViewPortDimensions(float x, float y, float width, float height) {
    glViewport(x,y,width,height);
    glScissor(x,y,width,height);
}

void GDeviceGL20::shrinkData()  {
    for (int i=0; i < 4; i++) {
        m_UBOFrames[i] = {};
    }

    aggregationBufferForUpload = {};
}

void GDeviceGL20::drawStageAndDeps(HDrawStage drawStage) {

    for (int i = 0; i < drawStage->drawStageDependencies.size(); i++) {
        this->drawStageAndDeps(drawStage->drawStageDependencies[i]);
    }

    if (drawStage->target != nullptr) {
        drawStage->target->bindFrameBuffer();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    this->setViewPortDimensions(
        drawStage->viewPortDimensions.mins[0],
        drawStage->viewPortDimensions.mins[1],
        drawStage->viewPortDimensions.maxs[0],
        drawStage->viewPortDimensions.maxs[1]
    );

    this->setInvertZ(drawStage->invertedZ);

    if (drawStage->clearScreen) {
        clearColor[0] = drawStage->clearColor[0];
        clearColor[1] = drawStage->clearColor[1];
        clearColor[2] = drawStage->clearColor[2];
        this->clearScreen();
    }


    if (drawStage->opaqueMeshes != nullptr) {
        for (auto hgMesh : drawStage->opaqueMeshes->meshes) {
            this->drawMesh(hgMesh, drawStage->sceneWideBlockVSPSChunk);
        }
    }

    if (drawStage->transparentMeshes != nullptr) {
        for (auto hgMesh : drawStage->transparentMeshes->meshes) {
            this->drawMesh(hgMesh, drawStage->sceneWideBlockVSPSChunk);
        }
    }

    if (drawStage->target != nullptr) {
        drawStage->target->copyRenderBufferToTexture();
    }
//    drawMeshes(drawStage->meshesToRender->meshes);

}
