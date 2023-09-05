//
// Created by Deamon on 2/16/2021.
//

#ifndef AWEBWOWVIEWERCPP_DATAEXPORTERCLASS_H
#define AWEBWOWVIEWERCPP_DATAEXPORTERCLASS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <mathfu/glsl_mappings.h>
#include "../../../wowViewerLib/src/engine/objects/m2/m2Object.h"
#include "../../database/csvtest/csv.h"
#include <fstream>
#include "../../../3rdparty/sqlite_orm/sqlite_orm.h"


namespace DataExporter {
    struct DBM2Batch : public M2Batch {
        int m2Id = -1;
        int batchIndex = -1;
    };

    struct DBSkinSection : public M2SkinSection{
        int m2Id = -1;
        int skinSectionIndex = -1;

        float getCenterPositionX() const {return this->centerPosition.x;};
        float getCenterPositionY() const {return this->centerPosition.y;};
        float getCenterPositionZ() const {return this->centerPosition.z;};

        float getSortCenterPositionX() const {return this->sortCenterPosition.x;};
        float getSortCenterPositionY() const {return this->sortCenterPosition.y;};
        float getSortCenterPositionZ() const {return this->sortCenterPosition.z;};

        void setCenterPositionX(float value) {this->sortCenterPosition.x = value;};
        void setCenterPositionY(float value) {this->sortCenterPosition.y = value;};
        void setCenterPositionZ(float value) {this->sortCenterPosition.z = value;};

        void setSortCenterPositionX(float value) {this->sortCenterPosition.x = value;};
        void setSortCenterPositionY(float value) {this->sortCenterPosition.y = value;};
        void setSortCenterPositionZ(float value) {this->sortCenterPosition.z = value;};
    };

    struct DBM2Material : M2Material{
        int m2Id = -1;
        int materialIndex;
    };

    struct DBM2RibbonData {
        int m2Id = -1;
        int ribbonIndex;
        uint32_t ribbonId;
        uint32_t boneIndex;
        int textureRows;
        int textureCols;
        int priorityPlane;
        int ribbonColorIndex;
        int textureTransformLookupIndex;
    };

    struct DBM2RibbonMaterial {
        int m2Id = -1;
        int ribbonIndex;
        int materialIndex;
        int m2MaterialIndex;
    };

    struct DBM2 {
        int m2Id = -1;
        int fileDataId;
        std::string fileName;
        int version;
        int global_flags;
        int global_loops_count;
        int sequences_count;
        int sequence_lookups_count;
        int bones_count;
        int key_bone_lookup_count;
        int vertices_count;
        int colors_count;
        int textures_count;
        int texture_weights_count;
        int texture_transforms_count;
        int replacable_texture_lookup_count;
        int materials_count;
        int bone_lookup_table_count;
        int texture_lookup_table_count;
        int tex_unit_lookup_table_count;
        int transparency_lookup_table_count;
        int texture_transforms_lookup_table_count;
        int collision_triangles_count;
        int collision_vertices_count;
        int collision_normals_count;
        int attachments_count;
        int attachment_lookup_table_count;
        int events_count;
        int lights_count;
        int cameras_count;
        int camera_lookup_table_count;
        int ribbon_emitters_count;
        int particle_emitters_count;
        int blend_map_overrides_count;
    };

