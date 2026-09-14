//
// Created by Deamon on 9/4/2022.
//

#ifndef AWEBWOWVIEWERCPP_BUILDDEFINITION_H
#define AWEBWOWVIEWERCPP_BUILDDEFINITION_H

#include <string>
#include <unordered_set>
#include "../../../3rdparty/nlohmann/json.hpp"

struct BuildDefinition {
    std::string buildVersion;
    std::string buildConfigHash;
    std::string cdnConfigHash;
    std::string productName;
    std::string cdnPath;
    // Optional space-separated list of CDN server hostnames to use instead of the ones
    // discovered from the battle.net patch service (e.g. "level3.blizzard.com").
    std::string cdnServersOverride;
    std::string region = "us";
    std::string installedLanguage;
    std::string armadilloKey;

    bool allowOnlineDownload = false;
    bool preferLowViolence = false;
    bool preferHiResTextures = true;

    bool useTactLocal = false;
    std::string localTactPath;

    bool isClassic = false;

    bool operator ==( const BuildDefinition& value) const {
        return
            (buildVersion == value.buildVersion) &&
            (buildConfigHash == value.buildConfigHash) &&
            (cdnConfigHash == value.cdnConfigHash) &&
            (productName == value.productName) &&
            (cdnPath == value.cdnPath) &&
            (cdnServersOverride == value.cdnServersOverride) &&
            (region == value.region) &&
            (installedLanguage == value.installedLanguage) &&
            (allowOnlineDownload == value.allowOnlineDownload) &&
            (preferLowViolence == value.preferLowViolence) &&
            (preferHiResTextures == value.preferHiResTextures) &&
            (armadilloKey == value.armadilloKey) &&
            (useTactLocal == value.useTactLocal) &&
            (localTactPath == value.localTactPath) &&
            (isClassic == value.isClassic);
    }
};

struct BuildDefinitionHasher {
    std::size_t operator()(const BuildDefinition& k) const {
        using std::hash;
        return hash<std::string>{}(k.buildVersion) ^
               hash<std::string>{}(k.buildConfigHash) ^
               hash<std::string>{}(k.cdnConfigHash) ^
               hash<std::string>{}(k.productName) ^
               hash<std::string>{}(k.cdnPath) ^
               hash<std::string>{}(k.cdnServersOverride) ^
               hash<std::string>{}(k.region) ^
               hash<std::string>{}(k.installedLanguage) ^
               hash<std::string>{}(k.armadilloKey);

    };
};

// JSON serialization for BuildDefinition.
// from_json only overwrites fields that are present in the JSON,
// so it can be used to apply partial overrides on top of an existing value.
inline void to_json(nlohmann::json& j, const BuildDefinition& b) {
    j = nlohmann::json{
            {"buildVersion", b.buildVersion},
            {"buildConfigHash", b.buildConfigHash},
            {"cdnConfigHash", b.cdnConfigHash},
            {"productName", b.productName},
            {"cdnPath", b.cdnPath},
            {"cdnServersOverride", b.cdnServersOverride},
            {"region", b.region},
            {"installedLanguage", b.installedLanguage},
            {"armadilloKey", b.armadilloKey},
            {"allowOnlineDownload", b.allowOnlineDownload},
            {"preferLowViolence", b.preferLowViolence},
            {"preferHiResTextures", b.preferHiResTextures},
            {"useTactLocal", b.useTactLocal},
            {"localTactPath", b.localTactPath},
            {"isClassic", b.isClassic}
    };
}

inline void from_json(const nlohmann::json& j, BuildDefinition& b) {
    if (j.contains("buildVersion")) j.at("buildVersion").get_to(b.buildVersion);
    if (j.contains("buildConfigHash")) j.at("buildConfigHash").get_to(b.buildConfigHash);
    if (j.contains("cdnConfigHash")) j.at("cdnConfigHash").get_to(b.cdnConfigHash);
    if (j.contains("productName")) j.at("productName").get_to(b.productName);
    if (j.contains("cdnPath")) j.at("cdnPath").get_to(b.cdnPath);
    if (j.contains("cdnServersOverride")) j.at("cdnServersOverride").get_to(b.cdnServersOverride);
    if (j.contains("region")) j.at("region").get_to(b.region);
    if (j.contains("installedLanguage")) j.at("installedLanguage").get_to(b.installedLanguage);
    if (j.contains("armadilloKey")) j.at("armadilloKey").get_to(b.armadilloKey);
    if (j.contains("allowOnlineDownload")) j.at("allowOnlineDownload").get_to(b.allowOnlineDownload);
    if (j.contains("preferLowViolence")) j.at("preferLowViolence").get_to(b.preferLowViolence);
    if (j.contains("preferHiResTextures")) j.at("preferHiResTextures").get_to(b.preferHiResTextures);
    if (j.contains("useTactLocal")) j.at("useTactLocal").get_to(b.useTactLocal);
    if (j.contains("localTactPath")) j.at("localTactPath").get_to(b.localTactPath);
    if (j.contains("isClassic")) j.at("isClassic").get_to(b.isClassic);
}

typedef std::unordered_set<BuildDefinition, BuildDefinitionHasher> SetOfBuildDefs;

#endif //AWEBWOWVIEWERCPP_BUILDDEFINITION_H
