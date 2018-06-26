//
// Created by deamon on 11.01.18.
//

#ifndef WEBWOWVIEWERCPP_WDLOBJECT_H
#define WEBWOWVIEWERCPP_WDLOBJECT_H

#include <vector>
#include <set>
#include "mathfu/glsl_mappings.h"
#include "../m2/m2Object.h"
#include "../../persistance/wdlFile.h"

class WdlObject {

public:
    WdlObject(IWoWInnerApi *api, std::string &wdlFileName);

    void setMapApi(IMapApi *api) {
        m_mapApi = api;
    }

    std::vector<M2Object*> m2Objects;
    std::vector<WmoObject*> wmoObjects;

private:
    IWoWInnerApi *m_api;
    IMapApi *m_mapApi;

    HWdlFile m_wdlFile;
    bool m_loaded = false;

    void loadingFinished();
    void loadM2s();
    void loadWmos();
public:
    bool getIsLoaded() {
        return m_loaded;
    }

    bool checkFrustumCulling(mathfu::vec4 &cameraPos,
                                        std::vector<mathfu::vec4> &frustumPlanes,
                                        std::vector<mathfu::vec3> &frustumPoints,
                                        std::vector<mathfu::vec3> &hullLines,
                                        mathfu::mat4 &lookAtMat4,
                                        std::vector<M2Object *> &m2ObjectsCandidates,
                                        std::vector<WmoObject *> &wmoCandidates);
};


#endif //WEBWOWVIEWERCPP_WDLOBJECT_H
