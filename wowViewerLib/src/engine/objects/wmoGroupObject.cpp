//
// Created by deamon on 10.07.17.
//

#include "wmoGroupObject.h"
#include "../algorithms/mathHelper.h"

void WmoGroupObject::draw(SMOMaterial *materials, std::function <BlpTexture&(int materialId)> getTextureFunc) {
    if (!this->m_loaded) {
        if (m_geom != nullptr && m_geom->isLoaded()){
            this->postLoad();
            this->m_loaded = true;
            return;
        }

        this->startLoading();
        return;
    }

    if (m_geom->batchesLen <= 0) return;

    m_geom->draw(m_api, materials, getTextureFunc);

}

void WmoGroupObject::startLoading() {
    if (!this->m_loading) {
        this->m_loading = true;

        m_geom = m_api->getWmoGroupGeomCache()->get(m_fileName);
    }
}
void WmoGroupObject::postLoad() {

    this->m_dontUseLocalLightingForM2 = ((m_geom->mogp->flags & 0x40) > 0) || ((m_geom->mogp->flags & 0x8) > 0);
    this->createWorldGroupBB(m_geom->mogp->boundingBox, *m_modelMatrix);
}

void WmoGroupObject::createWorldGroupBB (CAaBox &bbox, mathfu::mat4 &placementMatrix) {
//            groupInfo = this.groupInfo;
//            bb1 = groupInfo.bb1;
//            bb2 = groupInfo.bb2;
//        } else {
//            groupInfo = this.wmoGeom.wmoGroupFile.mogp;
//            bb1 = groupInfo.BoundBoxCorner1;
//            bb2 = groupInfo.BoundBoxCorner2;
//        }
    C3Vector &bb1 = bbox.min;
    C3Vector &bb2 = bbox.max;

    mathfu::vec4 bb1vec = mathfu::vec4(bb1.x, bb1.y, bb1.z, 1);
    mathfu::vec4 bb2vec = mathfu::vec4(bb2.x, bb2.y, bb2.z, 1);

    CAaBox worldAABB = MathHelper::transformAABBWithMat4(placementMatrix, bb1vec, bb2vec);

    this->m_worldGroupBorder = worldAABB;
    this->m_volumeWorldGroupBorder = worldAABB;
}
void WmoGroupObject::updateWorldGroupBBWithM2 () {
//    var doodadRefs = this.wmoGeom.wmoGroupFile.doodadRefs;
//    var mogp = this.wmoGeom.wmoGroupFile.mogp;
//    var groupAABB = this.worldGroupBorder;
//
//    var dontUseLocalLighting = ((mogp.flags & 0x40) > 0) || ((mogp.flags & 0x8) > 0);
//
//    for (var j = 0; j < this.wmoDoodads.length; j++) {
//        var mdxObject = this.wmoDoodads[j];
//        //1. Update the mdx
//        //If at least one exterior WMO group reference the doodad - do not use the diffuse lightning from modd chunk
//        if (dontUseLocalLighting) {
//            mdxObject.setUseLocalLighting(false);
//        }
//
//        if (!mdxObject.loaded) continue; //corrupted :(
//
//        //2. Update the world group BB
//        groupAABB[0] = vec3.fromValues(Math.min(mdxObject.aabb[0][0],groupAABB[0][0]),
//                                       Math.min(mdxObject.aabb[0][1],groupAABB[0][1]),
//                                       Math.min(mdxObject.aabb[0][2],groupAABB[0][2]));
//
//        groupAABB[1] = vec3.fromValues(Math.max(mdxObject.aabb[1][0],groupAABB[1][0]),
//                                       Math.max(mdxObject.aabb[1][1],groupAABB[1][1]),
//                                       Math.max(mdxObject.aabb[1][2],groupAABB[1][2]));
//    }
}
bool WmoGroupObject::checkGroupFrustum(mathfu::vec4 &cameraPos,
                                   std::vector<mathfu::vec4> &frustumPlanes,
                                   std::vector<mathfu::vec3> &points,
                                   std::set<M2Object*> &wmoM2Candidates) {
    CAaBox &bbArray = this->m_worldGroupBorder;

    bool isInsideM2Volume = (
            cameraPos[0] > bbArray.min.z && cameraPos[0] < bbArray.max.x &&
            cameraPos[1] > bbArray.min.y && cameraPos[1] < bbArray.max.y &&
            cameraPos[2] > bbArray.min.z && cameraPos[2] < bbArray.max.z
    );

    bool drawDoodads = isInsideM2Volume || MathHelper::checkFrustum(frustumPlanes, bbArray, points);

    bbArray = this->m_volumeWorldGroupBorder;
    bool isInsideGroup = (
            cameraPos[0] > bbArray.min.z && cameraPos[0] < bbArray.max.x &&
            cameraPos[1] > bbArray.min.y && cameraPos[1] < bbArray.max.y &&
            cameraPos[2] > bbArray.min.z && cameraPos[2] < bbArray.max.z
    );

    bool drawGroup = isInsideGroup || MathHelper::checkFrustum(frustumPlanes, bbArray, points);

    if (drawDoodads) {
        this->checkDoodads(wmoM2Candidates);
    }
    return drawGroup;
}
bool WmoGroupObject::checkDoodads(std::set<M2Object*> &wmoM2Candidates){
//    for (int i = 0; i< this->wmoDoodads.length; i++) {
//        if (this.wmoDoodads[i]) {
//            if (this.dontUseLocalLightingForM2) {
//                this.wmoDoodads[i].setUseLocalLighting(false);
//            }
//            wmoM2Candidates.add(this.wmoDoodads[i]);
//        }
//    }
}