//
// Created by Deamon on 10/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_BINDLESSTEXTUREHOLDER_H
#define AWEBWOWVIEWERCPP_BINDLESSTEXTUREHOLDER_H


#include <cstdint>
#include "../../../interface/IDevice.h"
#include "../../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"
#include "../../../../include/custom_container_key.h"
#include "../GDescriptorSet.h"
#include "../../../../renderer/mapScene/materials/BindlessTexture.h"

class BindlessTextureHolder : public std::enable_shared_from_this<BindlessTextureHolder> {
public:
    BindlessTextureHolder(uint32_t textureCount);

    std::shared_ptr<BindlessTexture> allocate(const HGSamplableTexture& texture);

private:
    OffsetAllocator::Allocator m_textureAllocator;

    //Texture -> Bindless in array
    std::unordered_map<wtf::KeyContainer<std::weak_ptr<HGSamplableTexture::element_type>>, std::weak_ptr<BindlessTexture>> m_textureMap;
};


#endif //AWEBWOWVIEWERCPP_BINDLESSTEXTUREHOLDER_H
