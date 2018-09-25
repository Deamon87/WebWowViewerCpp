//
// Created by deamon on 05.06.18.
//
#include <iostream>
#include <algorithm>
#include "../../engine/opengl/header.h"
#include "GDeviceGL4x.h"
#include "../../engine/algorithms/hashString.h"
#include "shaders/GM2ShaderPermutationGL4x.h"
#include "meshes/GM2MeshGL4x.h"
#include "GOcclusionQueryGL4x.h"
#include "meshes/GParticleMeshGL4x.h"
#include "shaders/GM2ParticleShaderPermutationGL4x.h"
#include "shaders/GAdtShaderPermutationGL4x.h"
#include "shaders/GWMOShaderPermutationGL4x.h"
#include "syncronization/GPUFenceGL44.h"

typedef std::shared_ptr<GUniformBufferGL4x> HGLUniformBuffer;
typedef std::shared_ptr<GMeshGL4x> HGLMesh;


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

//void debug_func(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* /*userParam*/) {
//    fprintf(stdout, "source: %u, type: %u, id: %u, severity: %u, msg: %s\n",
//            source,
//            type,
//            id,
//            severity,
//            std::string(message, message+length).c_str());
//    fflush(stdout);
//}

void GDeviceGL4x::bindIndexBuffer(IIndexBuffer *buffer) {
    GIndexBufferGL4x * gBuffer = (GIndexBufferGL4x *) buffer;
    if (gBuffer == nullptr ) {
        if (m_lastBindIndexBuffer != nullptr) {
            ((GIndexBufferGL4x * )m_lastBindIndexBuffer)->unbind();
            m_lastBindIndexBuffer = nullptr;
        }
    } else if (gBuffer != m_lastBindIndexBuffer) {
        gBuffer->bind();
        m_lastBindIndexBuffer = gBuffer;
    }
}

void GDeviceGL4x::bindVertexBuffer(IVertexBuffer *buffer)  {
    GVertexBufferGL4x *gbuffer = (GVertexBufferGL4x *) buffer;

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

void GDeviceGL4x::bindVertexUniformBuffer(IUniformBuffer *buffer, int slot)  {
    GUniformBufferGL4x *gbuffer = (GUniformBufferGL4x *) buffer;

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
void GDeviceGL4x::bindFragmentUniformBuffer(IUniformBuffer *buffer, int slot)  {
    GUniformBufferGL4x *gbuffer = (GUniformBufferGL4x *) buffer;
    if (buffer == nullptr) {
        if (m_fragmentUniformBuffer[slot] != nullptr) {
            ((GUniformBufferGL4x *)m_fragmentUniformBuffer[slot])->unbind();
            m_fragmentUniformBuffer[slot] = nullptr;
        }
    }  else if (gbuffer != m_fragmentUniformBuffer[slot]) {
        gbuffer->bind(3+slot);

        m_fragmentUniformBuffer[slot] = gbuffer;
    }
}


void GDeviceGL4x::bindVertexBufferBindings(IVertexBufferBindings *buffer) {
    GVertexBufferBindingsGL4x *gbuffer = ((GVertexBufferBindingsGL4x *) buffer);

    if (buffer == nullptr) {
       if (m_vertexBufferBindings != nullptr) {
//           m_vertexBufferBindings->unbind();
           ( (GVertexBufferBindingsGL4x *)m_defaultVao.get())->bind();
           m_vertexBufferBindings = (GVertexBufferBindingsGL4x *)(m_defaultVao.get());
       }
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
    } else {
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;

        if (gbuffer != m_vertexBufferBindings) {
            gbuffer->bind();
            m_vertexBufferBindings = (GVertexBufferBindingsGL4x *) buffer;
        }
    }
}

std::shared_ptr<IShaderPermutation> GDeviceGL4x::getShader(std::string shaderName) {
    const char * cstr = shaderName.c_str();
    size_t hash = CalculateFNV(cstr);
    if (m_shaderPermutCache.count(hash) > 0) {

        HGShaderPermutation ptr = m_shaderPermutCache.at(hash);
        return ptr;
    }

    std::shared_ptr<IShaderPermutation> sharedPtr;
    IShaderPermutation *iPremutation = nullptr;
    if (shaderName == "m2Shader") {
        iPremutation = new GM2ShaderPermutationGL4x(shaderName, this);
        m_m2ShaderCreated = true;
    } else if (shaderName == "m2ParticleShader") {
        iPremutation = new GM2ParticleShaderPermutationGL4x(shaderName, this);
    } else if (shaderName == "wmoShader") {
        iPremutation = new GWMOShaderPermutationGL4x(shaderName, this);
    } else if (shaderName == "adtShader") {
        iPremutation = new GAdtShaderPermutationGL4x(shaderName, this);
    } else {
        iPremutation = new GShaderPermutationGL4x(shaderName, this);
    }

    sharedPtr.reset(iPremutation);

    GShaderPermutationGL4x * gShaderPermutation = (GShaderPermutationGL4x *)sharedPtr.get();

    gShaderPermutation->compileShader();
    m_shaderPermutCache[hash] = sharedPtr;

    glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboVertexBlockIndex[0], 0);
    glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboVertexBlockIndex[1], 1);
    glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboVertexBlockIndex[2], 2);

    glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboFragmentBlockIndex[0], 3+0);
    glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboFragmentBlockIndex[1], 3+1);
    glUniformBlockBinding(gShaderPermutation->m_programBuffer, gShaderPermutation->m_uboFragmentBlockIndex[2], 3+2);

    return m_shaderPermutCache[hash];
}

