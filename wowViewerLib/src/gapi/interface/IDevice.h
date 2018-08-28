//
// Created by Deamon on 8/25/2018.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICE_H
#define AWEBWOWVIEWERCPP_IDEVICE_H

#include "IVertexBufferBindings.h"
#include "meshes/IMesh.h"

class IDevice {
    public:
        virtual void reset() = 0;
        virtual bool getIsEvenFrame() = 0;

        virtual void toogleEvenFrame() = 0;

        virtual void bindProgram(GShaderPermutation *program) = 0;

        virtual void bindVertexBuffer(GVertexBuffer *buffer) = 0;
        virtual void bindVertexUniformBuffer(GUniformBuffer *buffer, int slot) = 0;
        virtual void bindFragmentUniformBuffer(GUniformBuffer *buffer, int slot) = 0;
        virtual void bindVertexBufferBindings(GVertexBufferBindings *buffer) = 0;

        virtual void bindTexture(GTexture *texture, int slot) = 0;

        virtual void updateBuffers(std::vector<HGMesh> &meshes)= 0 ;
        virtual void drawMeshes(std::vector<HGMesh> &meshes) = 0;
    public:
        std::shared_ptr<GShaderPermutation> getShader(std::string shaderName) = 0;

        virtual HGUniformBuffer createUniformBuffer(size_t size) = 0;
        virtual HGVertexBuffer createVertexBuffer() = 0;
        virtual HGIndexBuffer createIndexBuffer() = 0;
        virtual HGVertexBufferBindings createVertexBufferBindings() = 0;

        virtual HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) = 0;
        virtual HGTexture createTexture() = 0;
        virtual HGMesh createMesh(gMeshTemplate &meshTemplate) = 0;
        virtual HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) = 0;
        virtual HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) = 0;

        virtual HGOcclusionQuery createQuery(HGMesh boundingBoxMesh) = 0;

        virtual static bool sortMeshes(const HGMesh& a, const HGMesh& b) = 0;
        virtual HGVertexBufferBindings getBBVertexBinding() = 0;
        virtual HGVertexBufferBindings getBBLinearBinding() = 0;
};


#endif //AWEBWOWVIEWERCPP_IDEVICE_H
