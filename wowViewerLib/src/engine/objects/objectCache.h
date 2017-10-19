//
// Created by Deamon on 10/10/2017.
//

#ifndef WEBWOWVIEWERCPP_OBJECTCACHE_H
#define WEBWOWVIEWERCPP_OBJECTCACHE_H
template <typename T, typename P = int>
class ObjectCache {
private:
    class Container {
    public:
        Container(){}
        T *obj;
        int counter;
    };
public:
    std::unordered_map<P, Container*> m_cache;
public:
    ObjectCache() {}

    T* get (P uniqueId) {
        auto it = m_cache.find(uniqueId);
        if(it != m_cache.end())
        {
            //element found;
            Container *container = it->second;
            container->counter = container->counter + 1;
            return container->obj;
        }

        return nullptr;
    }
    void put (P uniqueId, T* object) {
        Container * newContainer = new Container();
        m_cache[uniqueId] = newContainer;
        newContainer->obj = object;
        newContainer->counter = 1;
    }
    void free (P uniqueId) {
        Container &container = m_cache.at(uniqueId);

        /* Destroy container if usage counter is 0 or less */
        container.counter -= 1;
        if (container.counter <= 0) {
            m_cache.erase(uniqueId);
        }
    }
};
#endif //WEBWOWVIEWERCPP_OBJECTCACHE_H
