//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include "../engine/opengl/header.h"
#include "GDevice.h"
#include "../engine/algorithms/hashString.h"
#include "shaders/GM2ShaderPermutation.h"
#include "meshes/GM2Mesh.h"
#include "GOcclusionQuery.h"
#include "meshes/GParticleMesh.h"
#include "shaders/GM2ParticleShaderPermutation.h"
#include "shaders/GAdtShaderPermutation.h"
#include "shaders/GWMOShaderPermutation.h"

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

__stdcall void debug_func(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* /*userParam*/) {
    fprintf(stdout, "source: %u, type: %u, id: %u, severity: %u, msg: %s\n",
            source,
            type,
            id,
            severity,
            std::string(message, message+length).c_str());
    fflush(stdout);
}

void GDevice::bindIndexBuffer(GIndexBuffer *buffer) {
    if (buffer == nullptr ) {
        if (m_lastBindIndexBuffer != nullptr) {
            m_lastBindIndexBuffer->unbind();
            m_lastBindIndexBuffer = nullptr;
        }
    } else if (buffer != m_lastBindIndexBuffer) {
        buffer->bind();
        m_lastBindIndexBuffer = buffer;
    }
}

void GDevice::bindVertexBuffer(GVertexBuffer *buffer)  {
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

void GDevice::bindVertexUniformBuffer(GUniformBuffer *buffer, int slot)  {
    if (buffer == nullptr) {
        if (m_vertexUniformBuffer[slot] != nullptr) {
            m_vertexUniformBuffer[slot]->unbind();
            m_vertexUniformBuffer[slot] = nullptr;
        }
    }  else {
        if (slot == -1) {
            buffer->bind(slot);
            m_vertexUniformBuffer[0] = nullptr;
        } else if (buffer != m_vertexUniformBuffer[slot]) {
            buffer->bind(slot);

            m_vertexUniformBuffer[slot] = buffer;
        }
    }
}
void GDevice::bindFragmentUniformBuffer(GUniformBuffer *buffer, int slot)  {
    if (buffer == nullptr) {
        if (m_fragmentUniformBuffer[slot] != nullptr) {
            m_fragmentUniformBuffer[slot]->unbind();
            m_fragmentUniformBuffer[slot] = nullptr;
        }
    }  else if (buffer != m_fragmentUniformBuffer[slot]) {
        buffer->bind(3+slot);

        m_fragmentUniformBuffer[slot] = buffer;
    }
}


void GDevice::bindVertexBufferBindings(GVertexBufferBindings *buffer) {
    if (buffer == nullptr) {
       if (m_vertexBufferBindings != nullptr) {
//           m_vertexBufferBindings->unbind();
           m_defaultVao->bind();
           m_vertexBufferBindings = m_defaultVao.get();
       }
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
    } else {
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;

        if (buffer != m_vertexBufferBindings) {
            buffer->bind();
            m_vertexBufferBindings = buffer;
        }
    }
}

std::shared_ptr<GShaderPermutation> GDevice::getShader(std::string shaderName) {
    const char * cstr = shaderName.c_str();
    size_t hash = CalculateFNV(cstr);
    if (m_shaderPermutCache.count(hash) > 0) {

        HGShaderPermutation ptr = m_shaderPermutCache.at(hash);
        return ptr;
    }

    std::shared_ptr<GShaderPermutation> sharedPtr;
    if (shaderName == "m2Shader") {
        sharedPtr.reset( new GM2ShaderPermutation(shaderName, *this));
        m_m2ShaderCreated = true;
    } else if (shaderName == "m2ParticleShader") {
        sharedPtr.reset( new GM2ParticleShaderPermutation(shaderName, *this));
    } else if (shaderName == "wmoShader") {
        sharedPtr.reset( new GWMOShaderPermutation(shaderName, *this));
    } else if (shaderName == "adtShader") {
        sharedPtr.reset( new GAdtShaderPermutation(shaderName, *this));
    } else {
        sharedPtr.reset(new GShaderPermutation(shaderName, *this));
    }

    sharedPtr->compileShader();
    m_shaderPermutCache[hash] = sharedPtr;

    glUniformBlockBinding(sharedPtr->m_programBuffer, sharedPtr->m_uboVertexBlockIndex[0], 0);
    glUniformBlockBinding(sharedPtr->m_programBuffer, sharedPtr->m_uboVertexBlockIndex[1], 1);
    glUniformBlockBinding(sharedPtr->m_programBuffer, sharedPtr->m_uboVertexBlockIndex[2], 2);

    glUniformBlockBinding(sharedPtr->m_programBuffer, sharedPtr->m_uboFragmentBlockIndex[0], 3+0);
    glUniformBlockBinding(sharedPtr->m_programBuffer, sharedPtr->m_uboFragmentBlockIndex[1], 3+1);
    glUniformBlockBinding(sharedPtr->m_programBuffer, sharedPtr->m_uboFragmentBlockIndex[2], 3+2);

    return m_shaderPermutCache[hash];
}

HGUniformBuffer GDevice::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBuffer> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBuffer(*this, size));

    h_uniformBuffer->m_creationIndex = uniformBuffersCreated++;
    std::weak_ptr<GUniformBuffer> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);

    return h_uniformBuffer;
}

