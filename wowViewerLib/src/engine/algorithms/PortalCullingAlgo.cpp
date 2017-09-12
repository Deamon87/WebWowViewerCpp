//
// Created by deamon on 11.09.17.
//

#include "PortalCullingAlgo.h"

bool PortalCullingAlgo::startTraversingFromInteriorWMO(WmoObject &wmoObject, WmoGroupObject &wmoGroupsResult,
                                                       mathfu::vec4 &cameraVec4, mathfu::mat4 &lookat,
                                                       std::vector<mathfu::vec4> &frustumPlanes,
                                                       std::set<M2Object *> &m2RenderedThisFrame) {
    return false;
}

bool
PortalCullingAlgo::startTraversingFromExterior(WmoObject &wmoObject, mathfu::vec4 &cameraVec4, mathfu::mat4 &lookat,
                                               std::vector<mathfu::vec4> &frustumPlanes,
                                               std::set<M2Object *> &m2RenderedThisFrame) {
    return false;
}

void PortalCullingAlgo::checkGroupDoodads(WmoObject &wmoObject, int groupId, mathfu::vec4 &cameraVec4,
                                          std::vector<mathfu::vec4> &frustumPlanes, int level,
                                          std::set<M2Object *> &m2ObjectSet) {

}

void
PortalCullingAlgo::transverseGroupWMO(WmoObject &wmoObject, int groupId, bool fromInterior, mathfu::vec4 &cameraVec4,
                                      mathfu::vec4 &cameraLocal, mathfu::mat4 &lookat,
                                      std::vector<mathfu::vec4> &frustumPlanes, int level,
                                      std::set<M2Object *> &m2ObjectSet) {

}
