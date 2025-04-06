//
// Created by Deamon on 9/4/2022.
//

#include "buildInfoParser.h"
#include "../csvtest/csv.h"
#include "../../../wowViewerLib/src/include/string_utils.h"
#include "../../../wowViewerLib/src/engine/stringTrim.h"

#include "buildDefinition.h"


std::string getLangFromTags(const std::string &s) {
    std::vector<std::string> values;
    tokenize(s, ":", values);
    for (const auto &tagLine : values) {
        std::vector<std::string> tagVals;

        tokenize(tagLine, "?", tagVals);
        for (const auto &tag : tagVals) {
            std::string tagTrimmed = tag;

            if (endsWith(tag, "text")) {
                ltrim(tagTrimmed); 
                std::vector<std::string> tagParams;
                tokenize(tagTrimmed, " ", tagParams);
                return tagParams[0];
            }
        }
    }
    return "";
}

SetOfBuildDefs BuildInfoParser::parseFileContent(std::string &data) {
    SetOfBuildDefs result;

    auto lineReader = io::LineReader(".build.info", data.data(), data.data()+data.size());
    std::string header = lineReader.next_line();
    std::vector<std::string> headerNames;
    tokenize(header, "|", headerNames);

    int branchIndex = -1;
    int productIndex = -1;
    int buildKeyIndex = -1;
    int versionIndex = -1;
    int tagsIndex = -1;
    for (int i = 0; i < headerNames.size(); i++) {
        if (startsWith(headerNames[i], "Branch")) {
            branchIndex = i;
        }
        if (startsWith(headerNames[i], "Product")) {
            productIndex = i;
        }
        if (startsWith(headerNames[i], "Build Key")) {
            buildKeyIndex = i;
        }
        if (startsWith(headerNames[i], "Version")) {
            versionIndex = i;
        }
        if (startsWith(headerNames[i], "Tags")) {
            tagsIndex = i;
        }
    }
    if (productIndex == -1 || buildKeyIndex == -1 || versionIndex == -1 || branchIndex == -1)
        return {};

    while(char*line = lineReader.next_line()) {
        BuildDefinition buildDef;
        std::string content = line;
        std::vector<std::string> values;
        tokenize(content, "|", values);

        if (branchIndex < values.size())
            buildDef.region = values[branchIndex];

        if (productIndex < values.size()) {
            buildDef.productName = values[productIndex];
        }

        if (buildKeyIndex < values.size())
            buildDef.buildConfig = values[buildKeyIndex];

        if (versionIndex < values.size())
            buildDef.buildVersion = values[versionIndex];

        if (tagsIndex < values.size())
            buildDef.installedLanguage = getLangFromTags(values[tagsIndex]);


        result.insert(buildDef);
    }

    return result;
}