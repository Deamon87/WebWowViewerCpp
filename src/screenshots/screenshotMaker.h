//
// Created by Deamon on 9/8/2020.
//

#ifndef AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H
#define AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H

#include <string>
#include <vector>
#include <memory>

//Do not include libpng in Emscripten version
#ifndef __EMSCRIPTEN__
#include <png.h>
#endif

#include "../../wowViewerLib/src/engine/ApiContainer.h"
#ifndef __EMSCRIPTEN__
void saveScreenshot(const std::string& name, int width, int height, std::vector<uint8_t> &rgbaBuff);
#endif

void saveScreenshotLodePng(const std::string &name, int width, int height, std::vector <uint8_t> &rgbaBuff);

void saveDataFromDrawStage(const HFrameBuffer& fb,
                           const std::string& screenshotFileName,
                           int screenshotWidth, int screenshotHeight,
                           std::vector <uint8_t> &buffer);
#endif //AWEBWOWVIEWERCPP_SCREENSHOTMAKER_H
