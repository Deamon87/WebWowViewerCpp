//
// Created by Deamon on 12/13/2020.
//

#include "GFFXGlowVLK.h"

namespace GFFXGlowVLKPrivate {
    auto vertShaderName = std::string("drawQuad");
    auto fragShaderName = std::string("ffxglow");
}

GFFXGlowVLK::GFFXGlowVLK(std::string &shaderName, IDevice *device) :
    GShaderPermutationVLK(shaderName, GFFXGlowVLKPrivate::vertShaderName, GFFXGlowVLKPrivate::fragShaderName, device) {
}
