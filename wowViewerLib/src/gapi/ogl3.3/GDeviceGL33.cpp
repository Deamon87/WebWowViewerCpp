//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include <algorithm>
#include <fstream>
#include "../../engine/opengl/header.h"
#include "GDeviceGL33.h"
#include "../../engine/algorithms/hashString.h"
#include "shaders/GM2ShaderPermutationGL33.h"
#include "meshes/GM2MeshGL33.h"
#include "GOcclusionQueryGL33.h"
#include "meshes/GParticleMeshGL33.h"
#include "shaders/GM2ParticleShaderPermutationGL33.h"
#include "shaders/GAdtShaderPermutationGL33.h"
#include "shaders/GWMOShaderPermutationGL33.h"
#include "../../engine/stringTrim.h"
#include "buffers/GVertexBufferDynamicGL33.h"
#include "buffers/GUnformBufferChunk33.h"
#include "../../engine/DrawStage.h"

namespace GL33 {
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


    void debug_func(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                    const void *userParam) {
        fprintf(stdout, "source: %u, type: %u, id: %u, severity: %u, msg: %s\n",
                source,
                type,
                id,
                severity,
                std::string(message, message + length).c_str());
        if (severity == 37190) {
            std::cout << "lol";
        }

        fflush(stdout);
    }
}

void GDeviceGL33::bindIndexBuffer(IIndexBuffer *buffer) {
    GIndexBufferGL33 * gBuffer = (GIndexBufferGL33 *) buffer;
    if (gBuffer == nullptr ) {
        if (m_lastBindIndexBuffer != nullptr) {
            ((GIndexBufferGL33 * )m_lastBindIndexBuffer)->unbind();
            m_lastBindIndexBuffer = nullptr;
        }
    } else if (gBuffer != m_lastBindIndexBuffer) {
        gBuffer->bind();
        m_lastBindIndexBuffer = gBuffer;
    }
}

void GDeviceGL33::bindVertexBuffer(IVertexBuffer *buffer)  {
//    GVertexBufferGL33 *gbuffer = (IVertexBuffer *) buffer;

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

void GDeviceGL33::bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length) {
    GUniformBufferGL33 *gbuffer = (GUniformBufferGL33 *) buffer;

    if (buffer == nullptr) {
        if (m_UniformBuffer[slot].buffer != nullptr) {
            m_UniformBuffer[slot].buffer->unbind();
            m_UniformBuffer[slot] = {};
        }
    }  else {
        if ((buffer != m_UniformBuffer[slot].buffer) || (m_UniformBuffer[slot].offset != offset)) {
            gbuffer->bind(slot, offset, length);

            m_UniformBuffer[slot] = {gbuffer, (uint32_t)offset};
        }
    }
}

void GDeviceGL33::bindVertexBufferBindings(IVertexBufferBindings *buffer) {
    GVertexBufferBindingsGL33 *gbuffer = ((GVertexBufferBindingsGL33 *) buffer);

    if (buffer == nullptr) {
       if (m_vertexBufferBindings != nullptr) {
//           m_vertexBufferBindings->unbind();
       }
        ( (GVertexBufferBindingsGL33 *)m_defaultVao.get())->bind();
        m_vertexBufferBindings = (GVertexBufferBindingsGL33 *)(m_defaultVao.get());

        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
    } else {
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;

        if (gbuffer != m_vertexBufferBindings) {
            gbuffer->bind();
            m_vertexBufferBindings = (GVertexBufferBindingsGL33 *) buffer;
        }
    }
}

