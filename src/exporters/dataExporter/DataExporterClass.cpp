//
// Created by Deamon on 2/16/2021.
//

#include "DataExporterClass.h"
#include "../../../wowViewerLib/src/include/string_utils.h"

DataExporter::DataExporterClass::DataExporterClass(HApiContainer apiContainer) : m_storage(DataExporter::makeStorage("dataExport.db3")) {
    m_apiContainer = apiContainer;
    processedFiles = 0;
    outputLog.open ("m2Log.txt");
    csv = new io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>>("listfile.csv");

    m_storage.pragma.synchronous(0);
    m_storage.pragma.journal_mode(sqlite_orm::journal_mode::MEMORY);
    m_storage.sync_schema();
}

void DataExporter::DataExporterClass::process() {
    if (finished)
        return;

    bool useFileId = true;

    if (m_m2Geom == nullptr) {

        while (csv->read_row(currentFileDataId, currentFileName)) {
            if (endsWith(currentFileName, ".m2"))
                break;
        }
        if (!endsWith(currentFileName, ".m2")) {
            std::cout << "export ended on " << currentFileName << std::endl;
            finished = true;
            return;
        }
        m_m2Geom = m_apiContainer->cacheStorage->getM2GeomCache()->getFileId(currentFileDataId);
    }

    if (m_m2Geom != nullptr && m_m2Geom->getStatus() == FileStatus::FSRejected)
        m_m2Geom = nullptr;

    if (m_m2Geom != nullptr && m_m2Geom->getStatus() == FileStatus::FSLoaded) {
        if (m_m2Geom->getM2Data()->magic != '02DM') {
            m_m2Geom = nullptr;
            m_skinGeom = nullptr;
            return;
        }
        if (m_skinGeom == nullptr) {
            Cache<SkinGeom> *skinGeomCache = m_apiContainer->cacheStorage->getSkinGeomCache();
            if (m_m2Geom->skinFileDataIDs.size() > 0) {
                assert(m_m2Geom->skinFileDataIDs.size() > 0);
                m_skinGeom = skinGeomCache->getFileId(m_m2Geom->skinFileDataIDs[0]);
//            } else if (!useFileId){
//                assert(m_nameTemplate.size() > 0);
//                std::string skinFileName = m_nameTemplate + "00.skin";
//                m_skinGeom = skinGeomCache->get(skinFileName);
            }
            return;
        }
        if (m_skinGeom->getStatus() == FileStatus::FSNotLoaded) return;
        if (m_skinGeom->getStatus() == FileStatus::FSRejected) {
            m_m2Geom = nullptr;
            m_skinGeom = nullptr;
            return;
        };

        int md2Id = exportDBM2();
        if (md2Id > 0) {
            exportDBSkin(md2Id);
        }

        if (m_m2Geom->m_m2Data->particle_emitters.size > 0 || m_m2Geom->m_m2Data->ribbon_emitters.size > 0) {

            outputLog << currentFileName << " ";
            for (int i = 0; i < m_m2Geom->m_m2Data->particle_emitters.size; i++) {
                auto pe = m_m2Geom->m_m2Data->particle_emitters[i];
                bool isRecursive = (i < m_m2Geom->recursiveFileDataIDs.size()) && (m_m2Geom->recursiveFileDataIDs[i] > 0);
                bool isModelEmitter = (i < m_m2Geom->particleModelFileDataIDs.size()) && (m_m2Geom->particleModelFileDataIDs[i] > 0);

                std::string emitterGenerator = "";
                if (pe->old.emitterType == 1) {
                    emitterGenerator = "PLANE";
                } else if (pe->old.emitterType == 2) {
                    emitterGenerator = "SPHERE";
                } else if (pe->old.emitterType == 3) {
                    emitterGenerator = "SPLINE";
                } else if (pe->old.emitterType == 4) {
                    emitterGenerator = "BONE";
                } else  {
                    emitterGenerator = "UNK_GENERATOR_"+std::to_string(pe->old.emitterType);
                }

                std::string txacVal = "";
                if (i < m_m2Geom->txacMParticle.size() && (m_m2Geom->txacMParticle[i].value != 0)) {
                    txacVal = "TXAC = ("
                        + std::to_string((int)m_m2Geom->txacMParticle[i].perByte[0])
                        + ", "
                        + std::to_string((int)m_m2Geom->txacMParticle[i].perByte[0])
                        + ") ";
                }

                outputLog << "( "
                    << (isRecursive ? ("RECURSIVE " + std::to_string(m_m2Geom->recursiveFileDataIDs[i]) + " ") : "")
                    << (isModelEmitter ? ("MODEL " + std::to_string(m_m2Geom->particleModelFileDataIDs[i]) + " ") : "")
                    << txacVal
                    << emitterGenerator
                    << " ) ";
            }

            for (int i = 0; i < m_m2Geom->m_m2Data->ribbon_emitters.size; i++) {
                auto ribbon = m_m2Geom->m_m2Data->ribbon_emitters[i];
                bool isRibbonSubstitution = ribbon->ribbonColorIndex > 0;
                bool isTextureTransform = (m_m2Geom->m_m2Data->global_flags.flag_unk_0x20000 > 0);

                if (isRibbonSubstitution || isTextureTransform) {
                    outputLog << "( ribbon " << i << " "
                        << (isRibbonSubstitution ? ("ribbonColorIndex = " + std::to_string(ribbon->ribbonColorIndex) + " " ) : "")
                        << (isTextureTransform ? ("textureTransformLookupIndex = " + std::to_string(ribbon->textureTransformLookupIndex) + " ") : "")
                        << " ) ";
                }
            }
            processedFiles++;
            outputLog << std::endl;
            if (processedFiles % 1000 == 0) {
                outputLog << std::flush;
            }

        }

        m_skinGeom = nullptr;
        m_m2Geom = nullptr;
    }
}

