//
// Created by deamon on 21.06.17.
//

#ifndef WOWVIEWERLIB_CACHE_H
#define WOWVIEWERLIB_CACHE_H
#include <string>
#include <map>
#include <list>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <forward_list>
#include "../stringTrim.h"
#include "../../include/iostuff.h"
#include "../../include/sharedFile.h"

struct FileCacheRecord {
    std::string fileName;
    HFileContent fileContent;
};

template <typename T>
class Cache {
protected:
    CacheHolderType holderType;
private:
    IFileRequest *m_fileRequestProcessor;
public:
    std::mutex cacheMapMutex;
    std::mutex getFileMutex;
    std::mutex objectsToBeProcessedMutex;

    std::unordered_map<std::string, std::weak_ptr<T>> m_cache;
    std::unordered_map<int, std::weak_ptr<T>> m_cacheFdid;
    std::forward_list<FileCacheRecord> m_objectsToBeProcessed;
public:
    Cache(IFileRequest *fileRequestProcessor, CacheHolderType holderType) : m_fileRequestProcessor(fileRequestProcessor), holderType(holderType){
    }
    void processCacheQueue(int limit) {
        int objectsProcessed = 0;

        auto const &m_cache_const = m_cache;
        while (!m_objectsToBeProcessed.empty())
        {
            std::unique_lock<std::mutex> lck (objectsToBeProcessedMutex,std::defer_lock);
            std::unique_lock<std::mutex> cacheLck (cacheMapMutex,std::defer_lock);

            lck.lock();
            auto const it = m_objectsToBeProcessed.front();
            m_objectsToBeProcessed.pop_front();
            lck.unlock();


            cacheLck.lock();
            std::weak_ptr<T> weakPtr = m_cache_const.at(it.fileName);
            cacheLck.unlock();

//            std::cout << "Processing file " << it.fileName << std::endl << std::flush;
            if (!weakPtr.expired()) {
                std::shared_ptr<T> sharedPtr = weakPtr.lock();
                if (sharedPtr->getStatus() == FileStatus::FSLoaded) {
                    std::cout << "sharedPtr->getStatus == FileStatus::FSLoaded" << std::endl;
                } else {
                    sharedPtr->process(it.fileContent, it.fileName);
                }
            }
//            std::cout << "Processed file " << it.fileName << std::endl << std::flush;

            objectsProcessed++;
            if (objectsProcessed > limit) {
                break;
            }
        }

    }
    void provideFile(std::string &fileName, HFileContent fileContent) {
        trim(fileName);
//        std::cout << "called provideFile with fileName = " << fileName << std::endl;
//        std::cout << "m_cache.size() == " << m_cache.size() << " " << __PRETTY_FUNCTION__ << std::endl;

//        std::cout << "filename:" << fileName << " hex: " << string_to_hex(fileName) << std::endl;
//        std::cout << "first in storage:" << m_cache.begin()->first << " hex: " << string_to_hex(m_cache.begin()->first) << std::endl << std::flush;
        auto const &m_cache_const = m_cache;
        
        std::unique_lock<std::mutex> cacheLck(cacheMapMutex, std::defer_lock);
        cacheLck.lock();
        auto it = m_cache_const.find(fileName);
        bool found = it != m_cache_const.end();
        cacheLck.unlock();
        if (found) {
            std::lock_guard<std::mutex> guard(objectsToBeProcessedMutex);
            m_objectsToBeProcessed.push_front({fileName, fileContent});
        }
    }

    /*
     * Queue load functions
     */
    std::shared_ptr<T> get (std::string fileName) {
        std::lock_guard<std::mutex> lock(getFileMutex);

        std::unique_lock<std::mutex> cacheLck(cacheMapMutex, std::defer_lock);

        fileName = trimmed(fileName);
        std::transform(fileName.begin(), fileName.end(),fileName.begin(), ::toupper);
        std::replace(fileName.begin(), fileName.end(), '\\', '/');

        cacheLck.lock();
        auto it = m_cache.find(fileName);
        bool found = it != m_cache.end();
        cacheLck.unlock();
        if(found) {
            if (std::shared_ptr<T> shared = it->second.lock()) {
                //element found;
                return shared;
            }
        }

        std::shared_ptr<T> sharedPtr = std::make_shared<T>(fileName);
        std::weak_ptr<T> weakPtr(sharedPtr);
        cacheLck.lock();
        m_cache[fileName] = weakPtr;
        cacheLck.unlock();
//        std::cout << "m_cache.size() == " << m_cache.size() << " " << __PRETTY_FUNCTION__ << std::endl;

        m_fileRequestProcessor->requestFile(fileName, this->holderType, weakPtr);

        return sharedPtr;
    }

    std::shared_ptr<T> getFileId (int id) {
        if (id == 0) {
//            __debugbreak();
            return nullptr;
        }

        std::lock_guard<std::mutex> guard(getFileMutex);

        std::unique_lock<std::mutex> cacheLck(cacheMapMutex, std::defer_lock);

        std::stringstream ss;
        ss << "File" << std::setfill('0') << std::setw(8) << std::hex << id <<".unk";
        std::string fileName = ss.str();

        cacheLck.lock();
        {
            auto it = m_cache.find(fileName);
            bool found = it != m_cache.end();
            if (found) {
                if (!it->second.expired()) {
                    return it->second.lock();
                } else {
//                std::cout << "getFileId: fileName = " << fileName << " is expired" << std::endl;
                }
            }
        }
        std::shared_ptr<T> sharedPtr = std::make_shared<T>(id);

        m_cache[fileName] = sharedPtr;
        cacheLck.unlock();

        m_fileRequestProcessor->requestFile(fileName, this->holderType, sharedPtr);

        return sharedPtr;
    }

    void reject(std::string fileName) {
        trim(fileName);

        std::unique_lock<std::mutex> cacheLck(cacheMapMutex, std::defer_lock);

        cacheLck.lock();
        std::weak_ptr<T> weakPtr = m_cache[fileName];
        cacheLck.unlock();

        if (!weakPtr.expired())
        {
            if (std::shared_ptr<T> sharedPtr = weakPtr.lock()) {
                sharedPtr->rejected();
            }
        }
    }

    void clear() {
        std::unique_lock<std::mutex> cacheLck(cacheMapMutex, std::defer_lock);
        cacheLck.lock();
        m_cache.clear();
        cacheLck.unlock();

        std::unique_lock<std::mutex> lck(objectsToBeProcessedMutex, std::defer_lock);
        lck.lock();
        m_objectsToBeProcessed.clear();
        lck.unlock();
    }
private:
    /*
    * Cache storage functions
    */


};


#endif //WOWVIEWERLIB_CACHE_H
