//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H
#define AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H

#include <string>
#include <vector>
#include <memory>
#include <png.h>
#include "../../wowViewerLib/src/engine/ApiContainer.h"

void saveScreenshot(const std::string& name, int width, int height, std::vector<uint8_t> &rgbaBuff);
void saveScreenshotLodePng(const std::string &name, int width, int height, std::vector <uint8_t> &rgbaBuff);

void saveDataFromDrawStage(const HFrameBuffer& fb,
                           const std::string& screenshotFileName,
                           int screenshotWidth, int screenshotHeight,
                           std::vector <uint8_t> &buffer);
#endif //AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H