HGUniformBuffer GDeviceGL4x::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBufferGL4x> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBufferGL4x(*this, size));

    h_uniformBuffer->m_creationIndex = uniformBuffersCreated++;
    std::weak_ptr<GUniformBufferGL4x> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);

    return h_uniformBuffer;
}

void GDeviceGL4x::drawMeshes(std::vector<HGMesh> &meshes) {
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

//    m_uniformUploadFence->setGpuFence();
}

void GDeviceGL4x::updateBuffers(std::vector<HGMesh> &iMeshes) {
    aggregationBufferForUpload.resize(maxUniformBufferSize);

    std::vector<HGLMesh> &meshes = (std::vector<HGLMesh> &) iMeshes;

    //1. Collect buffers
    std::vector<GUniformBufferGL4x *> buffers;
    int renderIndex = 0;
    for (const auto &mesh : meshes) {
        for (int i = 0; i < 3; i++ ) {
            GUniformBufferGL4x *buffer = (GUniformBufferGL4x *) mesh->m_fragmentUniformBuffer[i].get();
            if (buffer != nullptr) {
                buffers.push_back(buffer);
                buffer->m_creationIndex = renderIndex++;
            }
        }
        for (int i = 0; i < 3; i++ ) {
            GUniformBufferGL4x * buffer = (GUniformBufferGL4x *)mesh->m_vertexUniformBuffer[i].get();
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

    std::vector<HGUniformBuffer> *m_unfiormBuffersForUpload = &m_UBOFrames[(getFrameNumber() + 1) & 3].m_uniformBuffersForUpload;
    HGUniformBuffer bufferForUpload = m_unfiormBuffersForUpload->at(buffersIndex);

    for (const auto &buffer : buffers) {
        if (buffer->m_buffCreated) continue;

        if ((currentSize + buffer->m_size) > maxUniformBufferSize) {
            ((GUniformBufferGL4x *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], currentSize);

            buffersIndex++;
            currentSize = 0;

            if (buffersIndex >= m_unfiormBuffersForUpload->size()) {
                assert("ALLLLERRRT");
            } else {
                bufferForUpload = m_unfiormBuffersForUpload->at(buffersIndex);
            }
        }

        buffer->setIdentifierBuffer(((GUniformBufferGL4x *) bufferForUpload.get())->getIdentifierBuffer(), currentSize);
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
            int bytesToAdd = uniformBufferOffsetAlign - (currentSize % uniformBufferOffsetAlign);

            currentSize += bytesToAdd;
        }
    }

	if (aggregationBufferForUpload.size() > 0) {
        ((GUniformBufferGL4x *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], currentSize);
	}


    //FillIndirectBuffer
    int frameNum = m_frameNumber % 3;
    DrawElementsIndirectCommand * pointerToWrite =
        &((DrawElementsIndirectCommand *) m_indirectBufferPointer)[20000*frameNum];

    int commandsWritten = 0;
    for (auto &hgMesh : meshes) {
        GMeshGL4x * hmesh = (GMeshGL4x *) hgMesh.get();
        DrawElementsIndirectCommand * command = &pointerToWrite[commandsWritten];
        command->firstIndex = hmesh->m_start >>1;
        command->count = hmesh->m_end;

        command->baseInstance = 0;
        command->baseVertex = 0;

        command->primCount = 1;

        hmesh->setInDirectPointer((void *)((20000*frameNum + commandsWritten) * sizeof(DrawElementsIndirectCommand)));
        commandsWritten++;
    }
}

void GDeviceGL4x::drawMesh(HGMesh &hIMesh) {
    GMeshGL4x * hmesh = (GMeshGL4x *) hIMesh.get();
    if (hmesh->m_end <= 0) return;

    GOcclusionQueryGL4x * gOcclusionQuery = nullptr;
    GM2MeshGL4x * gm2Mesh = nullptr;
    if (hmesh->m_meshType == MeshType::eOccludingQuery) {
        gOcclusionQuery = (GOcclusionQueryGL4x *)(hmesh);
    }
    if (hmesh->m_meshType == MeshType::eM2Mesh) {
        gm2Mesh = (GM2MeshGL4x *)(hmesh);
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
        ((GOcclusionQueryGL4x *)gm2Mesh->m_query.get())->beginConditionalRendering();
    }

    glDrawElementsIndirect(hmesh->m_element, GL_UNSIGNED_SHORT, hmesh->getIndirectPointer());
//    glDrawElements(hmesh->m_element, hmesh->m_end, GL_UNSIGNED_SHORT, (const void *) (hmesh->m_start ));

    if (gm2Mesh != nullptr && gm2Mesh->m_query != nullptr) {
        ((GOcclusionQueryGL4x *)gm2Mesh->m_query.get())->endConditionalRendering();
    }

    if (gOcclusionQuery != nullptr) {
        gOcclusionQuery->endQuery();
    }
}

HGVertexBuffer GDeviceGL4x::createVertexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GVertexBufferGL4x> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBufferGL4x(*this));

    return h_vertexBuffer;
}

