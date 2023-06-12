#include "IDevice.h"

static const bool SortMeshes(const HGSortableMesh &indexA, const HGSortableMesh &indexB) {
    ISortableMesh * pA = indexA.get();
    ISortableMesh * pB = indexB.get();

    if (pA == nullptr) return false;
    if (pB == nullptr) return true;

    if (pA->getIsTransparent() && pB->getIsTransparent()) {
        if (pA->priorityPlane() != pB->priorityPlane()) {
            return pB->priorityPlane() > pA->priorityPlane();
        }

        if (pA->getSortDistance() < pB->getSortDistance()) {
            return true;
        }
        if (pA->getSortDistance() > pB->getSortDistance()) {
            return false;
        }

        if (pA->bindings() == pB->bindings() && pA->getMeshType() == pB->getMeshType() && pB->getMeshType() == MeshType::eM2Mesh) {
            IM2Mesh *pA2 = dynamic_cast<IM2Mesh *>(pA);
            IM2Mesh *pB2 = dynamic_cast<IM2Mesh *>(pB);

            if (pB2->layer() != pA2->layer()) {
                return pB2->layer() > pA2->layer();
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