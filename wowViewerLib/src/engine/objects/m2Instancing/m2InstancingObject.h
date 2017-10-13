//
// Created by Deamon on 10/11/2017.
//

#ifndef WEBWOWVIEWERCPP_M2INSTANCINGOBJECT_H
#define WEBWOWVIEWERCPP_M2INSTANCINGOBJECT_H


#include "../../wowInnerApi.h"

class M2InstancingObject {

public:
    M2InstancingObject(IWoWInnerApi *sceneApi) {
        createPlacementVBO();
    }
private:
    IWoWInnerApi *sceneApi = nullptr;
    std::vector<M2Object *> m_mdxObjectList;
    std::set<M2Object *> m_mdxObjectSet;
    int lastUpdatedNumber = 0;
    int maxAmountWritten = 0;

    GLuint placementVBO;
public:
    void clearList() {
        this->m_mdxObjectList = {};
        this->m_mdxObjectSet = std::set<M2Object *>();
    }
    void addMDXObject(M2Object *m2Object) {
        // The object has already been added to this manager
        if (this->m_mdxObjectSet.find(m2Object) != m_mdxObjectSet.end()) return;

        this->m_mdxObjectSet.insert(m2Object);
        this->m_mdxObjectList.push_back(m2Object);
    }
    void createPlacementVBO() {
        glCreateBuffers(1, &placementVBO);
    }
    void updatePlacementVBO() {
        GLuint paramsVbo = this->placementVBO;

        int written = 0;
        std::vector<float> permanentBuffer = {};

        //1. Collect objects
        if (this->m_mdxObjectList.size() > 0) {;
            for (int i = 0; i < this->m_mdxObjectList.size(); i++) {
                M2Object* mdxObject = this->m_mdxObjectList[i];

                mathfu::mat4 &placementMatrix = mdxObject->m_placementMatrix;
                float *diffuseColor;
                if (mdxObject->m_useLocalDiffuseColor) {
                    diffuseColor = &mdxObject->m_localDiffuseColorV[0];
                } else {
                    static float diffuse_default[4] = {0xff, 0xff, 0xff, 0xff};
                    diffuseColor = diffuse_default;
                }
                for (int j = 0; j < 16; j++) {
                    permanentBuffer[written * 20 + j] = placementMatrix[j];
                }
                for (int j = 0; j < 4; j++) {
                    permanentBuffer[written * 20 + 16 + j] = diffuseColor[j];
                }

                written++;
            }

            if (written > 0) {
                glBindBuffer(GL_ARRAY_BUFFER, paramsVbo);
                if (written > this->maxAmountWritten) {
                    glBufferData(GL_ARRAY_BUFFER, permanentBuffer.size()* sizeof(float), &permanentBuffer[0], GL_DYNAMIC_DRAW);

                    this->maxAmountWritten = written;
                } else {
                    glBufferSubData(GL_ARRAY_BUFFER, 0, permanentBuffer.size()* sizeof(float), &permanentBuffer[0]);
                }
            }
        }
        this->placementVBO = paramsVbo;
        this->lastUpdatedNumber = written;
    }
    void drawInstancedNonTransparentMeshes(std::set<M2Object *> &opaqueMap) {
            if (this->m_mdxObjectList.size() <= 0) return;

            M2Object * lastDrawn = nullptr;
            for (int i = 0; i < this->m_mdxObjectList.size(); i++) {
                opaqueMap.insert(this->m_mdxObjectList[i]);
                lastDrawn = this->m_mdxObjectList[i];
            }

            if (lastDrawn != nullptr) {
                lastDrawn->drawInstanced(false, this->lastUpdatedNumber, this->placementVBO);
            }
    }
    void drawInstancedTransparentMeshes(std::set<M2Object *> &transparentMap) {
        if (this->m_mdxObjectList.size() <= 0) return;

        M2Object * lastDrawn = nullptr;
        for (int i = 0; i < this->m_mdxObjectList.size(); i++) {
            transparentMap.insert(this->m_mdxObjectList[i]);
            lastDrawn = this->m_mdxObjectList[i];
        }
        if (lastDrawn != nullptr) {
            lastDrawn->drawInstanced(true, this->lastUpdatedNumber, this->placementVBO);
        }
    }

    int getLastUpdatedNumber() {
        return lastUpdatedNumber;
    }
};


#endif //WEBWOWVIEWERCPP_M2INSTANCINGOBJECT_H
