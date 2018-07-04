//
// Created by deamon on 21.06.17.
//

#ifndef WOWVIEWERLIB_CACHE_H
#define WOWVIEWERLIB_CACHE_H
#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>
#include "../../include/wowScene.h"
#include "../stringTrim.h"


template <typename T>
class Cache {
private:
    IFileRequest *m_fileRequestProcessor;
public:

    std::unordered_map<std::string, std::weak_ptr<T>> m_cache;
    std::unordered_map<std::string, std::vector<unsigned char>> m_objectsToBeProcessed;
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

            std::weak_ptr<T> weakPtr = m_cache.at(fileName);

//            std::cout << "Processing file " << fileName << std::endl << std::flush;
            if (std::shared_ptr<T> sharedPtr = weakPtr.lock()) {
                sharedPtr->process(fileContent, fileName);
            }

            m_objectsToBeProcessed.erase(it++);    // or "it = m.erase(it)" since C++11

            objectsProcessed++;
            if (objectsProcessed > limit) {
                break;
            }
        }
    }
    void provideFile(std::string fileName, std::vector<unsigned char> &file) {
        trim(fileName);

//        std::cout << "filename:" << fileName << " hex: " << string_to_hex(fileName) << std::endl;
//        std::cout << "first in storage:" << m_cache.begin()->first << " hex: " << string_to_hex(m_cache.begin()->first) << std::endl << std::flush;
        if (m_cache.count(fileName) > 0) {
            m_objectsToBeProcessed[fileName] = file;
        }
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

        std::shared_ptr<T> sharedPtr = std::make_shared<T>();
        std::weak_ptr<T> weakPtr(sharedPtr);
        m_cache[fileName] = weakPtr;

        m_fileRequestProcessor->requestFile(fileName.c_str());

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

        std::shared_ptr<T> sharedPtr = std::make_shared<T>();
        std::weak_ptr<T> weakPtr(sharedPtr);
        m_cache[fileName] = weakPtr;


        m_fileRequestProcessor->requestFile(fileName.c_str());

        return sharedPtr;
    }

    void reject(std::string fileName) {
        trim(fileName);
    }

private:
    /*
    * Cache storage functions
    */


};


#endif //WOWVIEWERLIB_CACHE_H
