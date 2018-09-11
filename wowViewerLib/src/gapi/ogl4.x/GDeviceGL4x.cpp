//
// Created by Deamon on 9/3/2018.
//

#include "GDeviceGL4x.h"
#include "GOcclusionQueryGL33.h"
#include <algorithm>

void GDeviceGL4x::drawMeshes(std::vector<HGMesh> &meshes) {
    updateBuffers(meshes);

    int j = 0;
    for (auto &hgMesh : meshes) {
        this->drawMesh(hgMesh);
        j++;
    }
}


void GDeviceGL4x::updateBuffers(std::vector<HGMesh> &iMeshes) {
    aggregationBufferForUpload.resize(maxUniformBufferSize);

    std::vector<HGLMeshGL4x> &meshes = (std::vector<HGLMeshGL4x> &) iMeshes;

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

    std::sort( buffers.begin(), buffers.end(), [](const GUniformBufferGL4x *a, const GUniformBufferGL4x * b) -> bool {
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
            ((GUniformBufferGL4x *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], currentSize);

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

        buffer->pIdentifierBuffer = ((GUniformBufferGL4x *) bufferForUpload.get())->pIdentifierBuffer;
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
        ((GUniformBufferGL4x *) bufferForUpload.get())->uploadData(&aggregationBufferForUpload[0], currentSize);
    }
    buffersIndex++;
    currentSize = 0;

//    std::cout << "m_unfiormBuffersForUpload.size = " << m_unfiormBuffersForUpload.size() << std::endl;

}

void GDeviceGL4x::drawMesh(HGMesh &hIMesh) {
    auto * hmesh = (GMeshGL4x *) hIMesh.get();
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

    glDrawElements(hmesh->m_element, hmesh->m_end, GL_UNSIGNED_SHORT, (const void *) (hmesh->m_start ));

    if (gm2Mesh != nullptr && gm2Mesh->m_query != nullptr) {
        ((GOcclusionQueryGL4x *)gm2Mesh->m_query.get())->endConditionalRendering();
    }

    if (gOcclusionQuery != nullptr) {
        gOcclusionQuery->endQuery();
    }
}