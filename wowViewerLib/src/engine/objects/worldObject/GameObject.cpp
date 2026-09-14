//
// Created by Deamon on 7/15/2026.
//

#include "GameObject.h"

bool GameObject::isWmoSpawningType(int typeId) {
    //Empirically observed: GameObjects.TypeID == 38 spawns a WMO (rather than an M2) for its DisplayID.
    //There may be other WMO-spawning types not yet identified; extend here if more are found.
    return typeId == 38;
}

GameObject::GameObject(HApiContainer api, const GameObjectRecord &record, const GameObjectDisplayInfoRecord &displayInfo) :
    m_id(record.ID), m_ownerId(record.OwnerID), m_displayId(record.DisplayID), m_typeId(record.TypeID),
    m_name(record.Name), m_propValue0(record.PropValue0) {

    mathfu::vec3 pos(record.Pos[0], record.Pos[1], record.Pos[2]);
    mathfu::vec3 scaleVec(record.Scale, record.Scale, record.Scale);
    mathfu::quat rotationQuat(record.Rot[3], record.Rot[0], record.Rot[1], record.Rot[2]);
    mathfu::mat4 rotationMat = rotationQuat.ToMatrix4();

    mathfu::vec4 minVec(displayInfo.GeoBoxMin[0], displayInfo.GeoBoxMin[1], displayInfo.GeoBoxMin[2], 1);
    mathfu::vec4 maxVec(displayInfo.GeoBoxMax[0], displayInfo.GeoBoxMax[1], displayInfo.GeoBoxMax[2], 1);

    if (isWmoSpawningType(record.TypeID)) {
        m_wmoObject = wmoFactory->createObject(api);
        m_wmoObject->setModelFileId(displayInfo.FileDataID);
        m_wmoObject->setLoadingParam(pos, scaleVec, &rotationMat, minVec, maxVec);
    } else {
        m_m2Object = m2Factory->createObject(api);
        m_m2Object->setLoadParams(0, {}, {});
        m_m2Object->setModelFileId(displayInfo.FileDataID);

        m_m2Object->createPlacementMatrix(pos, 0.0f, scaleVec, &rotationMat);
        m_m2Object->calcWorldPosition();

        CAaBox initialAABB = MathHelper::transformAABBWithMat4(m_m2Object->getModelMatrix(), minVec, maxVec);
        m_m2Object->setInitialAABB(initialAABB);
    }
}
