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
    explicit WdlObject(HApiContainer api, std::string &wdlFileName);
    explicit WdlObject(HApiContainer api, int wdlFileDataId);

    void setMapApi(IMapApi *api) {
        m_mapApi = api;
    }

    std::vector<std::shared_ptr<M2Object>> m2Objects;
    std::vector<std::shared_ptr<WmoObject>> wmoObjects;

private:
    HApiContainer m_api;
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

    bool checkFrustumCulling(const MathHelper::FrustumCullingData &frustumData,
                             mathfu::vec4 &cameraPos,
                             M2ObjectListContainer &m2ObjectsCandidates,
                             WMOListContainer &wmoCandidates);

    void checkSkyScenes(const StateForConditions &state,
                        M2ObjectListContainer &m2ObjectsCandidates,
                        const mathfu::vec4 &cameraPos,
                        const MathHelper::FrustumCullingData &frustumData);
};


#endif //WEBWOWVIEWERCPP_WDLOBJECT_H
