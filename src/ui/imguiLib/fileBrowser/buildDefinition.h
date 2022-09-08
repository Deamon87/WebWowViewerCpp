//
// Created by Deamon on 9/4/2022.
//

#ifndef AWEBWOWVIEWERCPP_BUILDDEFINITION_H
#define AWEBWOWVIEWERCPP_BUILDDEFINITION_H

#include <string>
#include <unordered_set>

struct BuildDefinition {
    std::string buildVersion;
    std::string buildConfig;
    std::string productName;
    std::string region;
    std::string installedLanguage;

    bool operator ==( const BuildDefinition& value) const {
        return
            (buildVersion == value.buildVersion) &&
            (buildConfig == value.buildConfig) &&
            (region == value.region);
            (productName == value.productName);
    }
};

struct BuildDefinitionHasher {
    std::size_t operator()(const BuildDefinition& k) const {
        using std::hash;
        return hash<std::string>{}(k.buildVersion) ^
               hash<std::string>{}(k.buildConfig) ^
               hash<std::string>{}(k.productName);

    };
};

typedef std::unordered_set<BuildDefinition, BuildDefinitionHasher> SetOfBuildDefs;

#endif //AWEBWOWVIEWERCPP_BUILDDEFINITION_H
