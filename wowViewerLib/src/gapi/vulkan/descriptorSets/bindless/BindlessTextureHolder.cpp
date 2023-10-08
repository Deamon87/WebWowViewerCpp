//
// Created by Deamon on 10/6/2023.
//

#include "BindlessTextureHolder.h"

BindlessTextureHolder::BindlessTextureHolder(uint32_t textureCount) :
    m_textureAllocator(OffsetAllocator::Allocator(textureCount)){

}

std::shared_ptr<BindlessTexture> BindlessTextureHolder::allocate(const HGSamplableTexture &texture) {
    auto weakTexture = std::weak_ptr(texture);

    auto i = m_textureMap.find(weakTexture);
    if (i != m_textureMap.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            m_textureMap.erase(i);
        }
    }

    OffsetAllocator::Allocation textNum = m_textureAllocator.allocate(1);
    if (textNum.offset == OffsetAllocator::Allocation::NO_SPACE) {
        std::cerr << "Could not allocate bindless texture";
        throw "oops";
    }

    auto l_weak = this->weak_from_this();
    auto bindlessTexture = std::make_shared<BindlessTexture>(textNum.offset, [l_weak, textNum]() {
        auto l_this = l_weak.lock();
        if (l_this == nullptr) return;

        l_this->m_textureAllocator.free(textNum);
    });

    m_textureMap[weakTexture] = bindlessTexture;

    return bindlessTexture;
}
