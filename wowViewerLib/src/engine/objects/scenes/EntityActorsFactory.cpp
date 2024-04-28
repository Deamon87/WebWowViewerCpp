//
// Created by Deamon on 12/6/2023.
//

#include "EntityActorsFactory.h"
#include "../../../gapi/interface/IDevice.h"

#include <functional>
#include <list>

struct EntityDeallocationRecord {
    unsigned int frameNumberToDoAt;
    std::function<void()> callback;
};

std::mutex m_listOfEntityDeallocatorsAccessMtx;
std::list<EntityDeallocationRecord> listOfEntityDeallocators;

void addEntityDeallocationRecord(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(m_listOfEntityDeallocatorsAccessMtx);
    EntityDeallocationRecord dr;
    dr.frameNumberToDoAt = IDevice::getCurrentProcessingFrameNumber() + IDevice::MAX_FRAMES_IN_FLIGHT+1;
    dr.callback = callback;
    listOfEntityDeallocators.push_back(dr);
};

void executeEntityDeallocators() {
    std::unique_lock<std::mutex> lock(m_listOfEntityDeallocatorsAccessMtx, std::defer_lock);

    lock.lock();
    auto m_frameNumber = IDevice::getCurrentProcessingFrameNumber();
    while ((!listOfEntityDeallocators.empty()) && (listOfEntityDeallocators.front().frameNumberToDoAt <= m_frameNumber)) {
        auto stuff = listOfEntityDeallocators.front();

        if (stuff.callback != nullptr) {
            lock.unlock();
            stuff.callback();
            lock.lock();
        }

        listOfEntityDeallocators.pop_front();
    }
    lock.unlock();
}