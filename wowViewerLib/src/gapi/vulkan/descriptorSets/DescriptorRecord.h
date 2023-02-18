//
// Created by Deamon on 17.02.23.
//

#ifndef AWEBWOWVIEWERCPP_DESCRIPTORRECORD_H
#define AWEBWOWVIEWERCPP_DESCRIPTORRECORD_H

#include <memory>
#include "../buffers/IBufferVLK.h"
#include "../textures/GTextureVLK.h"

struct DescriptorRecord {
    enum class DescriptorRecordType {
        None, UBO, UBODynamic, Texture
    };

    DescriptorRecordType descType = DescriptorRecordType::None;
    std::shared_ptr<IBufferVLK> buffer = nullptr;
    std::shared_ptr<GTextureVLK> textureVlk = nullptr;
};

#endif //AWEBWOWVIEWERCPP_DESCRIPTORRECORD_H
