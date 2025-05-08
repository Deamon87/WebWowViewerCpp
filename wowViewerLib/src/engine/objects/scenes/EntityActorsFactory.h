//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
#define AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H

#include <memory>
#include <functional>
#include <iostream>
#include <mutex>
#include <tuple>
#include <cstring>
#include <type_traits>
#include <cassert>
#include "../../../../3rdparty/OffsetAllocator/offsetAllocator.hpp"


void addEntityDeallocationRecord(std::function<void()> callback);
void executeEntityDeallocators(bool forceDealloc);


// Block-based memory pool for MainClass (supports placement new/delete)
template<uint16_t MemoryBlockSize, typename T>
class MainClassBlockPool {
    static constexpr int BlockSizeInBytes = MemoryBlockSize * sizeof(T);
    std::vector<std::vector<uint8_t>> objectPools;
public:
    inline T * getPtrFromOffset(int offset) const {
        auto blockIndex = offset / MemoryBlockSize;
        auto blockOffset = (offset % MemoryBlockSize);
        auto byteOffset = blockOffset * sizeof(T);

        if (blockIndex >= (int)objectPools.size())
            return nullptr;

        auto &objectPool = objectPools[blockIndex];
        if (byteOffset >= objectPool.size())
            return nullptr;

        return (T *)&objectPools[blockIndex][byteOffset];
    }
    void expand() {
        auto &newMemory = objectPools.emplace_back();
        newMemory.resize(BlockSizeInBytes);
    }
};

// Paged storage for auxiliary component types
// Each page is a fixed-size array. Pointers into pages remain stable on growth.
// copyToBuffer assembles pages into a contiguous destination (e.g. GPU staging buffer).
template<uint16_t PageSize, typename T>
class PagedComponentStorage {
    std::vector<std::unique_ptr<std::array<T, PageSize>>> m_pages;
    size_t m_totalSize = 0;
    bool m_dirty = false;
public:
    void expand() {
        m_pages.push_back(std::make_unique<std::array<T, PageSize>>());
        m_totalSize += PageSize;
    }

    inline T * getPtrMutable(int offset) {
        auto pageIndex = offset / PageSize;
        auto pageOffset = offset % PageSize;
        if (pageIndex < 0 || pageIndex >= (int)m_pages.size())
            return nullptr;
        m_dirty = true;
        return &(*m_pages[pageIndex])[pageOffset];
    }

    inline const T * getPtrConst(int offset) const {
        auto pageIndex = offset / PageSize;
        auto pageOffset = offset % PageSize;
        if (pageIndex < 0 || pageIndex >= (int)m_pages.size())
            return nullptr;
        return &(*m_pages[pageIndex])[pageOffset];
    }

    bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    // Copy all pages into a contiguous destination buffer with type checking
    template<typename U>
    void copyToBuffer(U *dest, size_t count) const {
        static_assert(std::is_same_v<T, U>, "Type mismatch in copyToBuffer");
        assert(count <= m_totalSize);
        size_t remaining = count;
        for (auto &page : m_pages) {
            size_t toCopy = std::min(remaining, (size_t)PageSize);
            if (toCopy == 0) break;
            std::memcpy(dest, page->data(), toCopy * sizeof(T));
            dest += toCopy;
            remaining -= toCopy;
        }
    }

    size_t size() const { return m_totalSize; }

    using value_type = T;
};


template<uint16_t MemoryBlockSize, typename ObjIdType, typename MainClass, typename... Types>
class EntityFactory : public std::enable_shared_from_this<EntityFactory<MemoryBlockSize, ObjIdType, MainClass, Types...>> {
    static_assert(sizeof(ObjIdType) == sizeof(uintptr_t));

    static constexpr int ComponentAmount = sizeof...(Types);

    MainClassBlockPool<MemoryBlockSize, MainClass> m_mainPool;
    std::tuple<PagedComponentStorage<MemoryBlockSize, Types>...> m_componentStorages;

