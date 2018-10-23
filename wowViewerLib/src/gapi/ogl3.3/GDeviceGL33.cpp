//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include <algorithm>
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
    fprintf(stdout, "source: %u, type: %u, id: %u, severity: %u, msg: %s\n",
            source,
            type,
            id,
            severity,
            std::string(message, message+length).c_str());
    if (severity == 37190) {
        std::cout << "lol";
    }

    fflush(stdout);
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
    GVertexBufferGL33 *gbuffer = (GVertexBufferGL33 *) buffer;

    if (buffer == nullptr) {
        if (m_lastBindVertexBuffer != nullptr) {
            m_lastBindVertexBuffer->unbind();
            m_lastBindVertexBuffer = nullptr;
        }
    }  else if (buffer != m_lastBindVertexBuffer) {
        gbuffer->bind();
        m_lastBindVertexBuffer = gbuffer;
    }
}

void GDeviceGL33::bindVertexUniformBuffer(IUniformBuffer *buffer, int slot)  {
    GUniformBufferGL33 *gbuffer = (GUniformBufferGL33 *) buffer;

    if (buffer == nullptr) {
        if (m_vertexUniformBuffer[slot] != nullptr) {
            m_vertexUniformBuffer[slot]->unbind();
            m_vertexUniformBuffer[slot] = nullptr;
        }
    }  else {
        if (slot == -1) {
            gbuffer->bind(slot);
            m_vertexUniformBuffer[0] = nullptr;
        } else if (buffer != m_vertexUniformBuffer[slot]) {
            gbuffer->bind(slot);

            m_vertexUniformBuffer[slot] = gbuffer;
        }
    }
}
void GDeviceGL33::bindFragmentUniformBuffer(IUniformBuffer *buffer, int slot)  {
    GUniformBufferGL33 *gbuffer = (GUniformBufferGL33 *) buffer;
    if (buffer == nullptr) {
        if (m_fragmentUniformBuffer[slot] != nullptr) {
            ((GUniformBufferGL33 *)m_fragmentUniformBuffer[slot])->unbind();
            m_fragmentUniformBuffer[slot] = nullptr;
        }
    }  else if (gbuffer != m_fragmentUniformBuffer[slot]) {
        gbuffer->bind(3+slot);

        m_fragmentUniformBuffer[slot] = gbuffer;
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

//    glUseProgram(gShaderPermutation->m_programBuffer);
    for (int i = 0; i < 3; i++) {
        if (gShaderPermutation->m_uboVertexBlockIndex[i] > -1) {
            glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboVertexBlockIndex[i], i);
        }
        if (gShaderPermutation->m_uboFragmentBlockIndex[i] > -1) {
            glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboFragmentBlockIndex[i], 3 + i);
        }
    }
//    glUseProgram(0);

    return sharedPtr;
}

HGUniformBuffer GDeviceGL33::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBufferGL33> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferGL33(*this, size));

    h_uniformBuffer->m_creationIndex = uniformBuffersCreated++;
    std::weak_ptr<GUniformBufferGL33> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);

    return h_uniformBuffer;
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
    for (auto &hgMesh : meshes) {
        this->drawMesh(hgMesh);
        j++;
    }
}

