//
// Created by deamon on 02.08.17.
//

#ifndef WOWVIEWERLIB_GRAHAMSCAN_H
#define WOWVIEWERLIB_GRAHAMSCAN_H

#include "../persistance/header/commonFileStructs.h"
#include <stack>

typedef mathfu::vec3 Point;
std::stack<Point> grahamScan(std::vector<Point> &points);

#endif //WOWVIEWERLIB_GRAHAMSCAN_H
