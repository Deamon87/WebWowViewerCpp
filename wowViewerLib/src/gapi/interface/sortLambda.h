#include "IDevice.h"

static const bool SortMeshes(const HGMesh &indexA, const HGMesh &indexB) {
    ITransparentMesh * pA = dynamic_cast<ITransparentMesh *>(indexA.get());
    ITransparentMesh * pB = dynamic_cast<ITransparentMesh *>(indexB.get());

//    HGMesh pA = sortedArrayPtr[indexA];
//    HGMesh pB = sortedArrayPtr[indexB];
    if (pA == nullptr) return false;
    if (pB == nullptr) return true;

//    if (pA->getMeshType() > pB->getMeshType()) {
//        return false;
//    }
//    if (pA->getMeshType() < pB->getMeshType()) {
//        return true;
//    }

//    if (pA->m_renderOrder != pB->m_renderOrder ) {
//        if (!pA->getIsTransparent()) {
//            return pA->m_renderOrder < pB->m_renderOrder;
//        } else {
//            return pA->m_renderOrder > pB->m_renderOrder;
//        }
//    }


    if (pA->getIsTransparent() && pB->getIsTransparent()) {
        if (((pA->getMeshType() == MeshType::eM2Mesh || pA->getMeshType() == MeshType::eParticleMesh) &&
            (pB->getMeshType() == MeshType::eM2Mesh || pB->getMeshType() == MeshType::eParticleMesh))) {
            IM2Mesh *pA1 = dynamic_cast<IM2Mesh *>(pA);
            IM2Mesh *pB1 = dynamic_cast<IM2Mesh *>(pB);

            if (pA1->priorityPlane()!= pB1->priorityPlane()) {
                return pB1->priorityPlane() > pA1->priorityPlane();
            }

            if (pA1->getSortDistance() < pB1->getSortDistance()) {
                return true;
            }
            if (pA1->getSortDistance() > pB1->getSortDistance()) {
                return false;
            }

//            if (pA1->getM2Object() == pB1->getM2Object()) {
            if (pA1->bindings() == pB1->bindings()) {
                if (pB1->layer() != pA1->layer()) {
                    return pB1->layer() < pA1->layer();
                }
            }
        } else {
            if (pA->getSortDistance() < pB->getSortDistance()) {
                return true;
            }
            if (pA->getSortDistance() > pB->getSortDistance()) {
                return false;
            }
        }
    }else {
        if (pA->getSortDistance() > pB->getSortDistance()) {
            return true;
        }
        if (pA->getSortDistance() < pB->getSortDistance()) {
            return false;
        }
    }

    if (pA->bindings() != pB->bindings()) {
        return pA->bindings() > pB->bindings();
    }

    if (pA->start() != pB->start()) {
        return pA->start() < pB->start();
    }
    if (pA->end() != pB->end()) {
        return pA->end() < pB->end();
    }


    return pA > pB;
}