    void expandAll() {
        m_mainPool.expand();
        if constexpr (ComponentAmount > 0) {
            std::apply([](auto &... storage) {
                (storage.expand(), ...);
            }, m_componentStorages);
        }
    }

public:
    explicit EntityFactory() {
        expandAll();
    };

    template<typename... _Args>
    std::shared_ptr<MainClass> createObject(_Args&&... __args) {
        OffsetAllocator::Allocation offsetData;
        MainClass * entity = nullptr;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            offsetData = allocator.allocate(1);
            if (offsetData.offset == OffsetAllocator::Allocation::NO_SPACE) {
                allocator.growSize(MemoryBlockSize);
                expandAll();

                offsetData = allocator.allocate(1);
            }
            auto ptr = m_mainPool.getPtrFromOffset(offsetData.offset);

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

    // Access MainClass (I == 0) or auxiliary component (I >= 1)
    // For I >= 1, marks the component storage as dirty
    template<int I>
    inline auto getObjectById(ObjIdType id) {
        int index = (int)id;
        if constexpr (I == 0) {
            if (index < 0) return (MainClass *)nullptr;
            return m_mainPool.getPtrFromOffset(index);
        } else {
            static_assert(I <= ComponentAmount, "Component index out of range");
            using CompType = typename std::tuple_element_t<I - 1, std::tuple<PagedComponentStorage<MemoryBlockSize, Types>...>>::value_type;
            if (index < 0) return (CompType *)nullptr;
            return std::get<I - 1>(m_componentStorages).getPtrMutable(index);
        }
    }

    // Read-only access to auxiliary component (I >= 1), does NOT mark dirty
    template<int I>
    inline auto getObjectByIdConst(ObjIdType id) const {
        static_assert(I > 0 && I <= ComponentAmount, "Use getObjectById<0> for MainClass, or index out of range");
        using CompType = typename std::tuple_element_t<I - 1, std::tuple<PagedComponentStorage<MemoryBlockSize, Types>...>>::value_type;
        int index = (int)id;
        if (index < 0) return (const CompType *)nullptr;
        return std::get<I - 1>(m_componentStorages).getPtrConst(index);
    }

    // Check if any element of component type I (1-based) was modified since last clear
    template<int I>
    bool isComponentDirty() const {
        static_assert(I > 0 && I <= ComponentAmount, "Component index out of range");
        return std::get<I - 1>(m_componentStorages).isDirty();
    }

    // Clear the dirty flag for component type I (1-based)
    template<int I>
    void clearComponentDirty() {
        static_assert(I > 0 && I <= ComponentAmount, "Component index out of range");
        std::get<I - 1>(m_componentStorages).clearDirty();
    }

    // Copy contiguous component data to a destination buffer with type checking
    // I is 1-based (same as getObjectById indexing)
    template<int I, typename U>
    void copyComponentToBuffer(U *dest, size_t count) const {
        static_assert(I > 0 && I <= ComponentAmount, "Component index out of range");
        std::get<I - 1>(m_componentStorages).copyToBuffer(dest, count);
    }

    // Get total capacity of component storage (number of slots, including dead ones)
    template<int I>
    size_t getComponentCapacity() const {
        static_assert(I > 0 && I <= ComponentAmount, "Component index out of range");
        return std::get<I - 1>(m_componentStorages).size();
    }

private:
    void deallocate(const OffsetAllocator::Allocation &alloc) {
        std::unique_lock<std::mutex> lock(m_mutex);

        auto *ptr = m_mainPool.getPtrFromOffset(alloc.offset);

        if (!ptr) {
            std::cout << "Dealloc PTR is null for " << typeid(MainClass).name() << std::endl;
        }

        ptr->~MainClass();
        allocator.free(alloc);
    }

private:
    std::mutex m_mutex;

    OffsetAllocator::Allocator allocator = OffsetAllocator::Allocator(MemoryBlockSize);
};


#endif //AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