std::shared_ptr<IShaderPermutation> GDeviceGL33::getShader(std::string shaderName, void *permutationDescriptor) {
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

        iPremutation = new GM2ShaderPermutationGL33(shaderName, this, *cacheRecord);
        sharedPtr.reset(iPremutation);

        std::weak_ptr<IShaderPermutation> weakPtr(sharedPtr);
        m2ShaderCache[*cacheRecord] = weakPtr;
    } else if (shaderName == "m2ParticleShader") {
        iPremutation = new GM2ParticleShaderPermutationGL33(shaderName, this);
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

        iPremutation = new GWMOShaderPermutationGL33(shaderName, this, *cacheRecord);
        sharedPtr.reset(iPremutation);

        std::weak_ptr<IShaderPermutation> weakPtr(sharedPtr);
        wmoShaderCache[*cacheRecord] = weakPtr;
    } else if (shaderName == "adtShader") {
        iPremutation = new GAdtShaderPermutationGL33(shaderName, this);
        sharedPtr.reset(iPremutation);
        m_shaderPermutCache[hash] = sharedPtr;
    } else {
        iPremutation = new GShaderPermutationGL33(shaderName, this);
        sharedPtr.reset(iPremutation);
        m_shaderPermutCache[hash] = sharedPtr;
    }

    GShaderPermutationGL33 * gShaderPermutation = (GShaderPermutationGL33 *)sharedPtr.get();

    gShaderPermutation->compileShader("", "");
    if (shaderName == "ribbonShader") {
        glUseProgram(gShaderPermutation->m_programBuffer);
        glUniform1i(gShaderPermutation->getUnf("uTexture"), 0);
        glUseProgram(0);
    }

    for (int i = 0; i < 5; i++) {
        if (gShaderPermutation->m_uboBlockIndex[i] > -1) {
            glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboBlockIndex[i], i);
        }
    }

    return sharedPtr;
}

HGUniformBuffer GDeviceGL33::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBufferGL33> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferGL33(*this, size));


    std::weak_ptr<GUniformBufferGL33> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);

    return h_uniformBuffer;
}

void GDeviceGL33::drawStageAndDeps(HDrawStage drawStage) {
    for (int i = 0; i < drawStage->drawStageDependencies.size(); i++) {
        this->drawStageAndDeps(drawStage->drawStageDependencies[i]);
    }

    this->setViewPortDimensions(
        drawStage->viewPortDimensions.mins[0],
        drawStage->viewPortDimensions.mins[1],
        drawStage->viewPortDimensions.maxs[0],
        drawStage->viewPortDimensions.maxs[1]
    );
    if (drawStage->clearScreen) {
        clearColor[0] = drawStage->clearColor[0];
        clearColor[1] = drawStage->clearColor[1];
        clearColor[2] = drawStage->clearColor[2];
        this->clearScreen();
    }


    for (auto hgMesh : drawStage->meshesToRender->meshes) {
        this->drawMesh(hgMesh, drawStage->sceneWideBlockVSPSChunk);
    }

    drawMeshes(drawStage->meshesToRender->meshes);
}

void GDeviceGL33::drawMeshes(std::vector<HGMesh> &meshes) {
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
    for (auto hgMesh : meshes) {
        this->drawMesh(hgMesh, nullptr);
        j++;
    }
}

