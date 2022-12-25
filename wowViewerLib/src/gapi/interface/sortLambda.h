[](const HGMesh &indexA, const HGMesh &indexB) {
    IMesh * pA = indexA.get();
    IMesh* pB = indexB.get();

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
            if (pA->priorityPlane()!= pB->priorityPlane()) {
                return pB->priorityPlane() > pA->priorityPlane();
            }

            if (pA->getSortDistance() < pB->getSortDistance()) {
                return true;
            }
            if (pA->getSortDistance() > pB->getSortDistance()) {
                return false;
            }

            if (pA->getM2Object() == pB->getM2Object()) {
                if (pB->layer() != pA->layer()) {
                    return pB->layer() < pA->layer();
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

    if (pA->getGxBlendMode() != pB->getGxBlendMode()) {
        return pA->getGxBlendMode() < pB->getGxBlendMode();
    }

    int minTextureCount = pA->textureCount() < pB->textureCount() ? pA->textureCount() : pB->textureCount();
    for (int i = 0; i < minTextureCount; i++) {
        if (pA->texture()[i] != pB->texture()[i]) {
            return pA->texture()[i] < pB->texture()[i];
        }
    }

    if (pA->textureCount() != pB->textureCount()) {
        return pA->textureCount() < pB->textureCount();
    }

    if (pA->start() != pB->start()) {
        return pA->start() < pB->start();
    }
    if (pA->end() != pB->end()) {
        return pA->end() < pB->end();
    }


    return pA > pB;
}