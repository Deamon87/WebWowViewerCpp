//
// Created by deamon on 22.10.19.
//

#include "ViewsObjects.h"
#include "../../gapi/UniformBufferStructures.h"
#include "../shader/ShaderDefinitions.h"
#include <execution>

void ExteriorView::collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes) {
    {
        auto inserter = std::back_inserter(opaqueMeshes);
        std::copy(this->m_opaqueMeshes.begin(), this->m_opaqueMeshes.end(), inserter);
    }

    {
        auto inserter = std::back_inserter(transparentMeshes);
        std::copy(this->m_transparentMeshes.begin(), this->m_transparentMeshes.end(), inserter);
    }

    GeneralView::collectMeshes(opaqueMeshes, transparentMeshes);
}


void GeneralView::collectMeshes(std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes) {
    for (auto& wmoGroup : drawnWmos) {
        wmoGroup->collectMeshes(opaqueMeshes, transparentMeshes, renderOrder);
    }

//    for (auto& m2 : drawnM2s) {
//        m2->collectMeshes(renderedThisFrame, renderOrder);
//        m2->drawParticles(renderedThisFrame , renderOrder);
//    }
}

void GeneralView::addM2FromGroups(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    std::vector<std::shared_ptr<M2Object>> candidates;
    for (auto &wmoGroup : wmosForM2) {
        auto doodads = wmoGroup->getDoodads();
        std::copy(doodads->begin(), doodads->end(), std::back_inserter(candidates));
    }

    //Delete duplicates
#if (_LIBCPP_HAS_PARALLEL_ALGORITHMS)
    std::sort(std::execution::par_unseq, candidates.begin(), candidates.end() );
#else
    std::sort(candidates.begin(), candidates.end() );
#endif
    candidates.erase( unique( candidates.begin(), candidates.end() ), candidates.end() );

//    std::vector<bool> candidateResults = std::vector<bool>(candidates.size(), false);

    std::for_each(
        candidates.begin(),
        candidates.end(),
        [this, &cameraPos](std::shared_ptr<M2Object> m2Candidate) {  // copies are safer, and the resulting code will be as quick.
            if (m2Candidate == nullptr) return;
            for (auto &frustumPlane : this->frustumPlanes) {
                if (m2Candidate->checkFrustumCulling(cameraPos, frustumPlane, {})) {
                    setM2Lights(m2Candidate);

                    break;
                }
            }
        });

//    drawnM2s = std::vector<M2Object *>();
    drawnM2s.reserve(drawnM2s.size() + candidates.size());
    for (auto &m2Candidate : candidates) {
        if (m2Candidate == nullptr) continue;
        if (m2Candidate->m_cullResult) {
            drawnM2s.push_back(m2Candidate);
        }
    }
}

void GeneralView::setM2Lights(std::shared_ptr<M2Object> m2Object) {
    m2Object->setUseLocalLighting(false);
}

static std::array<GBufferBinding, 3> DrawPortalBindings = {
    {+drawPortalShader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, 12, 0 }, // 0
    //24
};

void GeneralView::produceTransformedPortalMeshes(HApiContainer apiContainer, std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes) {

    std::vector<uint16_t> indiciesArray;
    std::vector<float> verticles;
    int k = 0;
    //int l = 0;
    std::vector<int> stripOffsets;
    stripOffsets.push_back(0);
    int verticleOffset = 0;
    int stripOffset = 0;
    for (int i = 0; i < this->worldPortalVertices.size(); i++) {
        //if (portalInfo.index_count != 4) throw new Error("portalInfo.index_count != 4");

        int verticlesCount = this->worldPortalVertices[i].size();
        if ((verticlesCount - 2) <= 0) {
            stripOffsets.push_back(stripOffsets[i]);
            continue;
        };

        for (int j =0; j < (((int)verticlesCount)-2); j++) {
            indiciesArray.push_back(verticleOffset+0);
            indiciesArray.push_back(verticleOffset+j+1);
            indiciesArray.push_back(verticleOffset+j+2);
        }
        stripOffset += ((verticlesCount-2) * 3);

        for (int j =0; j < verticlesCount; j++) {
            verticles.push_back(this->worldPortalVertices[i][j].x);
            verticles.push_back(this->worldPortalVertices[i][j].y);
            verticles.push_back(this->worldPortalVertices[i][j].z);
        }

        verticleOffset += verticlesCount;
        stripOffsets.push_back(stripOffset);
    }

    if (verticles.empty()) return;

    auto hDevice = apiContainer->hDevice;

    portalPointsFrame.m_indexVBO = hDevice->createIndexBuffer();
    portalPointsFrame.m_bufferVBO = hDevice->createVertexBuffer();

    portalPointsFrame.m_bindings = hDevice->createVertexBufferBindings();
    portalPointsFrame.m_bindings->setIndexBuffer(portalPointsFrame.m_indexVBO);

    GVertexBufferBinding vertexBinding;
    vertexBinding.vertexBuffer = portalPointsFrame.m_bufferVBO;
    vertexBinding.bindings = std::vector<GBufferBinding>(DrawPortalBindings.begin(), DrawPortalBindings.end());

    portalPointsFrame.m_bindings->addVertexBufferBinding(vertexBinding);
    portalPointsFrame.m_bindings->save();


    portalPointsFrame.m_indexVBO->uploadData((void *) indiciesArray.data(), (int) (indiciesArray.size() * sizeof(uint16_t)));
    portalPointsFrame.m_bufferVBO->uploadData((void *) verticles.data(), (int) (verticles.size() * sizeof(float)));

    {
        HGShaderPermutation shaderPermutation = hDevice->getShader("drawPortalShader", nullptr);

        //Create mesh
        gMeshTemplate meshTemplate(portalPointsFrame.m_bindings, shaderPermutation);

        meshTemplate.depthWrite = false;
        meshTemplate.depthCulling = !apiContainer->getConfig()->renderPortalsIgnoreDepth;
        meshTemplate.backFaceCulling = false;

        meshTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

        //Let's assume ribbons are always at least transparent
        if (meshTemplate.blendMode == EGxBlendEnum::GxBlend_Opaque) {
            meshTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;
        }

        meshTemplate.start = 0;
        meshTemplate.end = indiciesArray.size();
        meshTemplate.element = DrawElementMode::TRIANGLES;

        meshTemplate.textureCount = 0;

        meshTemplate.ubo[0] = nullptr; //m_api->getSceneWideUniformBuffer();
        meshTemplate.ubo[1] = nullptr;
        meshTemplate.ubo[2] = nullptr;

        meshTemplate.ubo[3] = nullptr;
        meshTemplate.ubo[4] = hDevice->createUniformBufferChunk(sizeof(DrawPortalShader::meshWideBlockPS));

        meshTemplate.ubo[4]->setUpdateHandler([](IUniformBufferChunk *self, const HFrameDepedantData &frameDepedantData) {
            auto& blockPS = self->getObject<DrawPortalShader::meshWideBlockPS>();

            blockPS.uColor = {0.058, 0.058, 0.819607843, 0.3};
        });


        transparentMeshes.push_back(hDevice->createParticleMesh(meshTemplate));
    }
}

void InteriorView::setM2Lights(std::shared_ptr<M2Object> m2Object) {
    if (!drawnWmos[0]->getIsLoaded()) return;

    if (drawnWmos[0]->getDontUseLocalLightingForM2()) {
        m2Object->setUseLocalLighting(false);
    } else {
        drawnWmos[0]->assignInteriorParams(m2Object);
    }
}
