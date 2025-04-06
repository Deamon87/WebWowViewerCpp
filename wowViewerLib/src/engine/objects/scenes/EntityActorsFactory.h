//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
#define AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H

#include <memory>
#include <functional>
#include <mutex>
#include "../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"


void addEntityDeallocationRecord(std::function<void()> callback);
void executeEntityDeallocators();


namespace EntityMemoryPool {

    template<uint16_t MemoryBlockSize, typename... Args>
    class EttMemoryPool;

    template<uint16_t MemoryBlockSize, typename T, typename... Tail>
    class EttMemoryPool<MemoryBlockSize, T, Tail...> : public EttMemoryPool<MemoryBlockSize, Tail...>
    {
        static constexpr int BlockSizeInBytes = MemoryBlockSize * sizeof(T);
    public:
        EttMemoryPool() : EttMemoryPool<MemoryBlockSize, Tail...>()
        {
        }
        inline T * getPtrFromOffset(int offset) const {
            auto blockIndex = offset / MemoryBlockSize;
            auto blockOffset = (offset % MemoryBlockSize);
            auto byteOffset = blockOffset * sizeof(T);

            if (blockIndex > objectPools.size())
                return nullptr;

            auto &objectPool = objectPools[blockIndex];
            if (byteOffset >= objectPool.size())
                return nullptr;

            return (T *)&objectPools[blockIndex][byteOffset];
        }
        void expand() {
            objectPools.emplace_back().resize(BlockSizeInBytes);
            EttMemoryPool<MemoryBlockSize, Tail...>::expand();
        }

        typedef T value_type;
    private:
        std::vector<std::vector<uint8_t>> objectPools;
    };
    template<uint16_t MemoryBlockSize>
    class EttMemoryPool<MemoryBlockSize>
    {
    public:
        void expand(){};
    };
}


template<uint16_t MemoryBlockSize, typename ObjIdType, typename MainClass, typename... Types>
class EntityFactory : public std::enable_shared_from_this<EntityFactory<MemoryBlockSize, ObjIdType, MainClass, Types...>> {
    static_assert(sizeof(ObjIdType) == sizeof(uintptr_t));


    static constexpr int ComponentAmount = sizeof...(Types);



    template<int I, typename Head, typename... Args>
    struct EttMemoryPoolGetter
    {
        using return_type = typename EttMemoryPoolGetter<I-1, Args...>::return_type;
        static inline const return_type &get(const EntityMemoryPool::EttMemoryPool<MemoryBlockSize, Head, Args...> &t)
        {
            return EttMemoryPoolGetter<I-1, Args...>::get(t);
        }
    };
    template<typename Head, typename... Args>
    struct EttMemoryPoolGetter<0, Head, Args...>
    {
        using return_type = EntityMemoryPool::EttMemoryPool<MemoryBlockSize, Head, Args...>;
        static const inline return_type &get(const EntityMemoryPool::EttMemoryPool<MemoryBlockSize, Head, Args...> &t)
        {
            return t;
        }
    };

    EntityMemoryPool::EttMemoryPool<MemoryBlockSize, MainClass, Types...> m_combinedMemoryPool;

    template<int I>
    const inline typename EttMemoryPoolGetter<I, MainClass, Types...>::return_type &
    getMemoryPool()
    {
        return EttMemoryPoolGetter<I, MainClass, Types...>::get(m_combinedMemoryPool);
    }


public:
    explicit EntityFactory() {
        m_combinedMemoryPool.expand();
    };

private:
    template<int I>
    inline typename EttMemoryPoolGetter<I, MainClass, Types...>::return_type::value_type *
    getPtrFromOffset(int offset)
    {
        return getMemoryPool<I>().getPtrFromOffset(offset);
    }
public:

//    inline T * getPtrFromOffset(int offset) {
//        auto blockIndex = offset / BlockSize;
//        auto blockOffset = (offset % BlockSize);
//        return (T *)&objectPools[blockIndex][blockOffset * sizeof(T)];
//    }

    template<typename... _Args>
    std::shared_ptr<MainClass> createObject(_Args&&... __args) {
        OffsetAllocator::Allocation offsetData;
        MainClass * entity = nullptr;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            offsetData = allocator.allocate(1);
            if (offsetData.offset == OffsetAllocator::Allocation::NO_SPACE) {
                allocator.growSize(MemoryBlockSize);
                m_combinedMemoryPool.expand();

                offsetData = allocator.allocate(1);
            }
            auto const offset = offsetData.offset;
            auto ptr = getPtrFromOffset<0>(offset);

            entity = new(ptr) MainClass(std::forward<decltype(__args)>(__args)...);
            entity->setId((ObjIdType)offsetData.offset);
        }

        auto l_this = this->shared_from_this();
        return std::shared_ptr<MainClass>(entity, [offsetData, l_this](MainClass *ls) -> void {
            addEntityDeallocationRecord([offsetData, l_this]() {
                l_this->deallocate(offsetData);
            });
        });
    }
    template<int I>
    inline typename EttMemoryPoolGetter<I, MainClass, Types...>::return_type::value_type *
    getObjectById(ObjIdType id) {
        int index = (int)id;
        if ((index < 0))
            return nullptr;

        return getPtrFromOffset<I>((int)id);
    }

private:
    void deallocate(const OffsetAllocator::Allocation &alloc) {
        std::unique_lock<std::mutex> lock(m_mutex);

        getPtrFromOffset<0>(alloc.offset)->~MainClass();
        allocator.free(alloc);
    }

private:
    std::mutex m_mutex;

    OffsetAllocator::Allocator allocator = OffsetAllocator::Allocator(MemoryBlockSize);
};


#endif //AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
