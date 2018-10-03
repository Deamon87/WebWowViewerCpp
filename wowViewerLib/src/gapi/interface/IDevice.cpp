//
// Created by Deamon on 8/25/2018.
//

#include <iostream>
#include "IDevice.h"


bool IDevice::sortMeshes(const HGMesh &a, const HGMesh &b) {
    if (a->getIsTransparent() > b-> getIsTransparent()) {
        return false;
    }
    if (a->getIsTransparent() < b->getIsTransparent()) {
        return true;
    }

    if (a->m_renderOrder != b->m_renderOrder ) {
        if (!a->getIsTransparent()) {
            return a->m_renderOrder < b->m_renderOrder;
        } else {
            return a->m_renderOrder > b->m_renderOrder;
        }
    }

    if (a->getMeshType() > b->getMeshType()) {
        return false;
    }
    if (a->getMeshType() < b->getMeshType()) {
        return true;
    }

    if (a->getMeshType() == MeshType::eM2Mesh && a->getIsTransparent() && b->getIsTransparent()) {
        HGM2Mesh a1 = std::static_pointer_cast<IMesh>(a);
        HGM2Mesh b1 = std::static_pointer_cast<IMesh>(b);
        if (a1->m_priorityPlane != b1->m_priorityPlane) {
            return b1->m_priorityPlane > a1->m_priorityPlane;
        }

        if (a1->m_sortDistance > b1->m_sortDistance) {
            return true;
        }
        if (a1->m_sortDistance < b1->m_sortDistance) {
            return false;
        }

        if (a1->m_m2Object > b1->m_m2Object) {
            return true;
        }
        if (a1->m_m2Object < b1->m_m2Object) {
            return false;
        }

        if (b1->m_layer != a1->m_layer) {
            return b1->m_layer < a1->m_layer;
        }
    }

    if (a->m_bindings != b->m_bindings) {
        return a->m_bindings > b->m_bindings;
    }

    if (a->getGxBlendMode() != b->getGxBlendMode()) {
        return a->getGxBlendMode() < b->getGxBlendMode();
    }

    int minTextureCount = std::min(a->m_textureCount, b->m_textureCount);
    for (int i = 0; i < minTextureCount; i++) {
        if (a->m_texture[i] != b->m_texture[i]) {
            return a->m_texture[i] < b->m_texture[i];
        }
    }

    if (a->m_textureCount != b->m_textureCount) {
        return a->m_textureCount < b->m_textureCount;
    }

    if (a->m_start != b->m_start) {
        return a->m_start < b->m_start;
    }
    if (a->m_end != b->m_end) {
        return a->m_end < b->m_end;
    }


    return a > b;
}

int compressedTexturesSupported = -1;
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

bool IDevice::getIsCompressedTexturesSupported() {
#ifdef __EMSCRIPTEN__
    if (compressedTexturesSupported == -1){
        if (emscripten_webgl_enable_extension(emscripten_webgl_get_current_context(), "WEBGL_compressed_texture_s3tc") == EM_TRUE) {
            compressedTexturesSupported = 1;
        } else {
            compressedTexturesSupported = 0;
        }
    }
      return compressedTexturesSupported == 1;

#else
    return true;
#endif
}
