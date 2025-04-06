//
// Created by deamon on 02.08.17.
//

#ifndef WOWVIEWERLIB_GRAHAMSCAN_H
#define WOWVIEWERLIB_GRAHAMSCAN_H

#include "../persistance/header/commonFileStructs.h"
#include <vector>
#include <stack>
#include "../custom_allocators/FrameBasedStackAllocator.h"

typedef mathfu::vec3 Point;
std::stack<Point> grahamScan(framebased::vector<Point> &points);

#endif //WOWVIEWERLIB_GRAHAMSCAN_H
