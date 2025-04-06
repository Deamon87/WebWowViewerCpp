//
// Created by Deamon on 7/5/2023.
//

#include "GDescriptorSetUpdater.h"

GDescriptorSetUpdater::GDescriptorSetUpdater() {
}

void GDescriptorSetUpdater::addToUpdate(const std::shared_ptr<GDescriptorSet>& descSet) {
    std::lock_guard lock(descriptorUpdateMutex);
    m_descriptorSetToBeUpdated.push_back(descSet);
}

void GDescriptorSetUpdater::updateDescriptorSets() {
    std::lock_guard lock(descriptorUpdateMutex);

    std::sort(m_descriptorSetToBeUpdated.begin(), m_descriptorSetToBeUpdated.end());
    m_descriptorSetToBeUpdated.erase(std::unique(m_descriptorSetToBeUpdated.begin(), m_descriptorSetToBeUpdated.end()), m_descriptorSetToBeUpdated.end());

    for (const auto& descriptor : m_descriptorSetToBeUpdated) {
        descriptor->update();
    }

    m_descriptorSetToBeUpdated.clear();
}
