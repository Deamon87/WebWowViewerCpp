//
// Created by Deamon on 2/24/2023.
//

#ifndef AWEBWOWVIEWERCPP_CUSTOM_CONTAINER_KEY_H
#define AWEBWOWVIEWERCPP_CUSTOM_CONTAINER_KEY_H

#include <memory>

namespace wtf {
    template<typename T, typename = std::enable_if_t<std::is_class_v<std::weak_ptr<typename T::element_type>>>>
    struct KeyContainer {
        KeyContainer(const std::weak_ptr<typename T::element_type> &weakPtr) {
            w_key = weakPtr;
            key = weakPtr.lock().get();
        }

        bool operator ==(const KeyContainer<T> &r_key) const {
            return std::owner_less<T>()(r_key.w_key, w_key);
        }

        bool isExpired() {
            return w_key.expired();
        }

        std::weak_ptr<typename T::element_type> w_key;
        typename T::element_type *key;
    };
}
namespace std {
    template <typename T>
    struct hash<wtf::KeyContainer<T>>
    {
        std::size_t operator()(const wtf::KeyContainer<T>& k) const
        {
            return (std::hash<void*>()(k.key));
        }
    };
}
#endif //AWEBWOWVIEWERCPP_CUSTOM_CONTAINER_KEY_H
