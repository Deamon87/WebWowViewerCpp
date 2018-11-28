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
#include "../../include/wowScene.h"
#include "../stringTrim.h"

struct FileCacheRecord {
    std::string fileName;
    std::vector<unsigned char> fileContent;
};

template <typename T>
class Cache {
protected:
    CacheHolderType holderType;
private:
    IFileRequest *m_fileRequestProcessor;
public:
    std::mutex accessMutex;
    std::unique_lock<std::mutex> processCacheLock;
    std::unique_lock<std::mutex> provideFileLock;
    std::unordered_map<std::string, std::weak_ptr<T>> m_cache;
    std::forward_list<FileCacheRecord> m_objectsToBeProcessed;
public:
    Cache(IFileRequest *fileRequestProcessor, CacheHolderType holderType) : m_fileRequestProcessor(fileRequestProcessor), holderType(holderType){
        processCacheLock = std::unique_lock<std::mutex>(accessMutex,std::defer_lock);
        provideFileLock = std::unique_lock<std::mutex>(accessMutex,std::defer_lock);
    }
    void processCacheQueue(int limit) {
        int objectsProcessed = 0;

        processCacheLock.lock();
        while (!m_objectsToBeProcessed.empty())
        {
            auto const it = m_objectsToBeProcessed.front();
            std::weak_ptr<T> weakPtr = m_cache.at(it.fileName);

//            std::cout << "Processing file " << it.fileName << std::endl << std::flush;
            if (std::shared_ptr<T> sharedPtr = weakPtr.lock()) {
                sharedPtr->process(it.fileContent, it.fileName);
            }
//            std::cout << "Processed file " << it.fileName << std::endl << std::flush;

            m_objectsToBeProcessed.pop_front();

            objectsProcessed++;
            if (objectsProcessed > limit) {
                break;
            }
        }
        processCacheLock.unlock();
    }
    void provideFile(std::string &fileName, uint8_t* fileContentPtr, int fileSize) {
        trim(fileName);
//        std::cout << "called provideFile with fileName = " << fileName << std::endl;
//        std::cout << "m_cache.size() == " << m_cache.size() << " " << __PRETTY_FUNCTION__ << std::endl;

//        std::cout << "filename:" << fileName << " hex: " << string_to_hex(fileName) << std::endl;
//        std::cout << "first in storage:" << m_cache.begin()->first << " hex: " << string_to_hex(m_cache.begin()->first) << std::endl << std::flush;
//        provideFileLock.lock();
        auto it = m_cache.find(fileName);
        if (it != m_cache.end()) {
            m_objectsToBeProcessed.push_front({fileName, std::vector<unsigned char>(fileContentPtr, fileContentPtr+fileSize)});
        }
//        provideFileLock.unlock();
    }

    /*
     * Queue load functions
     */
    std::shared_ptr<T> get (std::string fileName) {
        fileName = trimmed(fileName);
        std::transform(fileName.begin(), fileName.end(),fileName.begin(), ::toupper);
        std::replace(fileName.begin(), fileName.end(), '\\', '/');

        auto it = m_cache.find(fileName);
        if(it != m_cache.end() ) {
            if (std::shared_ptr<T> shared = it->second.lock()) {
                //element found;
                return shared;
            }
        }

        std::shared_ptr<T> sharedPtr = std::make_shared<T>(fileName);
        std::weak_ptr<T> weakPtr(sharedPtr);
        m_cache[fileName] = weakPtr;
//        std::cout << "m_cache.size() == " << m_cache.size() << " " << __PRETTY_FUNCTION__ << std::endl;

        m_fileRequestProcessor->requestFile(fileName.c_str(), this->holderType);

        return sharedPtr;
    }

    std::shared_ptr<T> getFileId (int id) {
        std::stringstream ss;
        ss << "File" << std::setfill('0') << std::setw(8) << std::hex << id <<".unk";
        std::string fileName = ss.str();


        auto it = m_cache.find(fileName);
        if(it != m_cache.end())
        {
            if (std::shared_ptr<T> shared = it->second.lock()) {
                //element found;
                return shared;
            }
        }

        std::shared_ptr<T> sharedPtr = std::make_shared<T>(id);
        std::weak_ptr<T> weakPtr(sharedPtr);
        m_cache[fileName] = weakPtr;


        m_fileRequestProcessor->requestFile(fileName.c_str(), this->holderType);

        return sharedPtr;
    }

    void reject(std::string fileName) {
        trim(fileName);
    }

    void clear() {
        m_cache.clear();
        m_objectsToBeProcessed.clear();
    }
private:
    /*
    * Cache storage functions
    */


};


#endif //WOWVIEWERLIB_CACHE_H
