//
// Created by Deamon on 12/6/2023.
//

#include "EntityActorsFactory.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/interface/FrameContext.h"

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
    dr.frameNumberToDoAt = FrameContext::getCurrentProcessingFrameNumber() + IDevice::MAX_FRAMES_IN_FLIGHT+1;
    dr.callback = callback;
    listOfEntityDeallocators.push_back(dr);
};

void executeEntityDeallocators(bool forceDealloc) {
    std::list<EntityDeallocationRecord> toRun;

    do {
        {
            std::lock_guard<std::mutex> lock(m_listOfEntityDeallocatorsAccessMtx);
            auto frame = FrameContext::getCurrentProcessingFrameNumber();

            while (!listOfEntityDeallocators.empty() &&
                (forceDealloc || listOfEntityDeallocators.front().frameNumberToDoAt <= frame)) {
                toRun.push_back(std::move(listOfEntityDeallocators.front()));
                listOfEntityDeallocators.pop_front();
            }
        }

        // Run callbacks without holding the lock
        for (auto& item : toRun) {
            if (item.callback)
                item.callback();
        }
        toRun.clear();
    } while ((!listOfEntityDeallocators.empty()) && forceDealloc);
}