int DataExporter::DataExporterClass::exportDBM2() {
    const auto m2Data = m_m2Geom->m_m2Data;

    DBM2 dbm2;
    dbm2.fileDataId = currentFileDataId;
    dbm2.fileName = currentFileName;
    dbm2.global_flags = *(uint32_t *)&m2Data->global_flags;
    dbm2.global_loops_count = m2Data->global_loops.size;
    dbm2.sequences_count = m2Data->sequences.size;
    dbm2.sequence_lookups_count = m2Data->sequence_lookups.size;
    dbm2.bones_count = m2Data->bones.size;
    dbm2.key_bone_lookup_count = m2Data->key_bone_lookup.size;
    dbm2.vertices_count = m2Data->vertices.size;
    dbm2.colors_count = m2Data->colors.size;
    dbm2.textures_count = m2Data->textures.size;
    dbm2.texture_weights_count = m2Data->texture_weights.size;
    dbm2.texture_transforms_count = m2Data->texture_transforms.size;
    dbm2.replacable_texture_lookup_count = m2Data->replacable_texture_lookup.size;
    dbm2.materials_count = m2Data->materials.size;
    dbm2.bone_lookup_table_count = m2Data->bone_lookup_table.size;
    dbm2.texture_lookup_table_count = m2Data->texture_lookup_table.size;
    dbm2.tex_unit_lookup_table_count = m2Data->tex_unit_lookup_table.size;
    dbm2.transparency_lookup_table_count = m2Data->transparency_lookup_table.size;
    dbm2.texture_transforms_lookup_table_count = m2Data->texture_transforms_lookup_table.size;
    dbm2.collision_triangles_count = m2Data->collision_triangles.size;
    dbm2.collision_vertices_count = m2Data->collision_vertices.size;
    dbm2.collision_normals_count = m2Data->collision_normals.size;
    dbm2.attachments_count = m2Data->attachments.size;
    dbm2.attachment_lookup_table_count = m2Data->attachment_lookup_table.size;
    dbm2.events_count = m2Data->events.size;
    dbm2.lights_count = m2Data->lights.size;
    dbm2.cameras_count = m2Data->cameras.size;
    dbm2.camera_lookup_table_count = m2Data->camera_lookup_table.size;
    dbm2.ribbon_emitters_count = m2Data->ribbon_emitters.size;
    dbm2.particle_emitters_count = m2Data->particle_emitters.size;
    dbm2.blend_map_overrides_count = m2Data->blend_map_overrides.size;

    return m_storage.insert<DataExporter::DBM2>(dbm2);
}

void DataExporter::DataExporterClass::exportDBSkin(int id) {
    auto skinProfile = m_skinGeom->getSkinData();
    M2Array<M2Batch> &batches = skinProfile->batches;
    for (int batchIndex = 0; batchIndex < batches.size; batchIndex++) {
        auto const batch = skinProfile->batches.getElement(batchIndex);

        DataExporter::DBM2Batch dbm2Batch;
        *dynamic_cast<M2Batch*>(&dbm2Batch) = *batch;

        dbm2Batch.m2Id = id;
        dbm2Batch.batchIndex = batchIndex;

        m_storage.insert(dbm2Batch);
    }
}
