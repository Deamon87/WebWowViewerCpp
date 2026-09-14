//
// Created by Deamon on 7/15/2026.
//

#ifndef AWEBWOWVIEWERCPP_WORLDOBJECTMANAGER_H
#define AWEBWOWVIEWERCPP_WORLDOBJECTMANAGER_H

#include <map>
#include <unordered_map>
#include <vector>
#include "GameObject.h"
#include "GameObjectBVH.h"
#include "../../ApiContainer.h"

// A user-placed custom M2 (not DB-driven). The manager owns the strong ref to the
// pooled M2Object; dropping it (removal / manager destruction) releases the pool slot
// through the factory's deferred deallocation, so no explicit deregistration is needed.
struct CustomM2Object {
    int id = -1;                    // stable manager-local id
    std::string name;               // user-facing label
    std::string modelFileName;      // CASC path; when empty, fileDataId is used instead
    int fileDataId = 0;
    mathfu::vec3 position = {0, 0, 0};
    mathfu::vec3 rotationEulerDeg = {0, 0, 0}; // degrees, applied as RotX * RotY * RotZ
    mathfu::vec3 scale = {1, 1, 1};

    std::shared_ptr<M2Object> m2Object = nullptr;
    // Placement matrix from the last applied transform; used to shift the (accurate,
    // load-time-computed) world AABB by the placement delta on subsequent moves.
    mathfu::mat4 lastPlacementMatrix = mathfu::mat4::Identity();
};

// Holds client-side world objects for a single map (DB-driven GameObjects,
// which spawn either an M2 or a WMO, plus user-placed custom M2s). Created once per
// opened map and handed to Map, which asks it to contribute BVH-narrowed candidates
// into the frame's exterior culling pass every frame (Map's existing M2/WMO
// candidate->drawn machinery does the exact per-object test and load-state routing
// from there, same as ADT-placed doodads).
class WorldObjectManager {
public:
    WorldObjectManager(HApiContainer api, int mapId);

    // Adds BVH-visible world objects as candidates into m2CandidatesOut/wmoCandidatesOut
    // (e.g. exteriorView->m2List / mapRenderPlan->wmoArray), mirroring how AdtObject
    // registers ADT-placed doodads as candidates for the same containers.
    void cullAndCollect(const MathHelper::FrustumCullingData &frustumData,
                        M2ObjectListContainer &m2CandidatesOut,
                        WMOListContainer &wmoCandidatesOut);

    // Adds BVH-visible WMO world objects as "is camera inside" candidates (used by
    // Map::getPotentialEntities / m_currentWMO detection). A box containing the camera
    // position always intersects the view frustum, so reusing the frustum-based BVH
    // query here doesn't miss any WMO the camera could actually be inside.
    void getPotentialWmoCandidates(const MathHelper::FrustumCullingData &frustumData, WMOListContainer &potentialWmoOut);

    HGameObject getGameObjectById(int id) const;

    // Reverse lookups from a pooled M2ObjId/WMOObjId (e.g. resolved from a GPU pick result)
    // back to the owning GameObject, if the picked M2/WMO happens to be GameObject-spawned.
    HGameObject getGameObjectByM2Id(M2ObjId id) const;
    HGameObject getGameObjectByWmoId(WMOObjId id) const;

    // ---------------- Custom user-placed M2s ----------------
    // Custom M2s bypass the (build-once, non-refit-capable) GameObjectBVH and are
    // submitted as exterior M2 candidates every frame; Map's per-object culling then
    // does the exact frustum test. This keeps add/remove/move cheap (no BVH rebuild).

    // Spawns a new custom M2 and returns its manager-local id, or -1 on invalid input
    // (no api, or neither a model file name nor a valid FileDataID given).
    int addCustomM2(const std::string &name, const std::string &modelFileName, int fileDataId,
                    const mathfu::vec3 &position, const mathfu::vec3 &rotationEulerDeg,
                    const mathfu::vec3 &scale);
    bool removeCustomM2(int id);
    void clearCustomM2s();

    // Re-applies position/rotation/scale of a custom M2 and updates its culling AABB.
    bool setCustomM2Transform(int id, const mathfu::vec3 &position,
                              const mathfu::vec3 &rotationEulerDeg, const mathfu::vec3 &scale);

    const CustomM2Object *getCustomM2(int id) const;
    const std::map<int, CustomM2Object> &getCustomM2s() const { return m_customM2s; }

    // Reverse lookup from a pooled M2ObjId (e.g. GPU pick result) to the custom object
    // id, or -1 if the picked M2 is not a custom-placed one.
    int getCustomM2IdByM2ObjId(M2ObjId id) const;

private:
    void loadGameObjects();
    std::vector<GameObject*> queryVisible(const MathHelper::FrustumCullingData &frustumData) const;

    void spawnCustomM2(CustomM2Object &object);
    void applyCustomM2Placement(CustomM2Object &object, bool updateAABB);

    HApiContainer m_api;
    int m_mapId;

    std::unordered_map<int, HGameObject> m_gameObjectsById; // keyed by GameObjectRecord.ID
    std::unordered_map<M2ObjId, GameObject*> m_gameObjectByM2Id;
    std::unordered_map<WMOObjId, GameObject*> m_gameObjectByWmoId;
    GameObjectBVH m_bvh;

    std::map<int, CustomM2Object> m_customM2s;
    std::unordered_map<M2ObjId, int> m_customIdByM2Id;
    int m_nextCustomId = 1;
};
typedef std::shared_ptr<WorldObjectManager> HWorldObjectManager;

#endif //AWEBWOWVIEWERCPP_WORLDOBJECTMANAGER_H
