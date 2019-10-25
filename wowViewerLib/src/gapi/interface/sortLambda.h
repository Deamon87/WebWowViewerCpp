[](const HGMesh& a, const HGMesh& b) {
    auto* pA = a.get();
    auto* pB = b.get();

    if (pA->getIsTransparent() > pB->getIsTransparent()) {
        return false;
    }
    if (pA->getIsTransparent() < pB->getIsTransparent()) {
        return true;
    }

    if (pA->getMeshType() > pB->getMeshType()) {
        return false;
    }
    if (pA->getMeshType() < pB->getMeshType()) {
        return true;
    }

    if (pA->renderOrder() != pB->renderOrder() ) {
        if (!pA->getIsTransparent()) {
            return pA->renderOrder() < pB->renderOrder();
        } else {
            return pA->renderOrder() > pB->renderOrder();
        }
    }

    if (pA->isSkyBox() > pB->isSkyBox()) {
        return true;
    }
    if (pA->isSkyBox() < pB->isSkyBox()) {
        return false;
    }

    if (pA->getMeshType() == MeshType::eM2Mesh && pA->getIsTransparent() && pB->getIsTransparent()) {
        if (pA->priorityPlane() != pB->priorityPlane()) {
            return pB->priorityPlane() > pA->priorityPlane();
        }

        if (pA->sortDistance() > pB->sortDistance()) {
            return true;
        }
        if (pA->sortDistance() < pB->sortDistance()) {
            return false;
        }

//        if (pA->m_m2Object > pB->m_m2Object) {
//            return true;
//        }
//        if (pA->m_m2Object < pB->m_m2Object) {
//            return false;
//        }

        if (pB->layer() != pA->layer()) {
            return pB->layer() < pA->layer();
        }
    }

    if (pA->getMeshType() == MeshType::eParticleMesh && pB->getMeshType() == MeshType::eParticleMesh) {
        if (pA->priorityPlane() != pB->priorityPlane()) {
            return pB->priorityPlane() > pA->priorityPlane();
        }

        if (pA->sortDistance() > pB->sortDistance()) {
            return true;
        }
        if (pA->sortDistance() < pB->sortDistance()) {
            return false;
        }
    }

    if (pA->bindings() != pB->bindings()) {
        return pA->bindings() > pB->bindings();
    }

    if (pA->getGxBlendMode() != pB->getGxBlendMode()) {
        return pA->getGxBlendMode() < pB->getGxBlendMode();
    }

    int minTextureCount = std::min(pA->textureCount(), pB->textureCount());
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


    return a > b;
}