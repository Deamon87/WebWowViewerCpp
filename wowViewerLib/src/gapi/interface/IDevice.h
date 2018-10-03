//
// Created by Deamon on 8/25/2018.
//

#ifndef AWEBWOWVIEWERCPP_IDEVICE_H
#define AWEBWOWVIEWERCPP_IDEVICE_H

class IVertexBuffer;
class IVertexBufferBindings;
class IIndexBuffer;
class IUniformBuffer;
class IBlpTexture;
class ITexture;
class IShaderPermutation;
class IMesh;
class IM2Mesh;
class IOcclusionQuery;
class IParticleMesh;
class IGPUFence;
class gMeshTemplate;
#include <memory>
#include "syncronization/IGPUFence.h"

typedef std::shared_ptr<IVertexBuffer> HGVertexBuffer;
typedef std::shared_ptr<IIndexBuffer> HGIndexBuffer;
typedef std::shared_ptr<IVertexBufferBindings> HGVertexBufferBindings;
typedef std::shared_ptr<IUniformBuffer> HGUniformBuffer;
typedef std::shared_ptr<IShaderPermutation> HGShaderPermutation;
typedef std::shared_ptr<IMesh> HGMesh;
typedef std::shared_ptr<IMesh> HGM2Mesh;
typedef std::shared_ptr<IMesh> HGParticleMesh;
typedef std::shared_ptr<IMesh> HGOcclusionQuery;
typedef std::shared_ptr<IBlpTexture> HGBlpTexture;
typedef std::shared_ptr<ITexture> HGTexture;
typedef std::shared_ptr<IGPUFence> HGPUFence;

#include "meshes/IMesh.h"
#include "meshes/IM2Mesh.h"
#include "IOcclusionQuery.h"
#include "IShaderPermutation.h"
#include "buffers/IIndexBuffer.h"
#include "buffers/IVertexBuffer.h"
#include "IVertexBufferBindings.h"
#include "buffers/IUniformBuffer.h"
#include "textures/ITexture.h"
#include "../../engine/wowCommonClasses.h"

struct M2ShaderCacheRecord {
    int vertexShader;
    int pixelShader;
    bool unlit;
    bool alphaTestOn;
    bool unFogged;
    bool unShadowed;


    bool operator==(const M2ShaderCacheRecord &other) const {
        return
            (vertexShader == other.vertexShader) &&
            (pixelShader == other.pixelShader) &&
            (alphaTestOn == other.alphaTestOn) &&
            (unlit == other.unlit) &&
            (unFogged == other.unFogged) &&
            (unShadowed == other.unShadowed);
    };
};

class IDevice {
    public:
        virtual ~IDevice() {};

        virtual void reset() = 0;
        virtual int getFrameNumber() = 0;
        virtual bool getIsAsynBuffUploadSupported() = 0;

        virtual void increaseFrameNumber() = 0;

        virtual void bindProgram(IShaderPermutation *program) = 0;

        virtual void bindIndexBuffer(IIndexBuffer *buffer) = 0;
        virtual void bindVertexBuffer(IVertexBuffer *buffer) = 0;
        virtual void bindVertexUniformBuffer(IUniformBuffer *buffer, int slot) = 0;
        virtual void bindFragmentUniformBuffer(IUniformBuffer *buffer, int slot) = 0;
        virtual void bindVertexBufferBindings(IVertexBufferBindings *buffer) = 0;

        virtual void bindTexture(ITexture *texture, int slot) = 0;

        virtual void updateBuffers(std::vector<HGMesh> &meshes)= 0 ;
        virtual void drawMeshes(std::vector<HGMesh> &meshes) = 0;

        virtual bool getIsCompressedTexturesSupported();
    public:
        virtual HGShaderPermutation getShader(std::string shaderName, void *permutationDescriptor) = 0;

        virtual HGPUFence createFence() = 0;

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

        static bool sortMeshes(const HGMesh& a, const HGMesh& b);
        virtual HGVertexBufferBindings getBBVertexBinding() = 0;
        virtual HGVertexBufferBindings getBBLinearBinding() = 0;
};


#endif //AWEBWOWVIEWERCPP_IDEVICE_H