#ifdef SINGLE_BUFFER_UPLOAD
void GDeviceGL33::updateBuffers(std::vector<HGMesh> &iMeshes, std::vector<HGUniformBufferChunk> additionalChunks) {
    std::vector<HGL33Mesh> &meshes = (std::vector<HGL33Mesh> &) iMeshes;

    //1. Collect buffers
    std::vector<IUniformBufferChunk *> buffers;
    int renderIndex = 0;
    for (const auto &mesh : meshes) {
        for (int i = 0; i < 5; i++ ) {
            IUniformBufferChunk *buffer = (IUniformBufferChunk *) mesh->m_UniformBuffer[i].get();
            if (buffer != nullptr) {
                buffers.push_back(buffer);
            }
        }
    }
    for (const auto &bufferChunks : additionalChunks) {
        if (bufferChunks != nullptr) {
            buffers.push_back(bufferChunks.get());
        }
    }


    std::sort( buffers.begin(), buffers.end());
    buffers.erase( unique( buffers.begin(), buffers.end() ), buffers.end() );

    int fullSize = 0;
    for (auto &buffer : buffers) {
        fullSize += buffer->getSize();
        int offsetDiff = fullSize % uniformBufferOffsetAlign;
        if (offsetDiff != 0) {
            int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

            fullSize += bytesToAdd;
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

    auto bufferForUploadGL = ((GUniformBufferGL33 *) bufferForUpload.get());
    //Buffer identifier was changed, so we need to update shader UBO descriptor
    if (fullSize > 0) {
        char *pointerForUpload = static_cast<char *>(&aggregationBufferForUpload[0]);
        for (const auto &buffer : buffers) {
            buffer->setOffset(currentSize);
            buffer->setPointer(&pointerForUpload[currentSize]);
            currentSize += buffer->getSize();

            int offsetDiff = currentSize % uniformBufferOffsetAlign;
            if (offsetDiff != 0) {
                int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

                currentSize += bytesToAdd;
            }
        }
        assert(currentSize == fullSize);

        for (auto &buffer : buffers) {
            buffer->update();
        }

        if (currentSize > 0) {
            bufferForUploadGL->uploadData(pointerForUpload, currentSize);
        }
    }
}
#else

void GDeviceGL33::updateBuffers(std::vector<HGMesh> &iMeshes, std::vector<HGUniformBufferChunk> additionalChunks) {
    std::vector<HGL33Mesh> &meshes = (std::vector<HGL33Mesh> &) iMeshes;
    aggregationBufferForUpload.resize(maxUniformBufferSize);

    //1. Collect buffers
    std::vector<IUniformBufferChunk *> buffers;
    int renderIndex = 0;
    for (const auto &mesh : meshes) {
        for (int i = 0; i < 5; i++ ) {
            IUniformBufferChunk *buffer = (IUniformBufferChunk *) mesh->m_UniformBuffer[i].get();
            if (buffer != nullptr) {
                buffers.push_back(buffer);
            }
        }
    }


    std::sort( buffers.begin(), buffers.end());
    buffers.erase( unique( buffers.begin(), buffers.end() ), buffers.end() );

    //2. Create buffers and update them
    int currentSize = 0;
    int buffersIndex = 0;

    std::vector<HGUniformBuffer> &m_unfiormBuffersForUpload = m_UBOFrames[getUpdateFrameNumber()].m_uniformBuffersForUpload;
    HGUniformBuffer bufferForUpload;
    if (buffersIndex >= m_unfiormBuffersForUpload.size()) {
        bufferForUpload = createUniformBuffer(maxUniformBufferSize);
        bufferForUpload->createBuffer();
        m_unfiormBuffersForUpload.push_back(bufferForUpload);
    } else {
        bufferForUpload = m_unfiormBuffersForUpload.at(buffersIndex);
    }


    char *pointerForUpload = static_cast<char *>(&aggregationBufferForUpload[0]);
    int lastUploadIndx = 0;
    for (int i = 0; i < buffers.size(); i++) {
        const auto &buffer = buffers[i];

        if ((currentSize + buffer->getSize()) > maxUniformBufferSize) {
            for (int j = lastUploadIndx; j < i; j++) {
                auto &bufferUpl = buffers[j];
                bufferUpl->update();
            }


            lastUploadIndx = i;
            ((GUniformBufferGL33 *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], maxUniformBufferSize);

            buffersIndex++;
            currentSize = 0;

            if (buffersIndex >= m_unfiormBuffersForUpload.size()) {
                bufferForUpload = createUniformBuffer(maxUniformBufferSize);
                bufferForUpload->createBuffer();
                m_unfiormBuffersForUpload.push_back(bufferForUpload);
            } else {
                bufferForUpload = m_unfiormBuffersForUpload.at(buffersIndex);
            }
        }

        buffer->setOffset(currentSize);
        buffer->setPointer(&pointerForUpload[currentSize]);
        ((GUniformBufferChunk33*) buffer)->setUniformBuffer(bufferForUpload);

        currentSize += buffer->getSize();

        int offsetDiff = currentSize % uniformBufferOffsetAlign;
        if (offsetDiff != 0) {
            int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

            currentSize += bytesToAdd;
        }
    }

    if (currentSize > 0) {
        for (int j = lastUploadIndx; j < buffers.size(); j++) {
            auto &bufferUpl = buffers[j];
            bufferUpl->update();
        }

        ((GUniformBufferGL33 *) bufferForUpload.get())->uploadData(pointerForUpload, currentSize);
    }
}
#endif

void GDeviceGL33::drawMesh(HGMesh hIMesh, HGUniformBufferChunk matrixChunk) {
    GMeshGL33 * hmesh = (GMeshGL33 *) hIMesh.get();
    if (hmesh->m_end <= 0) return;

    GOcclusionQueryGL33 * gOcclusionQuery = nullptr;
    GM2MeshGL33 * gm2Mesh = nullptr;
    if (hmesh->m_meshType == MeshType::eOccludingQuery) {
        gOcclusionQuery = (GOcclusionQueryGL33 *)(hmesh);
    }
    if (hmesh->m_meshType == MeshType::eM2Mesh) {
        gm2Mesh = (GM2MeshGL33 *)(hmesh);
    }

#ifndef __EMSCRIPTEN__
    HGUniformBuffer bufferForUpload = m_UBOFrames[getDrawFrameNumber()].m_uniformBufferForUpload;
#endif

    bindProgram(hmesh->m_shader.get());
    bindVertexBufferBindings(hmesh->m_bindings.get());


#ifndef __EMSCRIPTEN__
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
#else
    for (int i = 0; i < 5; i++) {
        GUniformBufferChunk33 * uniformChunk = (GUniformBufferChunk33 *) hmesh->m_UniformBuffer[i].get();
        if (uniformChunk != nullptr) {
            IUniformBuffer *uniformBuffer = nullptr;
            if (i == 0) {
                bufferForUpload = matrixChunk->getUniformBuffer().get();
            } else {
                bufferForUpload = uniformChunk->getUniformBuffer().get();
            }

            bindUniformBuffer(bufferForUpload, i, uniformChunk->getOffset(), uniformChunk->getSize());
        }
    }
#endif

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

    if (m_isScissorsEnabled != hmesh->m_isScissorsEnabled) {
        if (hmesh->m_isScissorsEnabled) {
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }

        m_isScissorsEnabled = true;
    }
    if (hmesh->m_isScissorsEnabled) {
        glScissor(
            hmesh->m_scissorOffset[0],
            hmesh->m_scissorOffset[1],
            hmesh->m_scissorSize[0],
            hmesh->m_scissorSize[1]
        );
    }

    if (m_lastBlendMode != hmesh->m_blendMode) {
        BlendModeDesc &selectedBlendMode = blendModes[(char)hmesh->m_blendMode];
        auto &lastBlendMode = blendModes[(char)m_lastBlendMode];
        if (lastBlendMode.blendModeEnable != selectedBlendMode.blendModeEnable ) {
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
        ((GOcclusionQueryGL33 *)gm2Mesh->m_query.get())->beginConditionalRendering();
    }

    glDrawElements(hmesh->m_element, hmesh->m_end, GL_UNSIGNED_SHORT, (const void *) (hmesh->m_start ));

    if (gm2Mesh != nullptr && gm2Mesh->m_query != nullptr) {
        ((GOcclusionQueryGL33 *)gm2Mesh->m_query.get())->endConditionalRendering();
    }

    if (gOcclusionQuery != nullptr) {
        gOcclusionQuery->endQuery();
    }
}

HGVertexBuffer GDeviceGL33::createVertexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GVertexBufferGL33> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferGL33(*this));

    return h_vertexBuffer;
}
HGVertexBufferDynamic GDeviceGL33::createVertexBufferDynamic(size_t size) {
    std::shared_ptr<GVertexBufferDynamicGL33> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferDynamicGL33(*this, size));

    return h_vertexBuffer;
};