    inline static auto makeStorage(const std::string &dataBaseFile) {
        using namespace sqlite_orm;
        return make_storage(dataBaseFile,
                make_table("M2",
                   make_column("id", &DBM2::m2Id, autoincrement(), primary_key()),
                   make_column("fileDataId", &DBM2::fileDataId),
                   make_column("fileName", &DBM2::fileName),
                   make_column("version", &DBM2::version),
                   make_column("global_flags", &DBM2::global_flags),
                   make_column("global_loops_count", &DBM2::global_loops_count),
                   make_column("sequences_count", &DBM2::sequences_count),
                   make_column("sequence_lookups_count", &DBM2::sequence_lookups_count),
                   make_column("bones_count", &DBM2::bones_count),
                   make_column("key_bone_lookup_count", &DBM2::key_bone_lookup_count),
                   make_column("vertices_count", &DBM2::vertices_count),
                   make_column("colors_count", &DBM2::colors_count),
                   make_column("textures_count", &DBM2::textures_count),
                   make_column("texture_weights_count", &DBM2::texture_weights_count),
                   make_column("texture_transforms_count", &DBM2::texture_transforms_count),
                   make_column("replacable_texture_lookup_count", &DBM2::replacable_texture_lookup_count),
#ifdef _MSC_VER
                   make_column("materials_count", &DBM2::materials_count),
                   make_column("bone_lookup_table_count", &DBM2::bone_lookup_table_count),
                   make_column("texture_lookup_table_count", &DBM2::texture_lookup_table_count),
                   make_column("tex_unit_lookup_table_count", &DBM2::tex_unit_lookup_table_count),
                   make_column("transparency_lookup_table_count", &DBM2::transparency_lookup_table_count),
                   make_column("texture_transforms_lookup_table_count", &DBM2::texture_transforms_lookup_table_count),
                   make_column("collision_triangles_count", &DBM2::collision_triangles_count),
                   make_column("collision_vertices_count", &DBM2::collision_vertices_count),
                   make_column("collision_normals_count", &DBM2::collision_normals_count),
                   make_column("attachments_count", &DBM2::attachments_count),
                   make_column("attachment_lookup_table_count", &DBM2::attachment_lookup_table_count),
                   make_column("events_count", &DBM2::events_count),
                   make_column("lights_count", &DBM2::lights_count),
                   make_column("cameras_count", &DBM2::cameras_count),
                   make_column("camera_lookup_table_count", &DBM2::camera_lookup_table_count),
#endif
                   make_column("ribbon_emitters_count", &DBM2::ribbon_emitters_count),
                   make_column("particle_emitters_count", &DBM2::particle_emitters_count)
                   //make_column("blend_map_overrides_count", &DBM2::blend_map_overrides_count)
                ),
                make_table("M2Batch",
                   make_column("m2Id", &DBM2Batch::m2Id),
                   make_column("batchIndex", &DBM2Batch::batchIndex),
                   make_column("_flags", &DBM2Batch::flags),
                   make_column("priorityPlane", &DBM2Batch::priorityPlane),
                   make_column("shader_id", &DBM2Batch::shader_id),
                   make_column("skinSectionIndex", &DBM2Batch::skinSectionIndex),
                   make_column("geosetIndex", &DBM2Batch::geosetIndex),
                   make_column("colorIndex", &DBM2Batch::colorIndex),
                   make_column("materialIndex", &DBM2Batch::materialIndex),
                   make_column("materialLayer", &DBM2Batch::materialLayer),
                   make_column("textureCount", &DBM2Batch::textureCount),
                   make_column("textureComboIndex", &DBM2Batch::textureComboIndex),
                   make_column("textureCoordComboIndex", &DBM2Batch::textureCoordComboIndex),
                   make_column("textureWeightComboIndex", &DBM2Batch::textureWeightComboIndex),
                   make_column("textureTransformComboIndex", &DBM2Batch::textureTransformComboIndex),
                   foreign_key(&DBM2Batch::m2Id).references(&DBM2::m2Id),
                   unique(&DBM2Batch::m2Id, &DBM2Batch::batchIndex)
                ),
                make_table("M2SkinSection",
                   make_column("m2Id", &DBSkinSection::m2Id),
                   make_column("skinSectionIndex", &DBSkinSection::skinSectionIndex),
#ifdef _MSC_VER
                   make_column("skinSectionId", &DBSkinSection::skinSectionId),
                   make_column("Level", &DBSkinSection::Level),
                   make_column("vertexStart", &DBSkinSection::vertexStart),
                   make_column("vertexCount", &DBSkinSection::vertexCount),
                   make_column("indexStart", &DBSkinSection::indexStart),
                   make_column("indexCount", &DBSkinSection::indexCount),
                   make_column("boneCount", &DBSkinSection::boneCount),
                   make_column("boneComboIndex", &DBSkinSection::boneComboIndex),
                   make_column("boneInfluences", &DBSkinSection::boneInfluences),
                   make_column("centerBoneIndex", &DBSkinSection::centerBoneIndex),

                   make_column("centerPosition_0", &DBSkinSection::getCenterPositionX, &DBSkinSection::setCenterPositionX),
                   make_column("centerPosition_1", &DBSkinSection::getCenterPositionY, &DBSkinSection::setCenterPositionY),
                   make_column("centerPosition_2", &DBSkinSection::getCenterPositionZ, &DBSkinSection::setCenterPositionZ),
                   make_column("sortCenterPosition_0", &DBSkinSection::getSortCenterPositionX, &DBSkinSection::setSortCenterPositionX),
                   make_column("sortCenterPosition_1", &DBSkinSection::getSortCenterPositionY, &DBSkinSection::setSortCenterPositionY),
                   make_column("sortCenterPosition_2", &DBSkinSection::getSortCenterPositionZ, &DBSkinSection::setSortCenterPositionZ),
#endif
                   make_column("sortRadius", &DBSkinSection::sortRadius),
                   foreign_key(&DBSkinSection::m2Id).references(&DBM2::m2Id),
                   unique(&DBSkinSection::m2Id, &DBSkinSection::skinSectionIndex)
                ),
                make_table("M2Material",
                   make_column("m2Id", &DBM2Material::m2Id),
                   make_column("materialIndex", &DBM2Material::materialIndex),
                   make_column("flags", &DBM2Material::flags),
                   make_column("blending_mode", &DBM2Material::blending_mode),
                   foreign_key(&DBM2Material::m2Id).references(&DBM2::m2Id),
                   unique(&DBM2Material::m2Id, &DBM2Material::materialIndex)
                ),
                make_table("M2RibbonData",
                   make_column("m2Id", &DBM2RibbonData::m2Id),
                   make_column("ribbonIndex", &DBM2RibbonData::ribbonIndex),
                   make_column("ribbonId", &DBM2RibbonData::ribbonId),
                   make_column("boneIndex", &DBM2RibbonData::boneIndex),
                   make_column("textureRows", &DBM2RibbonData::textureRows),
                   make_column("textureCols", &DBM2RibbonData::textureCols),
                   make_column("priorityPlane", &DBM2RibbonData::priorityPlane),
                   make_column("ribbonColorIndex", &DBM2RibbonData::ribbonColorIndex),
                   make_column("textureTransformLookupIndex", &DBM2RibbonData::textureTransformLookupIndex),

                   foreign_key(&DBM2RibbonData::m2Id).references(&DBM2::m2Id),
                   unique(&DBM2RibbonData::m2Id, &DBM2RibbonData::ribbonIndex)
                ),
                make_table("M2RibbonMaterial",
                   make_column("m2Id", &DBM2RibbonMaterial::m2Id),
                   make_column("ribbonIndex", &DBM2RibbonMaterial::ribbonIndex),
                   make_column("materialIndex", &DBM2RibbonMaterial::materialIndex),
                   make_column("m2MaterialIndex", &DBM2RibbonMaterial::m2MaterialIndex),

                   foreign_key(&DBM2RibbonMaterial::m2Id).references(&DBM2::m2Id),
                   unique(&DBM2RibbonMaterial::m2Id, &DBM2RibbonMaterial::ribbonIndex, &DBM2RibbonMaterial::materialIndex)
                )
        );
    };

    class DataExporterClass {
    public:
        DataExporterClass(HApiContainer apiContainer);
        void process();
        bool isDone() {
            return finished;
        }
    private:
        HApiContainer m_apiContainer;

        int currentFileDataId;
        std::string currentFileName;

        HM2Geom m_m2Geom = nullptr;
        HSkinGeom m_skinGeom = nullptr;
        io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>> *csv ;

        bool finished = false;
        int processedFiles = 0;

        std::ofstream outputLog;



        decltype(DataExporter::makeStorage("")) m_storage;

        int exportDBM2();

        void exportDBSkin(int id);
    };
}




#endif //AWEBWOWVIEWERCPP_DATAEXPORTERCLASS_H