void GDevice::drawMeshes(std::vector<HGMesh> &meshes) {
    updateBuffers(meshes);

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

void GDevice::updateBuffers(std::vector<HGMesh> &meshes) {
    aggregationBufferForUpload.resize(maxUniformBufferSize);

    //1. Collect buffers
    std::vector<HGUniformBuffer> buffers;
    int renderIndex = 0;
    for (const auto &mesh : meshes) {
        for (int i = 0; i < 3; i++ ) {
            HGUniformBuffer buffer = mesh->m_fragmentUniformBuffer[i];
            if (buffer != nullptr) {
                buffers.push_back(buffer);
                buffer->m_creationIndex = renderIndex++;
            }
        }
        for (int i = 0; i < 3; i++ ) {
            HGUniformBuffer buffer = mesh->m_vertexUniformBuffer[i];
            if (buffer != nullptr) {
                buffers.push_back(buffer);
                buffer->m_creationIndex = renderIndex++;
            }
        }
    }

    std::sort( buffers.begin(), buffers.end(), [](const HGUniformBuffer &a, const HGUniformBuffer &b) -> bool {
        return a->m_creationIndex > b->m_creationIndex;
    });
    buffers.erase( unique( buffers.begin(), buffers.end() ), buffers.end() );

    //2. Create buffers and update them
    int currentSize = 0;
    int buffersIndex = 0;

    std::vector<HGUniformBuffer> *m_unfiormBuffersForUpload = &m_firstUBOFrame.m_unfiormBuffersForUpload;
    if (getIsEvenFrame()) {
        m_unfiormBuffersForUpload = &m_secondUBOFrame.m_unfiormBuffersForUpload;
    }

    HGUniformBuffer bufferForUpload;
    if (buffersIndex >= m_unfiormBuffersForUpload->size()) {
        bufferForUpload = createUniformBuffer(0);
        bufferForUpload->createBuffer();
        m_unfiormBuffersForUpload->push_back(bufferForUpload);
    } else {
        bufferForUpload = m_unfiormBuffersForUpload->at(buffersIndex);
    }

    for (const auto &buffer : buffers) {
        if (buffer->m_buffCreated) continue;

        if ((currentSize + buffer->m_size) > maxUniformBufferSize) {
            bufferForUpload->uploadData(&aggregationBufferForUpload[0], currentSize);

            buffersIndex++;
            currentSize = 0;

            if (buffersIndex >= m_unfiormBuffersForUpload->size()) {
                bufferForUpload = createUniformBuffer(0);
                bufferForUpload->createBuffer();
                m_unfiormBuffersForUpload->push_back(bufferForUpload);
            } else {
                bufferForUpload = m_unfiormBuffersForUpload->at(buffersIndex);
            }
        }

        buffer->pIdentifierBuffer = bufferForUpload->pIdentifierBuffer;
        buffer->m_offset = (size_t) currentSize;
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

        int bytesToAdd = uniformBufferOffsetAlign - (currentSize % uniformBufferOffsetAlign);
//        for (int j = 0; j < bytesToAdd; j++) {
//            aggregationBufferForUpload.push_back(0);
//        }
        currentSize+=bytesToAdd;
    }

	if (aggregationBufferForUpload.size() > 0) {
		bufferForUpload->uploadData(&aggregationBufferForUpload[0], currentSize);
	}
    buffersIndex++;
    currentSize = 0;

//    std::cout << "m_unfiormBuffersForUpload.size = " << m_unfiormBuffersForUpload.size() << std::endl;

}

void GDevice::drawMesh(HGMesh &hmesh) {

    if (hmesh->m_end <= 0) return;

    GOcclusionQuery * gOcclusionQuery = nullptr;
    GM2Mesh * gm2Mesh = nullptr;
    if (hmesh->m_meshType == MeshType::eOccludingQuery) {
        gOcclusionQuery = (GOcclusionQuery *)(hmesh.get());
    }
    if (hmesh->m_meshType == MeshType::eM2Mesh) {
        gm2Mesh = (GM2Mesh *)(hmesh.get());
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
        gm2Mesh->m_query->beginConditionalRendering();
    }

    glDrawElements(hmesh->m_element, hmesh->m_end, GL_UNSIGNED_SHORT, (const void *) (hmesh->m_start ));

    if (gm2Mesh != nullptr && gm2Mesh->m_query != nullptr) {
        gm2Mesh->m_query->endConditionalRendering();
    }

    if (gOcclusionQuery != nullptr) {
        gOcclusionQuery->endQuery();
    }
}

HGVertexBuffer GDevice::createVertexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GVertexBuffer> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBuffer(*this));

    return h_vertexBuffer;
}

HGIndexBuffer GDevice::createIndexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GIndexBuffer> h_indexBuffer;
    h_indexBuffer.reset(new GIndexBuffer(*this));

    return h_indexBuffer;
}

