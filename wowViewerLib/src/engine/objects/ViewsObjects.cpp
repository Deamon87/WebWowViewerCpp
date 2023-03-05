//
// Created by deamon on 22.10.19.
//

#include "ViewsObjects.h"
#include "../../gapi/UniformBufferStructures.h"
#include "../shader/ShaderDefinitions.h"
#include "oneapi/tbb/parallel_for.h"

#if (__AVX__ && __SSE2__)
#include "../algorithms/mathHelper_culling_sse.h"
#endif
#include "../algorithms/mathHelper_culling.h"
#include "../../gapi/interface/materials/IMaterial.h"

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
    for (auto& wmoGroup : wmoGroupArray.getToDraw()) {
        wmoGroup->collectMeshes(opaqueMeshes, transparentMeshes, renderOrder);
    }

//    for (auto& m2 : drawnM2s) {
//        m2->collectMeshes(renderedThisFrame, renderOrder);
//        m2->drawParticles(renderedThisFrame , renderOrder);
//    }
}

void GeneralView::addM2FromGroups(const MathHelper::FrustumCullingData &frustumData, mathfu::vec4 &cameraPos) {
    for (auto &wmoGroup : wmoGroupArray.getToCheckM2()) {
        auto &doodads = wmoGroup->getDoodads();
        for (auto &m2object : doodads) {
            this->m2List.addCandidate(m2object);
        }
    }

    auto candidatesArr = this->m2List.getCandidates();
    auto candCullRes = std::vector<uint32_t>(candidatesArr.size(), 0xFFFFFFFF);

    oneapi::tbb::parallel_for(tbb::blocked_range<size_t>(0, candCullRes.size(), 1000),
                      [&](tbb::blocked_range<size_t> r) {
//for (int i = 0; i < candidatesArr.size(); i++) {
#if (__AVX__ && __SSE2__)
        ObjectCullingSEE<std::shared_ptr<M2Object>>::cull(this->frustumData, r.begin(),
                                                                       r.end(), candidatesArr,
                                                                       candCullRes);
#else
        ObjectCulling<std::shared_ptr<M2Object>>::cull(this->frustumData,
                                                          r.begin(), r.end(), candidatesArr,
                                                          candCullRes);
#endif
    }, tbb::auto_partitioner());

    for (int i = 0; i < candCullRes.size(); i++) {
        if (!candCullRes[i]) {
            auto &m2ObjectCandidate = candidatesArr[i];
            setM2Lights(m2ObjectCandidate);
            this->m2List.addToDraw(m2ObjectCandidate);
        }
    }
}

void GeneralView::setM2Lights(std::shared_ptr<M2Object> &m2Object) {
    m2Object->setUseLocalLighting(false);
}

static std::array<GBufferBinding, 3> DrawPortalBindings = {
    {+drawPortalShader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, 12, 0 }, // 0
    //24
};

void GeneralView::produceTransformedPortalMeshes(HApiContainer &apiContainer, std::vector<HGMesh> &opaqueMeshes, std::vector<HGMesh> &transparentMeshes) {

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

    //TODO:
//    portalPointsFrame.m_indexVBO = hDevice->createIndexBuffer();
//    portalPointsFrame.m_bufferVBO = hDevice->createVertexBuffer();

    portalPointsFrame.m_bindings = hDevice->createVertexBufferBindings();
    portalPointsFrame.m_bindings->setIndexBuffer(portalPointsFrame.m_indexVBO);

    auto const drawPortalBindings = std::vector<GBufferBinding>(DrawPortalBindings.begin(), DrawPortalBindings.end());

    portalPointsFrame.m_bindings->addVertexBufferBinding(portalPointsFrame.m_bufferVBO, drawPortalBindings);
    portalPointsFrame.m_bindings->save();


    portalPointsFrame.m_indexVBO->uploadData((void *) indiciesArray.data(), (int) (indiciesArray.size() * sizeof(uint16_t)));
    portalPointsFrame.m_bufferVBO->uploadData((void *) verticles.data(), (int) (verticles.size() * sizeof(float)));

    {
        HGShaderPermutation shaderPermutation = hDevice->getShader("drawPortalShader", "drawPortalShader", nullptr);

        //Create mesh
        gMeshTemplate meshTemplate(portalPointsFrame.m_bindings);
        PipelineTemplate pipelineTemplate;

        pipelineTemplate.element = DrawElementMode::TRIANGLES;
        pipelineTemplate.depthWrite = false;
        pipelineTemplate.depthCulling = !apiContainer->getConfig()->renderPortalsIgnoreDepth;
        pipelineTemplate.backFaceCulling = false;

        pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

        meshTemplate.start = 0;
        meshTemplate.end = indiciesArray.size();

        std::shared_ptr<IBufferChunk<DrawPortalShader::meshWideBlockPS>> drawPortalShaderMeshWideBlockPS = nullptr;
        drawPortalShaderMeshWideBlockPS->setUpdateHandler([](auto &data, const HFrameDependantData &frameDepedantData) {
            auto& blockPS = data;

            blockPS.uColor = {0.058, 0.058, 0.819607843, 0.3};
        });


        transparentMeshes.push_back(hDevice->createMesh(meshTemplate));
    }
}

void InteriorView::setM2Lights(std::shared_ptr<M2Object> &m2Object) {
    if (ownerGroupWMO == nullptr || !ownerGroupWMO->getIsLoaded()) return;

    if (ownerGroupWMO->getDontUseLocalLightingForM2()) {
        m2Object->setUseLocalLighting(false);
    } else {
        ownerGroupWMO->assignInteriorParams(m2Object);
    }
}

HExteriorView FrameViewsHolder::getOrCreateExterior(const MathHelper::FrustumCullingData &frustumData) {
    if (exteriorView == nullptr) {
        exteriorView = std::make_shared<ExteriorView>();
        exteriorView->frustumData = frustumData;
    }

    return exteriorView;
}

HInteriorView FrameViewsHolder::createInterior(const MathHelper::FrustumCullingData &frustumData) {
    auto interior = std::make_shared<InteriorView>();
    interior->frustumData = frustumData;

    interiorViews.push_back(interior);

    return interior;
}

HExteriorView FrameViewsHolder::getExterior() {
    return exteriorView;
}
