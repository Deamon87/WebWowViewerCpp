//
// Created by Deamon on 7/15/2026.
//

#ifndef AWEBWOWVIEWERCPP_GAMEOBJECTBVH_H
#define AWEBWOWVIEWERCPP_GAMEOBJECTBVH_H

#include <vector>
#include "GameObject.h"
#include "../../algorithms/mathHelper.h"

// Simple static (build-once) median-split BVH over GameObject world AABBs, used to
// narrow down frustum culling candidates before the exact per-object test.
// Not refit-capable: if an object's AABB changes after build() (e.g. once its M2
// finishes loading and its seeded AABB gets replaced by the real one), the BVH's
// node bounds will not grow to match.
class GameObjectBVH {
public:
    void build(const std::vector<GameObject*> &objects);
    void queryFrustum(const MathHelper::FrustumCullingData &frustumData, std::vector<GameObject*> &result) const;

private:
    struct Node {
        CAaBox bounds;
        int left = -1;
        int right = -1;
        int start = 0;
        int count = 0; // > 0 means leaf
    };

    static constexpr int kLeafSize = 8;

    int buildRecursive(std::vector<GameObject*> &items, int start, int count);

    std::vector<Node> m_nodes;
    std::vector<GameObject*> m_leaves;
};

#endif //AWEBWOWVIEWERCPP_GAMEOBJECTBVH_H