HGIndexBuffer GDeviceGL4x::createIndexBuffer() {
    bindVertexBufferBindings(nullptr);
    std::shared_ptr<GIndexBufferGL4x> h_indexBuffer;
    h_indexBuffer.reset(new GIndexBufferGL4x(*this));

    return h_indexBuffer;
}

HGVertexBufferBindings GDeviceGL4x::createVertexBufferBindings() {
    std::shared_ptr<GVertexBufferBindingsGL4x> h_vertexBufferBindings;
    h_vertexBufferBindings.reset(new GVertexBufferBindingsGL4x(*this));

    return h_vertexBufferBindings;
}

HGMesh GDeviceGL4x::createMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GMeshGL4x> h_mesh;
    h_mesh.reset(new GMeshGL4x(*this, meshTemplate));

    return h_mesh;
}

HGM2Mesh GDeviceGL4x::createM2Mesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GM2MeshGL4x> h_mesh;
    h_mesh.reset(new GM2MeshGL4x(*this, meshTemplate));

    return h_mesh;
}

HGParticleMesh GDeviceGL4x::createParticleMesh(gMeshTemplate &meshTemplate) {
    std::shared_ptr<GParticleMeshGL4x> h_mesh;
    h_mesh.reset(new GParticleMeshGL4x(*this, meshTemplate));

    return h_mesh;
}

