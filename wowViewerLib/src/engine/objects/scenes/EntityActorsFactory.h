//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
#define AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H

#include <memory>
#include <mutex>
#include "../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"
#include "memoryPool/MemoryPool.h"



template<typename T, typename ObjIdType>
class EntityFactory {
static constexpr int initSize = 1000;
static constexpr int BlockSize = sizeof(T)*5000;

MemoryPool<T, BlockSize> pool;
public:
    explicit EntityFactory() {
        objectCache.resize(initSize);
    };

    template<typename... _Args>
    std::shared_ptr<T> createObject(_Args&&... __args) {
        OffsetAllocator::Allocation offsetData;
        T * entity = nullptr;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            offsetData = allocator.allocate(1);
            if (offsetData.offset == OffsetAllocator::Allocation::NO_SPACE) {
                constexpr int growSize = 1000;

                allocator.growSize(growSize);
                offsetData = allocator.allocate(1);
                objectCache.resize(objectCache.size() + growSize);
            }
            auto ptr = pool.allocate();

            entity = new(ptr) T(std::forward<decltype(__args)>(__args)...);
            entity->setId((ObjIdType)offsetData.offset);
            objectCache[offsetData.offset] = entity;
        }

        auto l_this = this;
        return std::shared_ptr<T>(entity, [offsetData, l_this, entity](T *ls) -> void {
            l_this->deallocate(offsetData);
        });
    }
    T * getObjectById(ObjIdType id) {
        int index = (int)id;
        if ((index < 0) || (index >= ((int)objectCache.size())))
            return nullptr;

        return objectCache[(int)id];
    }

    void deallocate(const OffsetAllocator::Allocation &alloc) {
        std::unique_lock<std::mutex> lock(m_mutex);

        objectCache[alloc.offset]->~T();
        pool.deallocate(objectCache[alloc.offset]);

        objectCache[alloc.offset] = nullptr;
        allocator.free(alloc);
    }

private:
    std::mutex m_mutex;

    OffsetAllocator::Allocator allocator = OffsetAllocator::Allocator(initSize);
    std::vector<T *> objectCache;
};


#endif //AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
