[sortedArrayPtr](int indexA, int indexB) {
    IMesh * pA = sortedArrayPtr[indexA].get();
    IMesh* pB = sortedArrayPtr[indexB].get();

//    HGMesh pA = sortedArrayPtr[indexA];
//    HGMesh pB = sortedArrayPtr[indexB];
    if (pA == nullptr) return false;
    if (pB == nullptr) return true;

    if (pA->getIsTransparent() > pB->getIsTransparent()) {
        return false;
    }
    if (pA->getIsTransparent() < pB->getIsTransparent()) {
        return true;
}

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

    if (pA->isSkyBox() > pB->isSkyBox()) {
        return true;
    }
    if (pA->isSkyBox() < pB->isSkyBox()) {
        return false;
    }

    if (((pA->getMeshType() == MeshType::eM2Mesh && pB->getMeshType() == MeshType::eM2Mesh) ||
        (pA->getMeshType() == MeshType::eParticleMesh && pB->getMeshType() == MeshType::eParticleMesh)) &&
        pA->getIsTransparent() && pB->getIsTransparent()) {
        if (pA->priorityPlane()!= pB->priorityPlane()) {
            return pB->priorityPlane() > pA->priorityPlane();
        }

        if (pA->getSortDistance() > pB->getSortDistance()) {
            return true;
        }
        if (pA->getSortDistance() < pB->getSortDistance()) {
            return false;
        }
    }

    if (pA->getSortDistance() > pB->getSortDistance()) {
        return true;
    }
    if (pA->getSortDistance() < pB->getSortDistance()) {
        return false;
    }

    if (pA->getMeshType() == MeshType::eM2Mesh && pB->getMeshType() == MeshType::eM2Mesh && pA->getIsTransparent() && pB->getIsTransparent()) {
        if (pB->layer() != pA->layer()) {
            return !(pB->layer() < pA->layer());
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