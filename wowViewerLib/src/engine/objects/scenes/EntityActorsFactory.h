//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
#define AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H

#include <memory>
#include <mutex>
#include "../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"

template<typename T>
class EntityFactory : public std::enable_shared_from_this<EntityFactory<T>>{
static constexpr int initSize = 1000;
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

            entity = new T(std::forward<decltype(__args)>(__args)...);
            entity->setId(offsetData.offset);
            objectCache[offsetData.offset] = entity;
        }

        auto weakPtr = this->weak_from_this();
        return std::shared_ptr<T>(entity, [offsetData, weakPtr](T *ls) -> void {
            auto shared = weakPtr.lock();
            if (shared != nullptr)
                shared->deallocate(offsetData);
        });
    }
    T * getObjectById(int id) {
        return objectCache[id];
    }

    void deallocate(const OffsetAllocator::Allocation &alloc) {
        std::unique_lock<std::mutex> lock(m_mutex);

        delete objectCache[alloc.offset];

        objectCache[alloc.offset] = nullptr;
        allocator.free(alloc);
    }

private:
    std::mutex m_mutex;

    OffsetAllocator::Allocator allocator = OffsetAllocator::Allocator(initSize);
    std::vector<T *> objectCache;
};


#endif //AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
