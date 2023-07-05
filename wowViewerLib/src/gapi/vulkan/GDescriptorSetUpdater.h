//
// Created by Deamon on 7/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_GDESCRIPTORSETUPDATER_H
#define AWEBWOWVIEWERCPP_GDESCRIPTORSETUPDATER_H

#include <memory>
#include <functional>
#include "descriptorSets/GDescriptorSet.h"

class GDescriptorSetUpdater : public std::enable_shared_from_this<GDescriptorSetUpdater> {
public:
    GDescriptorSetUpdater();

    void addToUpdate(const std::shared_ptr<GDescriptorSet>& descSet);

    void updateDescriptorSets();
private:
    std::mutex descriptorUpdateMutex;

    std::vector<std::shared_ptr<GDescriptorSet>> m_descriptorSetToBeUpdated;
};


#endif //AWEBWOWVIEWERCPP_GDESCRIPTORSETUPDATER_H
