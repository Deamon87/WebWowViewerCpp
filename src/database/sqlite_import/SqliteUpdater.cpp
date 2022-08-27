//
// Created by Deamon on 8/24/2022.
//

#include "SqliteUpdater.h"
#include <string>

struct {
    int fileDataId;
    std::string tableName;
} requiredTables[14] = {
        {1353545, "AreaTable"},
        {1375579, "Light"},
        {1334669, "LightParams"},
        {1308501, "LightSkybox"},
        {1371380, "LiquidType"},
        {2261065, "LiquidTypeXTexture"},
        {1132538, "LiquidMaterial"},
        {1308058, "LiquidObject"},
        {1371380, "LiquidType"},
        {2261065, "LiquidTypeXTexture"},
        {1349477, "Map"},
        {1355528, "WMOAreaTable"},
        {1310253, "ZoneLight"},
        {1310256, "ZoneLightPoint"},
};


SqliteUpdater::SqliteUpdater(HWoWFilesCacheStorage cacheStorage) {

}

void SqliteUpdater::process(std::string currentMessage) {

}
