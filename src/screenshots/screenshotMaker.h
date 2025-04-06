//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H
#define AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H

#include <string>
#include <vector>
#include <memory>

#include "../../wowViewerLib/src/engine/ApiContainer.h"

void saveScreenshotLodePng(const std::string &name, int width, int height, std::vector <uint8_t> &rgbaBuff);

void saveDataFromDrawStage(std::function<void(int, int, int, int, uint8_t* data)> readRGBAPixels,
                           const std::string& screenshotFileName,
                           int screenshotWidth, int screenshotHeight);
#endif //AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H
