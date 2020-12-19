//
// Created by deamon on 22.10.19.
//

#include "ViewsObjects.h"
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
    for (auto &wmoGroup : drawnWmos) {
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

void InteriorView::setM2Lights(std::shared_ptr<M2Object> m2Object) {
    if (!drawnWmos[0]->getIsLoaded()) return;

    if (drawnWmos[0]->getDontUseLocalLightingForM2()) {
        m2Object->setUseLocalLighting(false);
    } else {
        drawnWmos[0]->assignInteriorParams(m2Object);
    }
}
