//
// Created by Deamon on 7/15/2026.
//

#include "GameObjectBVH.h"
#include <algorithm>

namespace {
    inline float getAxis(const C3Vector &v, int axis) {
        switch (axis) {
            case 0: return v.x;
            case 1: return v.y;
            default: return v.z;
        }
    }

    inline CAaBox unionAABB(const CAaBox &a, const CAaBox &b) {
        CAaBox result;
        result.min.x = std::min(a.min.x, b.min.x);
        result.min.y = std::min(a.min.y, b.min.y);
        result.min.z = std::min(a.min.z, b.min.z);
        result.max.x = std::max(a.max.x, b.max.x);
        result.max.y = std::max(a.max.y, b.max.y);
        result.max.z = std::max(a.max.z, b.max.z);
        return result;
    }
}

void GameObjectBVH::build(const std::vector<GameObject*> &objects) {
    m_nodes.clear();
    m_leaves.clear();

    if (objects.empty()) return;

    m_nodes.reserve(objects.size() * 2);
    m_leaves.reserve(objects.size());

    std::vector<GameObject*> items = objects;
    buildRecursive(items, 0, (int)items.size());
}

int GameObjectBVH::buildRecursive(std::vector<GameObject*> &items, int start, int count) {
    int nodeIndex = (int)m_nodes.size();
    m_nodes.emplace_back();

    CAaBox bounds = items[start]->getAABB();
    for (int i = start + 1; i < start + count; i++) {
        bounds = unionAABB(bounds, items[i]->getAABB());
    }

    if (count <= kLeafSize) {
        int leafStart = (int)m_leaves.size();
        for (int i = start; i < start + count; i++) {
            m_leaves.push_back(items[i]);
        }

        m_nodes[nodeIndex].bounds = bounds;
        m_nodes[nodeIndex].start = leafStart;
        m_nodes[nodeIndex].count = count;
        return nodeIndex;
    }

    float extentX = bounds.max.x - bounds.min.x;
    float extentY = bounds.max.y - bounds.min.y;
    float extentZ = bounds.max.z - bounds.min.z;

    int axis = 0;
    if (extentY >= extentX && extentY >= extentZ) axis = 1;
    else if (extentZ >= extentX && extentZ >= extentY) axis = 2;

    int mid = start + count / 2;
    std::nth_element(items.begin() + start, items.begin() + mid, items.begin() + start + count,
        [axis](GameObject *a, GameObject *b) {
            CAaBox boxA = a->getAABB();
            CAaBox boxB = b->getAABB();
            float centroidA = getAxis(boxA.min, axis) + getAxis(boxA.max, axis);
            float centroidB = getAxis(boxB.min, axis) + getAxis(boxB.max, axis);
            return centroidA < centroidB;
        });

    int leftIndex = buildRecursive(items, start, count / 2);
    int rightIndex = buildRecursive(items, mid, count - count / 2);

    //buildRecursive may have reallocated m_nodes; re-index instead of holding a stale reference.
    m_nodes[nodeIndex].bounds = bounds;
    m_nodes[nodeIndex].left = leftIndex;
    m_nodes[nodeIndex].right = rightIndex;
    m_nodes[nodeIndex].count = 0;

    return nodeIndex;
}

void GameObjectBVH::queryFrustum(const MathHelper::FrustumCullingData &frustumData, std::vector<GameObject*> &result) const {
    if (m_nodes.empty()) return;

    std::vector<int> stack;
    stack.push_back(0);

    while (!stack.empty()) {
        int nodeIndex = stack.back();
        stack.pop_back();

        const Node &node = m_nodes[nodeIndex];
        if (!MathHelper::checkFrustum(frustumData, node.bounds)) continue;

        if (node.count > 0) {
            for (int i = node.start; i < node.start + node.count; i++) {
                result.push_back(m_leaves[i]);
            }
        } else {
            if (node.left >= 0) stack.push_back(node.left);
            if (node.right >= 0) stack.push_back(node.right);
        }
    }
}
