//
// Created by Deamon on 7/15/2026.
//

#ifndef AWEBWOWVIEWERCPP_GAMEOBJECT_H
#define AWEBWOWVIEWERCPP_GAMEOBJECT_H

#include <memory>
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"
#include "../../ApiContainer.h"
#include "../../../include/database/dbStructs.h"

// Client-side world object driven by the GameObjects/GameObjectDisplayInfo DB2s.
// Builds and owns the strong ref to its spawned M2Object or WmoObject (mirrors how
// AdtObject/Map own doodad/MODF-placed objects) directly from the DB records.
// Exactly one of the two is set, never both. Other world-object types are expected
// to follow this same shape.
class GameObject {
public:
    GameObject(HApiContainer api, const GameObjectRecord &record, const GameObjectDisplayInfoRecord &displayInfo);

    int getId() const { return m_id; }
    int getOwnerId() const { return m_ownerId; }
    int getDisplayId() const { return m_displayId; }
    int getTypeId() const { return m_typeId; }
    const std::string &getName() const { return m_name; }

    bool isWmo() const { return m_wmoObject != nullptr; }
    const std::shared_ptr<M2Object> &getM2Object() const { return m_m2Object; }
    const std::shared_ptr<WmoObject> &getWmoObject() const { return m_wmoObject; }

    CAaBox getAABB() const {
        return m_wmoObject ? m_wmoObject->getAABB() : m_m2Object->getAABB();
    }

    // TypeID == 5 (GAMEOBJECT_TYPE_GENERIC) uses PropValue[0] as `floatingTooltip`:
    // a server-authored opt-in for showing a nameplate tooltip on hover.
    bool hasFloatingTooltip() const { return m_typeId == 5 && m_propValue0 > 0; }

private:
    static bool isWmoSpawningType(int typeId);

    int m_id;
    int m_ownerId;
    int m_displayId;
    int m_typeId;
    std::string m_name;
    int m_propValue0;

    std::shared_ptr<M2Object> m_m2Object;
    std::shared_ptr<WmoObject> m_wmoObject;
};
typedef std::shared_ptr<GameObject> HGameObject;

#endif //AWEBWOWVIEWERCPP_GAMEOBJECT_H
