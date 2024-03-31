//
// Created by Deamon on 3/8/2023.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIALINSTANCE_H
#define AWEBWOWVIEWERCPP_IMATERIALINSTANCE_H


#include "../../../../gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "../../materials/IMaterialStructs.h"

template <typename IMaterialClass>
class IMaterialInstance : public ISimpleMaterialVLK, public IMaterialClass {
public:
    inline IMaterialInstance(
        const std::function<void(IMaterialClass *)> &initializer,
        const std::shared_ptr<GShaderPermutationVLK> &shader,
        const PipelineTemplate &pipelineTemplate,
        const HPipelineVLK &pipeline,
        const HPipelineVLK &gBufferPipeline,
        const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets,
        uint32_t materialId) :
        ISimpleMaterialVLK(shader, pipelineTemplate, pipeline, gBufferPipeline, descriptorSets, materialId) {

        initializer(this);
    }
};


#endif //AWEBWOWVIEWERCPP_IMATERIALINSTANCE_H