HGVertexBufferBindings GDevice::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindings> h_vertexBufferBindings;
    h_vertexBufferBindings.reset(new GVertexBufferBindings(*this));

    return h_vertexBufferBindings;
}

HGMesh GDevice::createMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GMesh> h_mesh;
    h_mesh.reset(new GMesh(*this, meshTemplate));

    return h_mesh;
}

HGM2Mesh GDevice::createM2Mesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2Mesh> h_mesh;
    h_mesh.reset(new GM2Mesh(*this, meshTemplate));

    return h_mesh;
}

HGParticleMesh GDevice::createParticleMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GParticleMesh> h_mesh;
    h_mesh.reset(new GParticleMesh(*this, meshTemplate));

    return h_mesh;
}

void GDevice::bindTexture(GTexture *texture, int slot) {
    if (texture == nullptr) {
        if (m_lastTexture[slot] != nullptr) {
            glActiveTexture(GL_TEXTURE0 + slot);
            m_lastTexture[slot]->unbind();
            m_lastTexture[slot] = nullptr;
        }
    }  else if (texture != m_lastTexture[slot]) {
        glActiveTexture(GL_TEXTURE0 + slot);
        texture->bind();
        m_lastTexture[slot] = texture;
    }
}

HGTexture GDevice::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
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

    std::shared_ptr<GBlpTexture> hgTexture;
    hgTexture.reset(new GBlpTexture(*this, texture, xWrapTex, yWrapTex));

    std::weak_ptr<GTexture> weakPtr(hgTexture);
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGTexture GDevice::createTexture() {
    std::shared_ptr<GTexture> hgTexture;
    hgTexture.reset(new GTexture(*this));
    return hgTexture;
}

void GDevice::bindProgram(GShaderPermutation *program) {
    if (program == nullptr) {
        if (m_shaderPermutation != nullptr) {
            m_shaderPermutation->unbindProgram();
            m_shaderPermutation = nullptr;
        }
    } else if (program != m_shaderPermutation) {
        program->bindProgram();
        m_shaderPermutation = program;
    }
}

GDevice::GDevice() {
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

//    glEnable(GL_DEBUG_OUTPUT);
//    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
//    glDebugMessageCallback(debug_func, NULL);
//


}

bool GDevice::sortMeshes(const HGMesh &a, const HGMesh &b) {
    if (a->getIsTransparent() > b-> getIsTransparent()) {
        return false;
    }
    if (a->getIsTransparent() < b->getIsTransparent()) {
        return true;
    }

    if (a->m_renderOrder != b->m_renderOrder ) {
        if (!a->getIsTransparent()) {
            return a->m_renderOrder < b->m_renderOrder;
        } else {
            return a->m_renderOrder > b->m_renderOrder;
        }
    }

    if (a->getMeshType() > b->getMeshType()) {
        return false;
    }
    if (a->getMeshType() < b->getMeshType()) {
        return true;
    }

    if (a->getMeshType() == MeshType::eM2Mesh && a->getIsTransparent() && b->getIsTransparent()) {
        HGM2Mesh a1 = std::static_pointer_cast<GM2Mesh>(a);
        HGM2Mesh b1 = std::static_pointer_cast<GM2Mesh>(b);
        if (a1->m_priorityPlane != b1->m_priorityPlane) {
            return b1->m_priorityPlane > a1->m_priorityPlane;
        }

        if (a1->m_sortDistance > b1->m_sortDistance) {
            return true;
        }
        if (a1->m_sortDistance < b1->m_sortDistance) {
            return false;
        }

        if (a1->m_m2Object > b1->m_m2Object) {
            return true;
        }
        if (a1->m_m2Object < b1->m_m2Object) {
            return false;
        }

        if (b1->m_layer != a1->m_layer) {
            return b1->m_layer < a1->m_layer;
        }
    }

    if (a->m_bindings != b->m_bindings) {
        return a->m_bindings > b->m_bindings;
    }

    if (a->getGxBlendMode() != b->getGxBlendMode()) {
        return a->getGxBlendMode() < b->getGxBlendMode();
    }

    int minTextureCount = std::min(a->m_textureCount, b->m_textureCount);
    for (int i = 0; i < minTextureCount; i++) {
        if (a->m_texture[i] != b->m_texture[i]) {
            return a->m_texture[i] < b->m_texture[i];
        }
    }

    if (a->m_textureCount != b->m_textureCount) {
        return a->m_textureCount < b->m_textureCount;
    }

    if (a->m_start != b->m_start) {
        return a->m_start < b->m_start;
    }
    if (a->m_end != b->m_end) {
        return a->m_end < b->m_end;
    }


    return a > b;
}

HGOcclusionQuery GDevice::createQuery(HGMesh boundingBoxMesh) {
    std::shared_ptr<GOcclusionQuery> hgOcclusionQuery;
    hgOcclusionQuery.reset(new GOcclusionQuery(*this, boundingBoxMesh));

    return hgOcclusionQuery;
}