HGIndexBuffer GDeviceGL33::createIndexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GIndexBufferGL33> h_indexBuffer;
    h_indexBuffer.reset(new GIndexBufferGL33(*this));

    return h_indexBuffer;
}

HGVertexBufferBindings GDeviceGL33::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindingsGL33> h_vertexBufferBindings;
    h_vertexBufferBindings.reset(new GVertexBufferBindingsGL33(*this));

    return h_vertexBufferBindings;
}

HGUniformBufferChunk GDeviceGL33::createUniformBufferChunk(size_t size) {
    HGUniformBufferChunk h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferChunk33(size));

    return h_uniformBuffer;
};

HGMesh GDeviceGL33::createMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GMeshGL33> h_mesh = std::make_shared<GMeshGL33>(*this, meshTemplate);

    return h_mesh;
}

HGM2Mesh GDeviceGL33::createM2Mesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2MeshGL33> h_mesh = std::make_shared<GM2MeshGL33>(*this, meshTemplate);

    return h_mesh;
}

HGParticleMesh GDeviceGL33::createParticleMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GParticleMeshGL33> h_mesh;
    h_mesh.reset(new GParticleMeshGL33(*this, meshTemplate));

    return h_mesh;
}

void GDeviceGL33::bindTexture(ITexture *iTexture, int slot) {
    GTextureGL33 * texture = (GTextureGL33 *) iTexture;
    if (texture == nullptr) {
        auto blackPixel = ((GTextureGL33 *) m_blackPixelTexture.get());
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

HGTexture GDeviceGL33::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
    BlpCacheRecord blpCacheRecord;
    blpCacheRecord.texture = texture.get();
    blpCacheRecord.wrapX = xWrapTex;
    blpCacheRecord.wrapY = yWrapTex;

    auto i = loadedTextureCache[blpCacheRecord];
    if (!i.expired()) {
        return i.lock();
    } else {
//        loadedTextureCache.erase(blpCacheRecord);
    }


    std::shared_ptr<GBlpTextureGL33> hgTexture;
    hgTexture.reset(new GBlpTextureGL33(*this, texture, xWrapTex, yWrapTex));

    std::weak_ptr<GTextureGL33> weakPtr(hgTexture);
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGTexture GDeviceGL33::createTexture() {
    std::shared_ptr<GTextureGL33> hgTexture;
    hgTexture.reset(new GTextureGL33(*this));
    return hgTexture;
}

void GDeviceGL33::bindProgram(IShaderPermutation *iProgram) {
    GShaderPermutationGL33 *program = (GShaderPermutationGL33 *)iProgram;

    if (program == nullptr) {
        if (m_shaderPermutation != nullptr) {
            m_shaderPermutation->unbindProgram();
            m_shaderPermutation = nullptr;
        }
        m_vertexBufferBindings = nullptr;
    } else if (program != m_shaderPermutation) {
        program->bindProgram();
        m_shaderPermutation = program;

        m_vertexBufferBindings = nullptr;
        m_UniformBuffer[0] = {};
        m_UniformBuffer[1] = {};
        m_UniformBuffer[2] = {};
        m_UniformBuffer[3] = {};
        m_UniformBuffer[4] = {};
    }
}

GDeviceGL33::GDeviceGL33() {
    unsigned int ff = 0xFFFFFFFF;
    unsigned int zero = 0;
    m_blackPixelTexture = createTexture();
    m_blackPixelTexture->loadData(1,1,&zero);

    m_whitePixelTexture = createTexture();
    m_whitePixelTexture->loadData(1,1,&ff);

    m_defaultVao = this->createVertexBufferBindings();

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffsetAlign);
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

HGOcclusionQuery GDeviceGL33::createQuery(HGMesh boundingBoxMesh) {
    std::shared_ptr<GOcclusionQueryGL33> hgOcclusionQuery;
    hgOcclusionQuery.reset(new GOcclusionQueryGL33(*this, boundingBoxMesh));

    return hgOcclusionQuery;
}

void GDeviceGL33::reset() {
    m_triCCW = -1;
    m_lastDepthWrite = -1;
    m_lastDepthCulling = -1;
    m_backFaceCulling = -1;
    m_isScissorsEnabled = -1;
    m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    m_lastBindIndexBuffer = nullptr;
    m_lastBindVertexBuffer = nullptr;
    m_vertexBufferBindings = nullptr;

    m_lastTexture[0] = nullptr;
    m_lastTexture[1] = nullptr;
    m_lastTexture[2] = nullptr;
    m_lastTexture[3] = nullptr;


    for (int i = 0; i < 5; i++) {
        bindUniformBuffer(nullptr, i, 0, 0);
    }

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

unsigned int GDeviceGL33::getUpdateFrameNumber() {
    return (m_frameNumber + 1) & 3;
//    return 0;
}
unsigned int GDeviceGL33::getCullingFrameNumber() {
    return (m_frameNumber + 3) & 3;
//    return 0;
}
unsigned int GDeviceGL33::getDrawFrameNumber() {
    return m_frameNumber & 3;
}

void GDeviceGL33::increaseFrameNumber() {
    m_frameNumber++;
}

HGVertexBufferBindings GDeviceGL33::getBBLinearBinding() {
    return m_lineBBBindings;
}

HGVertexBufferBindings GDeviceGL33::getBBVertexBinding() {
    return m_vertexBBBindings;
}

HGPUFence GDeviceGL33::createFence() {
    return nullptr;
}

void GDeviceGL33::uploadTextureForMeshes(std::vector<HGMesh> &meshes) {
    std::vector<HGTexture> textures;
    textures.reserve(meshes.size() * 3);

    int texturesLoaded = 0;

    for (const auto &hmesh : meshes) {
        GMeshGL33 * mesh = (GMeshGL33 *) hmesh.get();
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
#include "../androidLogSupport.h"
#endif

std::string GDeviceGL33::loadShader(std::string fileName, IShaderType shaderType) {
    std::string fullPath;
    trim(fileName);
    fullPath = "./glsl/glsl3.3/" + fileName;
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
    std::string filename = "glsl/" + shaderName + ".glsl";

    std::cout << "AAssetManager_open" << std::endl;
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

    return std::string(outBuf.begin(), outBuf.end());
#else
    std::ifstream t(fullPath);

    std::string result = std::string((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    //Delete version
    {
        auto start = result.find("#version");
        if (start != std::string::npos) {
            auto end = result.find("\n");
            result = result.substr(end);
        }
    }

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
#endif
}

float GDeviceGL33::getAnisLevel() {
    return m_anisotropicLevel;
}

void GDeviceGL33::clearScreen() {
#ifndef WITH_GLESv2
    glClearDepthf(1.0f);
#else
    glClearDepthf(1.0f);
#endif
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
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

void GDeviceGL33::setClearScreenColor(float r, float g, float b) {
    clearColor[0] = r;
    clearColor[1] = g;
    clearColor[2] = b;
}

void GDeviceGL33::beginFrame() {
    this->clearScreen();
}

void GDeviceGL33::commitFrame() {
    //Release resources
    while ((!listOfDeallocators.empty())&&(listOfDeallocators.front().frameNumberToDoAt <= m_frameNumber)) {
        listOfDeallocators.front().callback();

        listOfDeallocators.pop_front();
    }
}

void GDeviceGL33::setViewPortDimensions(float x, float y, float width, float height) {
    glViewport(x,y,width,height);
}

void GDeviceGL33::shrinkData()  {
    for (int i=0; i < 4; i++) {
        m_UBOFrames[i] = {};
    }

    aggregationBufferForUpload = {};
}


