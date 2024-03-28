//
// Created by Deamon on 12/13/2023.
//

#ifndef AWEBWOWVIEWERCPP_FRAMEBASEDSTACKALLOCATOR_H
#define AWEBWOWVIEWERCPP_FRAMEBASEDSTACKALLOCATOR_H

#undef max

#include <new>
#include <limits>
#include <stdint.h>
#include "tbb/tbb.h"
#include "tbb/scalable_allocator.h"

template <class T>
using transp_vec = std::vector<T, tbb::cache_aligned_allocator<T>>;

namespace framebased {

#if ((defined (_GLIBCXX_VECTOR) \
      || defined (_LIBCPP_VECTOR) \
      || defined (_VECTOR_)))
#define ARENA_HAS_VECTOR_DEF
    template <class T>
    using vector = std::vector<T, tbb::cache_aligned_allocator<T>>;
#endif

#if ((defined (_GLIBCXX_UNORDERED_MAP) \
      || defined (_LIBCPP_UNORDERED_MAP) \
      || defined (_UNORDERED_MAP_)) \
     && !defined (ARENA_HAS_UNORDERED_MAP_DEF))
#define ARENA_HAS_UNORDERED_MAP_DEF
    template <class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
    using unordered_map = std::unordered_map<Key, Value, Hash, KeyEqual, tbb::cache_aligned_allocator<std::pair<const Key, Value>>>;
#endif
}

#endif //AWEBWOWVIEWERCPP_FRAMEBASEDSTACKALLOCATOR_H
