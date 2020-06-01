//
// Created by deamon on 11.01.18.
//

#ifndef WEBWOWVIEWERCPP_WDLOBJECT_H
#define WEBWOWVIEWERCPP_WDLOBJECT_H

#include <vector>
#include <set>
#include "../iMapApi.h"
#include "mathfu/glsl_mappings.h"
#include "../wmo/wmoObject.h"
#include "../m2/m2Object.h"
#include "../../persistance/wdlFile.h"

class WdlObject {

public:
    explicit WdlObject(ApiContainer *api, std::string &wdlFileName);
    explicit WdlObject(ApiContainer *api, int wdlFileDataId);

    void setMapApi(IMapApi *api) {
        m_mapApi = api;
    }

    std::vector<std::shared_ptr<M2Object>> m2Objects;
    std::vector<std::shared_ptr<WmoObject>> wmoObjects;

private:
    ApiContainer *m_api;
    IMapApi *m_mapApi;

    HWdlFile m_wdlFile;
    bool m_loaded = false;

    void loadingFinished();
    void loadM2s();
    void loadWmos();

    struct SkyObjectCondition {
        int conditionType;
        int conditionValue;
    };
    struct SkyObjectScene{
        std::vector<std::shared_ptr<M2Object>> m2Objects = {};
        std::vector<SkyObjectCondition> conditions = {};
    };

    std::vector<SkyObjectScene> skyScenes;
public:
    bool getIsLoaded() {
        return m_loaded;
    }

    bool checkFrustumCulling(mathfu::vec4 &cameraPos,
                                        std::vector<mathfu::vec4> &frustumPlanes,
                                        std::vector<mathfu::vec3> &frustumPoints,
                                        std::vector<mathfu::vec3> &hullLines,
                                        mathfu::mat4 &lookAtMat4,
                                        std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                        std::vector<std::shared_ptr<WmoObject>> &wmoCandidates);

    void checkSkyScenes(const StateForConditions &state, std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates);
};


#endif //WEBWOWVIEWERCPP_WDLOBJECT_H
