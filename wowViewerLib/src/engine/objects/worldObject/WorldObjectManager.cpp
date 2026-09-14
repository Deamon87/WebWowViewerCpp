//
// Created by Deamon on 7/15/2026.
//

#include "WorldObjectManager.h"

WorldObjectManager::WorldObjectManager(HApiContainer api, int mapId) : m_api(api), m_mapId(mapId) {
    loadGameObjects();

    std::vector<GameObject*> allObjects;
    allObjects.reserve(m_gameObjectsById.size());
    for (auto &[id, gameObject] : m_gameObjectsById) {
        allObjects.push_back(gameObject.get());
    }
    m_bvh.build(allObjects);
}

void WorldObjectManager::spawnCustomM2(CustomM2Object &object) {
    object.m2Object = m2Factory->createObject(m_api);
    object.m2Object->setLoadParams(0, {}, {});
    if (!object.modelFileName.empty()) {
        object.m2Object->setModelFileName(object.modelFileName);
    } else {
        object.m2Object->setModelFileId(object.fileDataId);
    }

    applyCustomM2Placement(object, false);
}

void WorldObjectManager::applyCustomM2Placement(CustomM2Object &object, bool updateAABB) {
    if (!object.m2Object) return;

    // Same euler convention as ImGuizmo::RecomposeMatrixFromComponents (Rx * Ry * Rz),
    // so values edited through the gizmo and through numeric inputs stay in agreement.
    mathfu::mat4 rotationMat =
        MathHelper::RotationX(toRadian(object.rotationEulerDeg.x)) *
        MathHelper::RotationY(toRadian(object.rotationEulerDeg.y)) *
        MathHelper::RotationZ(toRadian(object.rotationEulerDeg.z));

    object.m2Object->createPlacementMatrix(object.position, 0.0f, object.scale, &rotationMat);
    object.m2Object->calcWorldPosition();

    // The world culling AABB lives in the pool's component storage and is only computed
    // once, at load time (M2Object::createAABB). Re-placement doesn't recompute it, so
    // shift the existing box by the placement delta here (exact for translation,
    // conservative for rotation/scale). Unloaded objects have no box yet; theirs is
    // computed when the model finishes loading.
    if (updateAABB && object.m2Object->getHasBoundingBox()) {
        mathfu::mat4 delta = object.m2Object->getModelMatrix() * object.lastPlacementMatrix.Inverse();
        const CAaBox &prevBox = object.m2Object->getAABB();
        object.m2Object->setAABB(MathHelper::transformAABBWithMat4(
            delta,
            mathfu::vec4(prevBox.min.x, prevBox.min.y, prevBox.min.z, 1.0f),
            mathfu::vec4(prevBox.max.x, prevBox.max.y, prevBox.max.z, 1.0f)));
    }

    object.lastPlacementMatrix = object.m2Object->getModelMatrix();
}

int WorldObjectManager::addCustomM2(const std::string &name, const std::string &modelFileName, int fileDataId,
                                    const mathfu::vec3 &position, const mathfu::vec3 &rotationEulerDeg,
                                    const mathfu::vec3 &scale) {
    if (m_api == nullptr) return -1;
    if (modelFileName.empty() && fileDataId <= 0) return -1;

    int id = m_nextCustomId++;
    auto &object = m_customM2s[id];
    object.id = id;
    object.name = name.empty() ? ("Custom M2 " + std::to_string(id)) : name;
    object.modelFileName = modelFileName;
    object.fileDataId = fileDataId;
    object.position = position;
    object.rotationEulerDeg = rotationEulerDeg;
    object.scale = scale;

    spawnCustomM2(object);
    m_customIdByM2Id[object.m2Object->getObjectId()] = id;

    return id;
}

bool WorldObjectManager::removeCustomM2(int id) {
    auto it = m_customM2s.find(id);
    if (it == m_customM2s.end()) return false;

    if (it->second.m2Object) {
        m_customIdByM2Id.erase(it->second.m2Object->getObjectId());
    }
    m_customM2s.erase(it); // drops the strong ref; pool slot release is deferred by the factory
    return true;
}

void WorldObjectManager::clearCustomM2s() {
    m_customM2s.clear();
    m_customIdByM2Id.clear();
}

bool WorldObjectManager::setCustomM2Transform(int id, const mathfu::vec3 &position,
                                              const mathfu::vec3 &rotationEulerDeg, const mathfu::vec3 &scale) {
    auto it = m_customM2s.find(id);
    if (it == m_customM2s.end()) return false;

    auto &object = it->second;
    object.position = position;
    object.rotationEulerDeg = rotationEulerDeg;
    object.scale = scale;

    applyCustomM2Placement(object, true);
    return true;
}

