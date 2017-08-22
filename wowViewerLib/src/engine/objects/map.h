//
// Created by Deamon on 7/16/2017.
//

#ifndef WEBWOWVIEWERCPP_MAP_H
#define WEBWOWVIEWERCPP_MAP_H


#include "adtObject.h"
#include "m2Object.h"
#include "wmoObject.h"
#include "iMapApi.h"

class Map : public IMapApi {
private:
    template <typename T, typename P = int>
    class ObjectCache {
    private:
        class Container {
        public:
            T obj;
            int counter;
        };
    public:
        std::map<P, Container*> m_cache;
    public:
        ObjectCache() {}

        T* get (P uniqueId) {
            auto it = m_cache.find(uniqueId);
            if(it != m_cache.end())
            {
                //element found;
                Container *container = it->second;
                container->counter = container->counter + 1;
                return &container->obj;
            }

           return nullptr;
        }
        void put (P uniqueId, T* object) {
            Container * newContainer = new Container();
            m_cache[uniqueId] = newContainer;
            newContainer->obj = T();
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


private:
    AdtObject *mapTiles[64][64]={};


    ObjectCache<int, M2Object> m_m2MapObjects;
    ObjectCache<int, WmoObject> m_wmoMapObjects;
public:
    void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos);

private:
    void checkExterior(mathfu::vec4 &cameraPos,
            std::vector<mathfu::vec4> &frustumPlanes,
            std::vector<mathfu::vec3> &frustumPoints,
            std::vector<mathfu::vec3> &hullLines,
            mathfu::mat4 &lookAtMat4,
            std::set<AdtObject*> &adtRenderedThisFrame,
            std::set<M2Object*> &m2RenderedThisFrame,
            std::set<WmoObject*> &wmoRenderedThisFrame);

};


#endif //WEBWOWVIEWERCPP_MAP_H