void GDeviceGL4x::bindTexture(ITexture *iTexture, int slot) {
    GTextureGL4x * texture = (GTextureGL4x *) iTexture;
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

HGTexture GDeviceGL4x::createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) {
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

    std::shared_ptr<GBlpTextureGL4x> hgTexture;
    hgTexture.reset(new GBlpTextureGL4x(*this, texture, xWrapTex, yWrapTex));

    std::weak_ptr<GTextureGL4x> weakPtr(hgTexture);
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGTexture GDeviceGL4x::createTexture() {
    std::shared_ptr<GTextureGL4x> hgTexture;
    hgTexture.reset(new GTextureGL4x(*this));
    return hgTexture;
}

void GDeviceGL4x::bindProgram(IShaderPermutation *iProgram) {
    GShaderPermutationGL4x *program = (GShaderPermutationGL4x *)iProgram;

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

GDeviceGL4x::GDeviceGL4x() {
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

    m_uniformUploadFence = createFence();

    //Create indirectBuffer pointer
    GLuint indirectBuffer;

    int indirectBufferSize = sizeof(DrawElementsIndirectCommand) * 20000 * 3;
    GLbitfield flags = GL_MAP_WRITE_BIT           |
                       GL_MAP_PERSISTENT_BIT |
                       GL_MAP_COHERENT_BIT;
    GLbitfield createFlags = flags | GL_DYNAMIC_STORAGE_BIT;
    glGenBuffers(1, &indirectBuffer);
    glBindBuffer( GL_DRAW_INDIRECT_BUFFER, indirectBuffer );
    glBufferStorage( GL_DRAW_INDIRECT_BUFFER, indirectBufferSize, nullptr, createFlags );


    m_indirectBufferPointer = glMapBufferRange(
        GL_DRAW_INDIRECT_BUFFER,
        0,
        indirectBufferSize,
        flags );


    for (int i = 0; i < 4; i++) {
        std::vector<HGUniformBuffer> *m_unfiormBuffersForUpload = &m_UBOFrames[i].m_uniformBuffersForUpload;

        for (int j = 0; j < 100; j++) {
            HGUniformBuffer bufferForUpload;
            bufferForUpload = createUniformBuffer(maxUniformBufferSize);
            bufferForUpload->createBuffer();
            m_unfiormBuffersForUpload->push_back(bufferForUpload);
        }
    }
}

HGOcclusionQuery GDeviceGL4x::createQuery(HGMesh boundingBoxMesh) {
    std::shared_ptr<GOcclusionQueryGL4x> hgOcclusionQuery;
    hgOcclusionQuery.reset(new GOcclusionQueryGL4x(*this, boundingBoxMesh));

    return hgOcclusionQuery;
}

void GDeviceGL4x::reset() {
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

    m_vertexUniformBuffer[0] = nullptr;
    m_vertexUniformBuffer[1] = nullptr;
    m_vertexUniformBuffer[2] = nullptr;

    m_fragmentUniformBuffer[0] = nullptr;
    m_fragmentUniformBuffer[1] = nullptr;
    m_fragmentUniformBuffer[2] = nullptr;

    m_shaderPermutation = nullptr;
}

int GDeviceGL4x::getFrameNumber() {
    return m_frameNumber;
}

void GDeviceGL4x::increaseFrameNumber() {
    m_frameNumber++;
}

HGVertexBufferBindings GDeviceGL4x::getBBLinearBinding() {
    return m_lineBBBindings;
}

HGVertexBufferBindings GDeviceGL4x::getBBVertexBinding() {
    return m_vertexBBBindings;
}

HGPUFence GDeviceGL4x::createFence() {
    HGPUFence hGPUFence;
    hGPUFence.reset(new GPUFenceGL44());

    return hGPUFence;
}

