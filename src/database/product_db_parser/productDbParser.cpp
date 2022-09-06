//
// Created by Deamon on 9/4/2022.
//

#include "productDbParser.h"
#include "protobuf_out/ProtoDatabase.pb.h"

SetOfBuildDefs ProductDbParser::parseFileContent(std::string &data) {

    SetOfBuildDefs result;

    Database db;
    db.ParseFromString(data);

    for (const auto &productInstall : db.productinstall()) {
        BuildDefinition buildDef;

        if (productInstall.has_productcode()) {
            buildDef.productName = productInstall.productcode();
        } else
            continue;

        if (productInstall.has_settings()) {
            auto &settings = productInstall.settings();
            if (settings.has_playregion()) {
                buildDef.region = settings.playregion();
            }
        }

        if (productInstall.has_cachedproductstate()) {
            auto &cachedProductState = productInstall.cachedproductstate();


            if (cachedProductState.has_baseproductstate()) {
                //Get the version from all the fields;
                auto &baseProductState = cachedProductState.baseproductstate();
                if (baseProductState.has_currentversion()) {
                    buildDef.buildVersion = baseProductState.currentversion();
                }
                if (baseProductState.has_currentversionstr()) {
                    buildDef.buildVersion = baseProductState.currentversionstr();
                }
                //if there is no version - continue;
                if (buildDef.buildVersion.empty())
                    continue;

                //Add completedKeys to set
                for (auto &completedKey : baseProductState.completedbuildkeys()) {
                    buildDef.buildConfig = completedKey;
                    result.insert(buildDef);
                }
                //Add build key to set
                for (auto &activeKey : baseProductState.activebuildkey()) {
                    buildDef.buildConfig = activeKey;
                    result.insert(buildDef);
                }
            }
        }
    }

    return result;
}