const CustomM2Object *WorldObjectManager::getCustomM2(int id) const {
    auto it = m_customM2s.find(id);
    return it != m_customM2s.end() ? &it->second : nullptr;
}

int WorldObjectManager::getCustomM2IdByM2ObjId(M2ObjId id) const {
    auto it = m_customIdByM2Id.find(id);
    return it != m_customIdByM2Id.end() ? it->second : -1;
}

void WorldObjectManager::loadGameObjects() {
    if (m_api == nullptr || m_api->databaseHandler == nullptr) return;

    std::vector<GameObjectRecord> gameObjectRecords;
    m_api->databaseHandler->getGameObjectsForMap(m_mapId, gameObjectRecords);

    std::unordered_map<int, GameObjectDisplayInfoRecord> displayInfoCache;

    for (auto &record : gameObjectRecords) {
        // 38 GAMEOBJECT_TYPE_GARRISON_BUILDING are placed on plots, which is serverside info
        if (record.TypeID == 38) continue;

        auto cacheIt = displayInfoCache.find(record.DisplayID);
        if (cacheIt == displayInfoCache.end()) {
            GameObjectDisplayInfoRecord displayInfo;
            //On failure, displayInfo stays default-constructed (FileDataID == 0), which is
            //filtered out below; caching it either way avoids re-querying the same DisplayID.
            m_api->databaseHandler->getGameObjectDisplayInfo(record.DisplayID, displayInfo);
            cacheIt = displayInfoCache.emplace(record.DisplayID, displayInfo).first;
        }

        auto &displayInfo = cacheIt->second;
        if (displayInfo.FileDataID <= 0) continue;

        auto gameObject = std::make_shared<GameObject>(m_api, record, displayInfo);
        m_gameObjectsById[record.ID] = gameObject;

        if (gameObject->isWmo()) {
            m_gameObjectByWmoId[gameObject->getWmoObject()->getObjectId()] = gameObject.get();
        } else {
            m_gameObjectByM2Id[gameObject->getM2Object()->getObjectId()] = gameObject.get();
        }
    }
}

std::vector<GameObject*> WorldObjectManager::queryVisible(const MathHelper::FrustumCullingData &frustumData) const {
    std::vector<GameObject*> visible;
    m_bvh.queryFrustum(frustumData, visible);
    return visible;
}

void WorldObjectManager::cullAndCollect(const MathHelper::FrustumCullingData &frustumData,
                                        M2ObjectListContainer &m2CandidatesOut,
                                        WMOListContainer &wmoCandidatesOut) {
    for (auto *gameObject : queryVisible(frustumData)) {
        if (gameObject->isWmo()) {
            wmoCandidatesOut.addCand(gameObject->getWmoObject());
        } else {
            m2CandidatesOut.addCandidate(gameObject->getM2Object()->getObjectId());
        }
    }

    // Custom M2s aren't in the BVH; submit them all and let Map's exact per-object
    // culling (and load-state routing) sort them out.
    for (auto &[id, customObject] : m_customM2s) {
        if (customObject.m2Object) {
            m2CandidatesOut.addCandidate(customObject.m2Object->getObjectId());
        }
    }
}

void WorldObjectManager::getPotentialWmoCandidates(const MathHelper::FrustumCullingData &frustumData, WMOListContainer &potentialWmoOut) {
    for (auto *gameObject : queryVisible(frustumData)) {
        if (gameObject->isWmo()) {
            potentialWmoOut.addCand(gameObject->getWmoObject());
        }
    }
}

HGameObject WorldObjectManager::getGameObjectById(int id) const {
    auto it = m_gameObjectsById.find(id);
    return it != m_gameObjectsById.end() ? it->second : nullptr;
}

HGameObject WorldObjectManager::getGameObjectByM2Id(M2ObjId id) const {
    auto it = m_gameObjectByM2Id.find(id);
    if (it == m_gameObjectByM2Id.end()) return nullptr;

    return getGameObjectById(it->second->getId());
}

HGameObject WorldObjectManager::getGameObjectByWmoId(WMOObjId id) const {
    auto it = m_gameObjectByWmoId.find(id);
    if (it == m_gameObjectByWmoId.end()) return nullptr;

    return getGameObjectById(it->second->getId());
}
