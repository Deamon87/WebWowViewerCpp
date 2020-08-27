//
// Created by Deamon on 26.03.2020.
//

#include "GImguiShaderPermutation.h"
#include <string>
#include <fstream>

GImguiShaderPermutation::GImguiShaderPermutation(std::string &shaderName, IDevice *device) : GShaderPermutationVLK(
    shaderName, device) {

}