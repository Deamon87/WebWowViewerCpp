//
// Created by Deamon on 17.02.23.
//

#ifndef AWEBWOWVIEWERCPP_DESCRIPTORRECORD_H
#define AWEBWOWVIEWERCPP_DESCRIPTORRECORD_H

#include <memory>
#include <list>
#include "../buffers/IBufferVLK.h"
#include "../textures/GTextureVLK.h"



class DescriptorRecord {
public:
    enum class DescriptorRecordType {
        None, UBO, UBODynamic, Texture
    };

    DescriptorRecord() = delete;

    explicit DescriptorRecord(DescriptorRecord::DescriptorRecordType descType, const std::shared_ptr<GTextureVLK> &texture, const std::function<void()> &OnHandleChange) {
        this->descType = DescriptorRecord::DescriptorRecordType::Texture;
        this->textureVlk = texture;

        if (texture != nullptr) {
            iteratorUnique = texture->addOnHandleChange(OnHandleChange);
        }
    }
    explicit DescriptorRecord(DescriptorRecord::DescriptorRecordType descType, const std::shared_ptr<IBufferVLK> &buffer, const std::function<void()> &OnHandleChange) {
        this->descType = descType;
        this->buffer = buffer;

        iteratorUnique = buffer->addOnHandleChange(OnHandleChange);
    }

    ~DescriptorRecord() {
        if (iteratorUnique) {
            if (buffer) {
                buffer->eraseOnHandleChange(iteratorUnique);
            } else if (textureVlk) {
                textureVlk->eraseOnHandleChange(iteratorUnique);
            }
        }
    }

    typedef std::list<std::function<void()>>::const_iterator RecordInter;

public:
    DescriptorRecordType descType = DescriptorRecordType::None;

    std::shared_ptr<IBufferVLK> buffer = nullptr;
    std::shared_ptr<GTextureVLK> textureVlk = nullptr;

private:
    //holds record inside buffer/textureVlk. And frees it on destruction
    //It's safe to have this iterator here, cause class holds element with shared_ptr above ^
    std::unique_ptr<RecordInter> iteratorUnique = nullptr;
};

#endif //AWEBWOWVIEWERCPP_DESCRIPTORRECORD_H
