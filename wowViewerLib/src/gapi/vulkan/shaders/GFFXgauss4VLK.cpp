//
// Created by Deamon on 12/13/2020.
//

#include "GFFXgauss4VLK.h"

namespace GFFXgauss4VLKPrivate {
    auto vertShaderName = std::string("drawQuad");
    auto fragShaderName = std::string("ffxgauss4");
}

GFFXgauss4VLK::GFFXgauss4VLK(std::string &shaderName, IDevice *device) :
    GShaderPermutationVLK(shaderName, GFFXgauss4VLKPrivate::vertShaderName, GFFXgauss4VLKPrivate::fragShaderName, device) {


}
