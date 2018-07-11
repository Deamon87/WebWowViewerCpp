//
// Created by deamon on 05.06.18.
//

#include "../engine/opengl/header.h"
#include "GDevice.h"
#include "../engine/algorithms/hashString.h"
#include "shaders/GM2ShaderPermutation.h"
#include "../engine/opengl/header.h"

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

void GDevice::bindIndexBuffer(GIndexBuffer *buffer) {
    if (buffer == nullptr ) {
        if (m_lastBindIndexBuffer == nullptr) {
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

void GDevice::bindUniformBuffer(GUniformBuffer *buffer, int slot)  {
    if (buffer == nullptr) {
        if (m_uniformBuffer[slot] != nullptr) {
            m_uniformBuffer[slot]->unbind();
            m_uniformBuffer[slot] = nullptr;
        }
    }  else if (buffer != m_uniformBuffer[slot]) {
        buffer->bind();
        m_uniformBuffer[slot] = buffer;
    }
}

void GDevice::bindVertexBufferBindings(GVertexBufferBindings *buffer) {
    if (buffer == nullptr) {
       if (m_vertexBufferBindings != nullptr) {
           m_vertexBufferBindings->unbind();
       }
        m_lastBindIndexBuffer = nullptr;
        m_lastBindVertexBuffer = nullptr;
    } else {
        bindIndexBuffer(nullptr);
        bindVertexBuffer(nullptr);

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
        sharedPtr = std::make_shared<GShaderPermutation>(
            GM2ShaderPermutation(shaderName, *this));
    } else {
        sharedPtr = std::make_shared<GShaderPermutation>(
            GShaderPermutation(shaderName, *this));
    }
    m_shaderPermutCache[hash] = sharedPtr;

    return m_shaderPermutCache[hash];
}

HGUniformBuffer GDevice::createUniformBuffer(size_t size) {
    std::shared_ptr<GUniformBuffer> h_uniformBuffer;
    h_uniformBuffer.reset(new GUniformBuffer(*this, size));

    std::weak_ptr<GUniformBuffer> w_uniformBuffer = h_uniformBuffer;

    m_unfiormBufferCache.push_back(w_uniformBuffer);

    return h_uniformBuffer;
}

void GDevice::drawMeshes(std::vector<GMesh *> &meshes) {

}

HGVertexBuffer GDevice::createVertexBuffer() {
    std::shared_ptr<GVertexBuffer> h_vertexBuffer;
    h_vertexBuffer.reset(new GVertexBuffer(*this));

    return h_vertexBuffer;
}

HGIndexBuffer GDevice::createIndexBuffer() {
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

void GDevice::bindTexture(GTexture *texture, int slot) {
    if (texture == nullptr) {
        if (m_lastTexture[slot] != nullptr) {
            glActiveTexture(GL_TEXTURE0 + slot);
            m_lastTexture[slot]->unbind();
            m_uniformBuffer[slot] = nullptr;
        }
    }  else if (texture != m_lastTexture[slot]) {
        glActiveTexture(GL_TEXTURE0 + slot);
        texture->bind();
        m_lastTexture[slot] = texture;
    }
}

HGTexture GDevice::createTexture(HBlpTexture &texture) {
   GTexture gTexture = GTexture(*this, texture);
}
