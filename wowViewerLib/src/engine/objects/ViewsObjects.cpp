//
// Created by deamon on 22.10.19.
//

#include "ViewsObjects.h"
#include "../../gapi/UniformBufferStructures.h"
#include "oneapi/tbb/parallel_for.h"

#if (__AVX__ && __SSE2__)
#include "../algorithms/mathHelper_culling_sse.h"
#endif
#include "../algorithms/mathHelper_culling.h"

void ExteriorView::collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, COpaqueMeshCollector &opaqueMeshCollector, framebased::vector<HGSortableMesh> &transparentMeshes) {
    GeneralView::collectMeshes(renderADT, renderAdtLiquid, renderWMO, opaqueMeshCollector, transparentMeshes);
}


void GeneralView::collectMeshes(bool renderADT, bool renderAdtLiquid, bool renderWMO, COpaqueMeshCollector &opaqueMeshCollector, framebased::vector<HGSortableMesh> &transparentMeshes) {
    if (renderWMO) {
        for (auto &wmoGroup: wmoGroupArray.getToDraw()) {
            wmoGroup->collectMeshes(opaqueMeshCollector, transparentMeshes, renderOrder);
        }
    }
}
void GeneralView::collectLights(std::vector<LocalLight> &pointLights, std::vector<SpotLight> &spotLights, std::vector<std::shared_ptr<CWmoNewLight>> &newWmoLights) {
    for (auto &wmoGroup: wmoGroupArray.getToDraw()) {
        auto &wmoPointLights = wmoGroup->getPointLights();

        pointLights.reserve(pointLights.size() + wmoPointLights.size());

        for (auto &pointLight : wmoPointLights)
            pointLights.push_back(pointLight.getLightRec());

        auto &wmoNewLights = wmoGroup->getWmoNewLights();
        newWmoLights.reserve(newWmoLights.size() + wmoNewLights.size());
        for (auto &newLight : wmoNewLights) {
            newWmoLights.push_back(newLight);
        }
    }
};


void GeneralView::addM2FromGroups(const MathHelper::FrustumCullingData &frustumData, mathfu::vec4 &cameraPos) {
    for (auto &wmoGroup : wmoGroupArray.getToCheckM2()) {
        auto &doodads = wmoGroup->getDoodads();
        for (auto &m2object : doodads) {
            this->m2List.addCandidate(m2object);
        }
    }

    auto &candidatesArr = this->m2List.getCandidates();
    auto candCullRes = std::vector<uint32_t>(candidatesArr.size(), 0xFFFFFFFF);

    oneapi::tbb::task_arena arena(oneapi::tbb::task_arena::automatic, 3);
    arena.execute([&] {

        oneapi::tbb::parallel_for(tbb::blocked_range<size_t>(0, candCullRes.size(), 1000),
                          [&](tbb::blocked_range<size_t> r) {
    //for (int i = 0; i < candidatesArr.size(); i++) {
#if (__AVX__ && __SSE2__)
            ObjectCullingSEE<M2ObjId>::cull(this->frustumData, r.begin(),
                                                                           r.end(), candidatesArr,
                                                                           candCullRes);
#else
            ObjectCulling<M2ObjId>::cull(this->frustumData,
                                                              r.begin(), r.end(), candidatesArr,
                                                              candCullRes);
#endif
        }, tbb::auto_partitioner());
    });

    for (int i = 0; i < candCullRes.size(); i++) {
        if (!candCullRes[i]) {
            const auto m2ObjectCandidate = m2Factory->getObjectById<0>(candidatesArr[i]);
            if (m2ObjectCandidate != nullptr)
                this->m2List.addToDraw(m2ObjectCandidate);
        }
    }
}

void GeneralView::produceTransformedPortalMeshes(const HMapSceneBufferCreate &sceneRenderer, const HApiContainer &apiContainer,
                                                 const std::vector<std::vector<mathfu::vec3>> &portalsVerts, bool isAntiportal) {
    std::vector<uint16_t> indiciesArray;
    std::vector<float> verticles;
    int k = 0;
    //int l = 0;
    std::vector<int> stripOffsets;
    stripOffsets.push_back(0);
    int verticleOffset = 0;
    int stripOffset = 0;
    for (int i = 0; i < portalsVerts.size(); i++) {
        //if (portalInfo.index_count != 4) throw new Error("portalInfo.index_count != 4");

        int verticlesCount = portalsVerts[i].size();
        if ((verticlesCount - 2) <= 0) {
            stripOffsets.push_back(stripOffsets[i]);
            continue;
        };

        if (!isAntiportal) {
            for (int j = 0; j < (((int) verticlesCount) - 2); j++) {
                indiciesArray.push_back(verticleOffset + 0);
                indiciesArray.push_back(verticleOffset + j + 1);
                indiciesArray.push_back(verticleOffset + j + 2);
            }
        } else {
            for (int j = 0; j < (verticlesCount); j++) {
                indiciesArray.push_back(verticleOffset + j);
            }
        }
        stripOffset += ((verticlesCount-2) * 3);

        for (int j =0; j < verticlesCount; j++) {
            verticles.push_back(portalsVerts[i][j].x);
            verticles.push_back(portalsVerts[i][j].y);
            verticles.push_back(portalsVerts[i][j].z);
        }

        verticleOffset += verticlesCount;
        stripOffsets.push_back(stripOffset);
    }

    if (verticles.empty()) return;

    auto &portalPointsFrame = portals.emplace_back();
    portalPointsFrame.m_indexVBO = sceneRenderer->createPortalIndexBuffer((indiciesArray.size() * sizeof(uint16_t)));
    portalPointsFrame.m_bufferVBO = sceneRenderer->createPortalVertexBuffer((verticles.size() * sizeof(float)));

    portalPointsFrame.m_indexVBO->uploadData((void *) indiciesArray.data(), (int) (indiciesArray.size() * sizeof(uint16_t)));
    portalPointsFrame.m_bufferVBO->uploadData((void *) verticles.data(), (int) (verticles.size() * sizeof(float)));

    portalPointsFrame.m_bindings = sceneRenderer->createPortalVAO(portalPointsFrame.m_bufferVBO, portalPointsFrame.m_indexVBO);

    {
        PipelineTemplate pipelineTemplate;

        pipelineTemplate.element = DrawElementMode::TRIANGLES;
        pipelineTemplate.depthWrite = false;
        pipelineTemplate.depthCulling = !apiContainer->getConfig()->renderPortalsIgnoreDepth;
        pipelineTemplate.backFaceCulling = false;

        pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

        auto material = sceneRenderer->createPortalMaterial(pipelineTemplate);
        auto &portalColor = material->m_materialPS->getObject();
        if (!isAntiportal) {
            portalColor.uColor = {0.058, 0.058, 0.819607843, 0.3};
        } else {
            portalColor.uColor = {0.819607843, 0.058, 0.058, 0.3};
        }
        material->m_materialPS->save();

        //Create mesh
        gMeshTemplate meshTemplate(portalPointsFrame.m_bindings);
        meshTemplate.start = 0;
        meshTemplate.end = indiciesArray.size();

        auto mesh = sceneRenderer->createSortableMesh(meshTemplate, material, 0);

        m_portalMeshes.push_back(mesh);
    }
}

void GeneralView::collectPortalMeshes(framebased::vector<HGSortableMesh> &transparentMeshes) {
    for (auto const &mesh : m_portalMeshes) {
        transparentMeshes.push_back(mesh);
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

HExteriorView FrameViewsHolder::getSkybox() {
    if (!skyBoxView) {
        skyBoxView = std::make_shared<ExteriorView>();
    }
    return skyBoxView;
}
