//
// Created by Claude on 2026.
//

#ifndef AWEBWOWVIEWERCPP_CASCSTORAGEENTRY_H
#define AWEBWOWVIEWERCPP_CASCSTORAGEENTRY_H

#include <string>
#include "../../../database/buildInfoParser/buildDefinition.h"
#include "../../../../3rdparty/nlohmann/json.hpp"

// Storage entry combining BuildDefinition with the local folder path
struct CascStorageEntry {
    BuildDefinition buildDef;
    std::string cascLocalFolder;

    // Display name for the list (computed from buildDef)
    std::string getDisplayName() const {
        std::string name = buildDef.productName;
        if (!buildDef.buildVersion.empty()) {
            name += " - " + buildDef.buildVersion;
        }
        if (!cascLocalFolder.empty()) {
            // Show last part of path
            size_t pos = cascLocalFolder.find_last_of("/\\");
            if (pos != std::string::npos) {
                name += " (" + cascLocalFolder.substr(pos + 1) + ")";
            } else {
                name += " (" + cascLocalFolder + ")";
            }
        }
        return name;
    }
};



// JSON serialization for CascStorageEntry
inline void to_json(nlohmann::json& j, const CascStorageEntry& e) {
    j = nlohmann::json{
        {"buildDef", e.buildDef},
        {"cascLocalFolder", e.cascLocalFolder}
    };
}

inline void from_json(const nlohmann::json& j, CascStorageEntry& e) {
    if (j.contains("buildDef")) j.at("buildDef").get_to(e.buildDef);
    if (j.contains("cascLocalFolder")) j.at("cascLocalFolder").get_to(e.cascLocalFolder);
}

#endif //AWEBWOWVIEWERCPP_CASCSTORAGEENTRY_H
