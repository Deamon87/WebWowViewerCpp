//
// Created by deamon on 22.10.19.
//

#include "ViewsObjects.h"

void ExteriorView::collectMeshes(std::vector<HGMesh> &renderedThisFrame) {
    auto inserter = std::back_inserter(renderedThisFrame);
    std::copy(drawnChunks.begin(), drawnChunks.end(), inserter);

    GeneralView::collectMeshes(renderedThisFrame);
}


void GeneralView::collectMeshes(std::vector<HGMesh> &renderedThisFrame) {
    for (auto& wmoGroup : drawnWmos) {
        wmoGroup->collectMeshes(renderedThisFrame, renderOrder);
    }
//    for (auto& m2 : drawnM2s) {
//        m2->collectMeshes(renderedThisFrame, renderOrder);
//        m2->drawParticles(renderedThisFrame , renderOrder);
//    }
}

void GeneralView::addM2FromGroups(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) {
    std::vector<M2Object *> candidates;
    for (auto &wmoGroup : drawnWmos) {
        auto doodads = wmoGroup->getDoodads();
        std::copy(doodads->begin(), doodads->end(), std::back_inserter(candidates));
    }

    //Delete duplicates
    std::sort( candidates.begin(), candidates.end() );
    candidates.erase( unique( candidates.begin(), candidates.end() ), candidates.end() );

//    std::vector<bool> candidateResults = std::vector<bool>(candidates.size(), false);

    std::for_each(
        candidates.begin(),
        candidates.end(),
        [this, &cameraPos](M2Object * m2Candidate) {  // copies are safer, and the resulting code will be as quick.
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

void GeneralView::setM2Lights(M2Object *m2Object) {
    m2Object->setUseLocalLighting(false);
}

void InteriorView::setM2Lights(M2Object *m2Object) {
    if (!drawnWmos[0]->getIsLoaded()) return;

    if (drawnWmos[0]->getDontUseLocalLightingForM2()) {
        m2Object->setUseLocalLighting(false);
    } else {
        drawnWmos[0]->assignInteriorParams(m2Object);
    }
}
