//
// Created by Deamon on 1/22/2021.
//

#ifndef AWEBWOWVIEWERCPP_CMINIMAPDATADB_H
#define AWEBWOWVIEWERCPP_CMINIMAPDATADB_H

#include "../../../3rdparty/sqlite_orm/sqlite_orm.h"
#include "../entities.h"
#include "../minimapGenerator.h"
namespace CMinimapDataDB {
    //Intermediate structures for storing/loading data from/to DB
    struct ADTBoundingBoxDB {
        int map_id;
        int adt_x;
        int adt_y;
        float adtMin_x;
        float adtMin_y;
        float adtMin_z;
        float adtMax_x;
        float adtMax_y;
        float adtMax_z;
    };

    struct MapRenderDefDB {
        int id = -1;
        int scenarioId = -1;

        int mapId = -1;

        double deltaX = 0.0f;
        double deltaY = 0.0f;
        double deltaZ = 0.0f;
    };

    struct MapRenderDefPhases {
        int mapRenderDefId = -1;
        int mapId = -1;
    };

    struct ExcludedADT_and_chunk_DB {
        int map_definition_id;
        int adt_x;
        int adt_y;
        int chunk_x;
        int chunk_y;
    };

    //Function that creates storage
    inline static auto makeStorage(const std::string &dataBaseFile) {
        using namespace sqlite_orm;
        return make_storage(dataBaseFile,
            make_table("scenarios",
                make_column("id", &ScenarioDef::id, autoincrement(), primary_key()),
                make_column("name", &ScenarioDef::name), make_column("orientation", &ScenarioDef::getOrientation, &ScenarioDef::setOrientation),
                make_column("close_ocean_color_r", &ScenarioDef::getCloseOceanColor_R, &ScenarioDef::setCloseOceanColor_R),
                make_column("close_ocean_color_g", &ScenarioDef::getCloseOceanColor_G, &ScenarioDef::setCloseOceanColor_G),
                make_column("close_ocean_color_b", &ScenarioDef::getCloseOceanColor_B, &ScenarioDef::setCloseOceanColor_B),
                make_column("world_coord_min_x", &ScenarioDef::getMinWowWorldCoordX, &ScenarioDef::setMinWowWorldCoordX),
                make_column("world_coord_min_y", &ScenarioDef::getMinWowWorldCoordY, &ScenarioDef::setMinWowWorldCoordY),
                make_column("world_coord_max_x", &ScenarioDef::getMaxWowWorldCoordX, &ScenarioDef::setMaxWowWorldCoordX),
                make_column("world_coord_max_y", &ScenarioDef::getMaxWowWorldCoordY, &ScenarioDef::setMaxWowWorldCoordY),
                make_column("image_height", &ScenarioDef::imageHeight),
                make_column("image_width", &ScenarioDef::imageWidth),
                make_column("zoom", &ScenarioDef::zoom),
                make_column("folder_to_save", &ScenarioDef::folderToSave)
            ),
            make_table("scenario_map_def",
                make_column("id", &MapRenderDefDB::id, autoincrement(), primary_key()),
                make_column("map_id", &MapRenderDefDB::mapId),
                make_column("scenario_id", &MapRenderDefDB::scenarioId),
                make_column("delta_x", &MapRenderDefDB::deltaX),
                make_column("delta_y", &MapRenderDefDB::deltaY),
                make_column("delta_z", &MapRenderDefDB::deltaZ)
            ),
            make_table("scenario_map_def_excluded_adt",
                make_column("map_definition_id", &ExcludedADT_and_chunk_DB::map_definition_id),
                make_column("adt_x", &ExcludedADT_and_chunk_DB::adt_x),
                make_column("adt_y", &ExcludedADT_and_chunk_DB::adt_y),
                make_column("chunk_x", &ExcludedADT_and_chunk_DB::chunk_x),
                make_column("chunk_y", &ExcludedADT_and_chunk_DB::chunk_y)
            ),
            make_table("map_bounding_box",
                make_column("map_id", &ADTBoundingBoxDB::map_id),
                make_column("adt_x", &ADTBoundingBoxDB::adt_x),
                make_column("adt_y", &ADTBoundingBoxDB::adt_y),
                make_column("adt_min_x", &ADTBoundingBoxDB::adtMin_x),
                make_column("adt_min_y", &ADTBoundingBoxDB::adtMin_y),
                make_column("adt_min_z", &ADTBoundingBoxDB::adtMin_z),
                make_column("adt_max_x", &ADTBoundingBoxDB::adtMax_x),
                make_column("adt_max_y", &ADTBoundingBoxDB::adtMax_y),
                make_column("adt_max_z", &ADTBoundingBoxDB::adtMax_z)
            )
        );
    };


    class DataBaseClass {
    public:
        explicit DataBaseClass(const std::string &fileName);

        //Scenarios
        void getScenarios(std::vector<ScenarioDef> &scenarioList);
        void saveScenario(ScenarioDef &scenario);
        void saveScenarios(std::vector<ScenarioDef> &scenarios);

        //Map defs
        void getMapRenderDef(int scenarioId, std::vector<MapRenderDef> &mapRenderDef);
        void saveMapRenderDef(int scenarioId, MapRenderDef &mapRenderDef);

        //Active phase

        //ADT Bounding Boxes
        void getAdtBoundingBoxes(MapRenderDef &mapRenderDef);
        void saveAdtBoundingBoxes(MapRenderDef &mapRenderDef);

        void getAdtExcluded(MapRenderDef &mapRenderDef);
        void saveAdtExcluded(MapRenderDef &mapRenderDef);

    private:
        decltype(makeStorage("")) m_storage;
    };
}


#endif //AWEBWOWVIEWERCPP_CMINIMAPDATADB_H
