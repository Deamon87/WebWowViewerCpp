//
// Created by deamon on 21.06.17.
//

#ifndef WOWVIEWERLIB_CACHE_H
#define WOWVIEWERLIB_CACHE_H
#include <string>
#include <map>
#include <vector>
#include "../../include/wowScene.h"

template <typename T>
class Cache {
private:
    class Container {
    public:
        T obj;
        int counter;
    };

    IFileRequest *m_fileRequestProcessor;
public:

    std::map<std::string, Container*> m_cache;
    std::map<std::string, std::vector<unsigned char>> m_objectsToBeProcessed;
public:

    Cache(IFileRequest *fileRequestProcessor) : m_fileRequestProcessor(fileRequestProcessor){
    }
    void processCacheQueue(int limit) {
        int objectsProcessed = 0;
        for (auto it = m_objectsToBeProcessed.cbegin(); it != m_objectsToBeProcessed.cend() /* not hoisted */; /* no increment */)
        {
            std::string fileName = it->first;
            std::vector<unsigned char> fileContent = it->second;
            //ignore value
            //Value v = iter->second;

            Container *container = m_cache.at(fileName);
            container->obj.process(fileContent);

            m_objectsToBeProcessed.erase(it++);    // or "it = m.erase(it)" since C++11

            objectsProcessed++;
            if (objectsProcessed > limit) {
                break;
            }
        }
    }
    void provideFile(std::string fileName, std::vector<unsigned char> &file) {
        if (m_cache.count(fileName) > 0) {
            m_objectsToBeProcessed[fileName] = file;
        }
    }

    /*
     * Queue load functions
     */
    T* get (std::string fileName) {
        auto it = m_cache.find(fileName);
        if(it != m_cache.end())
        {
            //element found;
            Container *container = it->second;
            container->counter = container->counter + 1;
            return &container->obj;
        }

        Container * newContainer = new Container();
        m_cache[fileName] = newContainer;
        newContainer->obj = T();
        newContainer->counter = 1;

        m_fileRequestProcessor->requestFile(fileName.c_str());

        return &newContainer->obj;
    }
    void reject(std::string fileName) {
//        var queue = this.queueForLoad.get(fileName);
//        for (var i = 0; i < queue.length; i++) {
//            queue[i].reject(obj)
//        }
//        this.queueForLoad.delete(fileName);
    }
    void free (std::string fileName) {
        Container &container = m_cache.at(fileName);

        /* Destroy container if usage counter is 0 or less */
        container.counter -= 1;
        if (container.counter <= 0) {
            m_cache.erase(fileName);
        }
    }


private:
    /*
    * Cache storage functions
    */


};


#endif //WOWVIEWERLIB_CACHE_H
