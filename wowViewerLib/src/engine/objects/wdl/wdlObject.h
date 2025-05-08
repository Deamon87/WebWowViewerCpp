//
// Created by deamon on 11.01.18.
//

#ifndef WEBWOWVIEWERCPP_WDLOBJECT_H
#define WEBWOWVIEWERCPP_WDLOBJECT_H

#include <vector>
#include <map>
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
    struct SkyModelRec {
        std::shared_ptr<M2Object> m_model;
        bool animateWithTimeOfDay = false;
    };
    struct SkyObjectScene{
        uint32_t skySceneId = 0; // mssn_t::SkySceneID
        std::vector<SkyModelRec> skyModels = {};
        std::vector<SkyObjectCondition> conditions = {};
        // Player condition ids assigned to this scene by the SkySceneXPlayerCondition
        // db2. A scene passes when at least one of them is not disabled in the config
        // (treated as an additional OR'd condition group).
        std::vector<int> playerConditionIds = {};
    };

    std::vector<SkyObjectScene> skyScenes;

    // Sky scene ids grouped by the player condition that gates them, for UI display
    // (only scenes that actually have player conditions contribute). Built once in
    // loadM2s(); neither the db2 assignments nor the scenes change afterwards.
    std::map<int, std::set<int>> m_skyScenesByPlayerCondition;
public:
    bool getIsLoaded() {
        return m_loaded;
    }

    const std::map<int, std::set<int>> &getSkyScenesByPlayerCondition() const {
        return m_skyScenesByPlayerCondition;
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