void GDeviceGL33::updateBuffers(std::vector<HGMesh> &iMeshes) {
    aggregationBufferForUpload.resize(maxUniformBufferSize);

    std::vector<HGLMesh> &meshes = (std::vector<HGLMesh> &) iMeshes;

    //1. Collect buffers
    std::vector<GUniformBufferGL33 *> buffers;
    int renderIndex = 0;
    for (const auto &mesh : meshes) {
        for (int i = 0; i < 3; i++ ) {
            GUniformBufferGL33 *buffer = (GUniformBufferGL33 *) mesh->m_fragmentUniformBuffer[i].get();
            if (buffer != nullptr) {
                buffers.push_back(buffer);
                buffer->m_creationIndex = renderIndex++;
            }
        }
        for (int i = 0; i < 3; i++ ) {
            GUniformBufferGL33 * buffer = (GUniformBufferGL33 *)mesh->m_vertexUniformBuffer[i].get();
            if (buffer != nullptr) {
                buffers.push_back(buffer);
                buffer->m_creationIndex = renderIndex++;
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

    for (const auto &buffer : buffers) {
        if (buffer->m_buffCreated) continue;

        if ((currentSize + buffer->m_size) > maxUniformBufferSize) {
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

        buffer->setIdentifierBuffer(((GUniformBufferGL33 *) bufferForUpload.get())->getIdentifierBuffer(), currentSize);
        void * dataPtr = buffer->getPointerForUpload();
        std::copy((char*)dataPtr,
                  ((char*)dataPtr)+buffer->m_size,
                  &aggregationBufferForUpload[currentSize]);
//        aggregationBufferForUpload.insert(
//            aggregationBufferForUpload.end(),
//            (char*)buffer->pContent,
//            ((char*)buffer->pContent)+buffer->m_size
//        );
        currentSize += buffer->m_size;

        int offsetDiff = currentSize % uniformBufferOffsetAlign;
        if (offsetDiff != 0) {
            int bytesToAdd = uniformBufferOffsetAlign - offsetDiff;

            currentSize += bytesToAdd;
        }
    }

    if (currentSize > 0) {
        ((GUniformBufferGL33 *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], maxUniformBufferSize);
    }
}

void GDeviceGL33::drawMesh(HGMesh &hIMesh) {
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

    bindProgram(hmesh->m_shader.get());
    bindVertexBufferBindings(hmesh->m_bindings.get());

    bindVertexUniformBuffer(hmesh->m_vertexUniformBuffer[0].get(), 0);
    bindVertexUniformBuffer(hmesh->m_vertexUniformBuffer[1].get(), 1);
    bindVertexUniformBuffer(hmesh->m_vertexUniformBuffer[2].get(), 2);

    bindFragmentUniformBuffer(hmesh->m_fragmentUniformBuffer[0].get(), 0);
    bindFragmentUniformBuffer(hmesh->m_fragmentUniformBuffer[1].get(), 1);
    bindFragmentUniformBuffer(hmesh->m_fragmentUniformBuffer[2].get(), 2);

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
        if (blendModes[(char)m_lastBlendMode].blendModeEnable != selectedBlendMode.blendModeEnable ) {
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

HGMesh GDeviceGL33::createMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GMeshGL33> h_mesh;
    h_mesh.reset(new GMeshGL33(*this, meshTemplate));

    return h_mesh;
}

HGM2Mesh GDeviceGL33::createM2Mesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2MeshGL33> h_mesh;
    h_mesh.reset(new GM2MeshGL33(*this, meshTemplate));

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

//        m_vertexBufferBindings = nullptr;
//        m_vertexUniformBuffer[0] = nullptr;
//        m_vertexUniformBuffer[1] = nullptr;
//        m_vertexUniformBuffer[2] = nullptr;
//        m_fragmentUniformBuffer[0] = nullptr;
//        m_fragmentUniformBuffer[1] = nullptr;
//        m_fragmentUniformBuffer[2] = nullptr;
    }
}

GDeviceGL33::GDeviceGL33() {
    unsigned int ff = 0;
    m_blackPixelTexture = createTexture();

    m_blackPixelTexture->loadData(1,1,&ff);

    m_defaultVao = this->createVertexBufferBindings();

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffsetAlign);

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

    GBufferBinding bufferBinding = {0, 3, GL_FLOAT, false, 0, 0 };

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


//    m_defaultVao->save();

    aggregationBufferForUpload = std::vector<char>(maxUniformBufferSize);

    std::cout << "maxUniformBufferSize = " << maxUniformBufferSize << std::endl;

//    glEnable(GL_DEBUG_OUTPUT);
//    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
//    glDebugMessageCallback(debug_func, NULL);
//
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
    m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    m_lastBindIndexBuffer = nullptr;
    m_lastBindVertexBuffer = nullptr;
    m_vertexBufferBindings = nullptr;

    m_lastTexture[0] = nullptr;
    m_lastTexture[1] = nullptr;
    m_lastTexture[2] = nullptr;
    m_lastTexture[3] = nullptr;

    bindFragmentUniformBuffer(nullptr, 0);
    bindFragmentUniformBuffer(nullptr, 1);
    bindFragmentUniformBuffer(nullptr, 2);

    bindVertexUniformBuffer(nullptr, 0);
    bindVertexUniformBuffer(nullptr, 1);
    bindVertexUniformBuffer(nullptr, 2);

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

