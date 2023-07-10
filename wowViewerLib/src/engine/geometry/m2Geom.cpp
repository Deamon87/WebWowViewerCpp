//
// Created by deamon on 19.06.17.
//

#include "m2Geom.h"
#include "skinGeom.h"
#include "../shader/ShaderDefinitions.h"
#include <atomic>

std::atomic<int> m2SizeLoaded = 0;

chunkDef<M2Geom> M2Geom::m2FileTable = {
    [](M2Geom& file, ChunkData& chunkData){},
    {
        {
            '12DM',
                {
                    [](M2Geom &file, ChunkData &chunkData) {
                        debuglog("Entered MD21");
                        chunkData.readValue(file.m_m2Data);

                        chunkData.bytesRead = chunkData.chunkLen;
                    }
                }
        },
        {
            'DIFS',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered SFID");
                    file.skinFileDataIDs =
                            std::vector<uint32_t>(
                                    file.m_m2Data->num_skin_profiles);

                    for (int i = 0; i < file.skinFileDataIDs.size(); i++) {
                        chunkData.readValue(file.skinFileDataIDs[i]);
                    }
                }
            }
        },
        {
            'DIXT',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered DIXT");
                    file.textureFileDataIDs =
                            std::vector<uint32_t>(
                                    (unsigned long) (chunkData.chunkLen / 4));

                    for (int i = 0; i < file.textureFileDataIDs.size(); i++) {
                        chunkData.readValue(file.textureFileDataIDs[i]);
                    }
                }
            }
        },
        {
            '2PXE',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered 2PXE");
                    chunkData.readValue(file.exp2);
                }
            }
        },
        {
            'DIKS',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered DIKS");
                    chunkData.readValue(file.m_skid);
                }
            }
        },
        {
            'DIFA',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered AFID");
                    file.animationFileDataIDs =
                            std::vector<M2_AFID>(
                                    (unsigned long) (chunkData.chunkLen / sizeof(M2_AFID)));

                    for (int i = 0; i < file.animationFileDataIDs.size(); i++) {
                        chunkData.readValue(file.animationFileDataIDs[i]);
                    }
                }
            }
        },
        {
            'DIPR',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered RPID");
                    file.recursiveFileDataIDs =
                        std::vector<uint32_t>(chunkData.chunkLen / 4);

                    for (int i = 0; i < file.recursiveFileDataIDs.size(); i++) {
                        chunkData.readValue(file.recursiveFileDataIDs[i]);
                    }

                }
            }
        },
        {
            'DIPG',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered GPID");
                    file.particleModelFileDataIDs =
                        std::vector<uint32_t>(chunkData.chunkLen / 4);

                    for (int i = 0; i < file.particleModelFileDataIDs.size(); i++) {
                        chunkData.readValue(file.particleModelFileDataIDs[i]);
                    }

                }
            }
        },
        {
            'CAXT',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered CAXT");
                    file.txacMesh = std::vector<TXAC>(file.m_m2Data->materials.size);
                    file.txacMParticle = std::vector<TXAC>(file.m_m2Data->particle_emitters.size);

                    for (int i = 0; i < file.txacMesh.size(); i++) {
                        chunkData.readValue(file.txacMesh[i]);
                    }

                    for (int i = 0; i < file.txacMParticle.size(); i++) {
                        chunkData.readValue(file.txacMParticle[i]);
                    }

                }
            }
        },
        {
            '3VFW',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered 3VFW");
                    chunkData.readValue(file.m_wfv3);

                }
            }
        },
        {
            '1VFW',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered 1VFW");
                    float bumpScale;
                    chunkData.readValue(bumpScale);
                    WaterFallDataV3 *dataV3 = new WaterFallDataV3();
                    dataV3->bumpScale = bumpScale;
                    dataV3->values0_x = 0;
                    dataV3->values0_y = 0.5;
                    dataV3->values0_z = 0.0;
                    dataV3->value1_w = 1.0f;
                    dataV3->values0_w = 0.5;
                    dataV3->value1_x = 0;
                    dataV3->value1_y = 0;
                    dataV3->value2_w = 0;
                    dataV3->value3_y = 0;
                    dataV3->value3_x = 0;
                    dataV3->basecolor = {0xff, 0xff, 0xff, 0xff};
                    dataV3->flags = 0;
                    dataV3->unk0 = 0;
                    dataV3->values3_w = 0;
                    dataV3->values3_z = 0;
                    dataV3->values4_y = 0;

                    file.m_wfv1 = dataV3;
                }
            }
        },
        {
            '1DGP',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered PGD1");
                    chunkData.readValue(file.particleGeosetData);
                }
            }
        },
        {
            'CBAP',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered PABC");
                    int arrayLen = chunkData.chunkLen >> 1;
                    file.blackListAnimations = std::vector<uint16_t>(arrayLen);

                    for (int i = 0; i < arrayLen; i++) {
                        chunkData.readValue(file.blackListAnimations[i]);
                    }
                }
            }
        },
        {
            'FGDE',
            {
                [](M2Geom &file, ChunkData &chunkData) {
                    debuglog("Entered EDGF");
                    int arrayLen = chunkData.chunkLen / sizeof(EDGF);
                    file.edgf_count = arrayLen;
                    chunkData.readValues(file.edgf, arrayLen);
                    debuglog("Leaving EDGF");
                }
            }
        },
    }
};


void M2Geom::process(HFileContent m2File, const std::string &fileName) {
    this->m2File = m2File;

    m2SizeLoaded.fetch_add(m2File->size());

    auto &m2FileData = *m2File.get();
    if (
        m2FileData[0] == 'M' &&
        m2FileData[1] == 'D' &&
        m2FileData[2] == '2' &&
        m2FileData[3] == '1'
            ) {
        CChunkFileReader reader(*this->m2File.get());
        reader.processFile(*this, &M2Geom::m2FileTable);
    } else {
        M2Data *m2Header = (M2Data *) this->m2File->data();
        this->m_m2Data = m2Header;
    }
    M2Data *m2Header = this->m_m2Data;

    //Step 1: Init all m2Arrays
    m2Header->global_loops.initM2Array(m2Header);
    m2Header->sequences.initM2Array(m2Header);
    m2Header->sequence_lookups.initM2Array(m2Header);
    m2Header->bones.initM2Array(m2Header);
    m2Header->key_bone_lookup.initM2Array(m2Header);
    m2Header->vertices.initM2Array(m2Header);
    m2Header->colors.initM2Array(m2Header);
    m2Header->textures.initM2Array(m2Header);
    m2Header->texture_weights.initM2Array(m2Header);
    m2Header->texture_transforms.initM2Array(m2Header);
    m2Header->replacable_texture_lookup.initM2Array(m2Header);
    m2Header->materials.initM2Array(m2Header);
    m2Header->bone_lookup_table.initM2Array(m2Header);
    m2Header->texture_lookup_table.initM2Array(m2Header);
    m2Header->tex_unit_lookup_table.initM2Array(m2Header);
    m2Header->transparency_lookup_table.initM2Array(m2Header);
    m2Header->texture_transforms_lookup_table.initM2Array(m2Header);
    m2Header->collision_triangles.initM2Array(m2Header);
    m2Header->collision_vertices.initM2Array(m2Header);
    m2Header->collision_normals.initM2Array(m2Header);
    m2Header->attachments.initM2Array(m2Header);
    m2Header->attachment_lookup_table.initM2Array(m2Header);
    m2Header->events.initM2Array(m2Header);
    m2Header->lights.initM2Array(m2Header);
    m2Header->cameras.initM2Array(m2Header);
    m2Header->camera_lookup_table.initM2Array(m2Header);
    m2Header->ribbon_emitters.initM2Array(m2Header);
    m2Header->particle_emitters.initM2Array(m2Header);

    if (m2Header->lights.size > 0) {
        int directLights = 0;
        int pointLights = 0;

        for (int i = 0; i < m2Header->lights.size; i++) {
            switch(m2Header->lights.getElement(i)->type) {
                case 0 : {
                    directLights++;
                    break;
                }
                case 1 : {
                    pointLights++;
                    break;
                }
                default:
                    std::cout << "Found unk light type " << m2Header->lights.getElement(i)->type << std::endl;
                    break;
            }
        }

        //std::cout << "Found " << directLights << " direct lights and " << pointLights << " point lights" << std::endl;
    }

    if (m2Header->global_flags.flag_has_blend_maps) {
        m2Header->blend_map_overrides.initM2Array(m2Header);
    }
    initM2ParticlePartTracks(m2Header);
    initM2Textures(m2Header, m2Header->textures);

    if (exp2 != nullptr) {
        initEXP2(exp2);
    }

    initTracks(nullptr);

    if (m_wfv3 != nullptr)  {
        m2Header->textures.size = 4;
    }

    if (particleGeosetData != nullptr) {
        particleGeosetData->pgd.initM2Array(&particleGeosetData->pgd);
    }

    //Step 2: init tracks
    fsStatus = FileStatus::FSLoaded;
}

void M2Geom::initTracks(CM2SequenceLoad * cm2SequenceLoad) {

    M2Data *m2Header = this->m_m2Data;

    initCompBones(m2Header, &m2Header->bones, &m2Header->sequences, cm2SequenceLoad);
    initM2Color(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2TextureWeight(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2TextureTransform(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2Attachment(m2Header, &m2Header->attachments, &m2Header->sequences, cm2SequenceLoad);
    initM2Event(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2Light(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2Particle(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2Ribbon(m2Header, &m2Header->sequences, cm2SequenceLoad);
    initM2Camera(m2Header, &m2Header->sequences, cm2SequenceLoad);
}

HGVertexBuffer M2Geom::getVBO(const HMapSceneBufferCreate &sceneRenderer) {
    if (vertexVbo == nullptr) {
        if (m_m2Data->vertices.size == 0) {
            return nullptr;
        }

        vertexVbo = sceneRenderer->createM2VertexBuffer(m_m2Data->vertices.size*sizeof(M2Vertex));
        vertexVbo->uploadData(
            m_m2Data->vertices.getElement(0),
            m_m2Data->vertices.size*sizeof(M2Vertex));
    }

    return vertexVbo;
}

std::array<HGVertexBufferBindings, IDevice::MAX_FRAMES_IN_FLIGHT>
M2Geom::createDynamicVao(const HMapSceneBufferCreate &sceneRenderer,
                         std::array<HGVertexBufferDynamic, IDevice::MAX_FRAMES_IN_FLIGHT> &dynVBOs,
    SkinGeom *skinGeom, M2SkinSection *skinSection) {
    //1. Create index buffer
    std::vector<uint16_t > indicies(skinSection->indexCount);


    int minIndex = 9999999;
    int maxIndex = 0;
    for (int i = 0; i < skinSection->indexCount; i++) {
        int index = skinSection->indexStart + (skinSection->Level << 16) + i;
        indicies[i] =
            *skinGeom->getSkinData()->vertices.getElement(*skinGeom->getSkinData()->indices.getElement(index))
            - skinSection->vertexStart;

        minIndex = std::min<int>(minIndex, indicies[i]);
        maxIndex = std::max<int>(maxIndex, indicies[i]);
    }

//    std::cout
//        << " indexCount = " << skinSection->indexCount
//        << " minIndex = " << minIndex
//        << " maxIndex = " << maxIndex
//        << " vertexStart = " << skinSection->vertexStart
//        << " vertexCount = " << skinSection->vertexCount
//        << std::endl;


    auto indexIbo = sceneRenderer->createM2IndexBuffer(indicies.size() * sizeof(uint16_t));
    indexIbo->uploadData(
        &indicies[0],
        indicies.size() * sizeof(uint16_t));

    std::array<HGVertexBufferBindings, IDevice::MAX_FRAMES_IN_FLIGHT> result;
    for (int i = 0 ; i < IDevice::MAX_FRAMES_IN_FLIGHT; i ++) {
        //2.1. Create vertex buffer
        auto vertexVboDyn = sceneRenderer->createM2VertexBuffer(skinSection->vertexCount * sizeof(M2Vertex));
        dynVBOs[i] = vertexVboDyn;

        //2.2 Create VAO
        HGVertexBufferBindings bufferBindings = sceneRenderer->createM2VAO(vertexVboDyn, indexIbo);
        result[i] = bufferBindings;
    }


    return result;
}

HGVertexBufferBindings M2Geom::getVAO(const HMapSceneBufferCreate &sceneRenderer, SkinGeom *skinGeom) {
    HGVertexBufferBindings bufferBindings = nullptr;
    if (vaoMap.find(skinGeom) != vaoMap.end()) {
        bufferBindings = vaoMap.at(skinGeom);
    } else {
        HGVertexBuffer vboBuffer = this->getVBO(sceneRenderer);
        if (vboBuffer == nullptr) {
            vaoMap[skinGeom] = nullptr;
            return nullptr;
        }

        HGIndexBuffer iboBuffer = skinGeom->getIBO(sceneRenderer);

        bufferBindings = sceneRenderer->createM2VAO(vertexVbo, iboBuffer);
        vaoMap[skinGeom] = bufferBindings;
    }

    return bufferBindings;
}



void M2Geom::loadLowPriority(const HApiContainer& m_api, uint32_t animationId, uint32_t variationId) {
    int animationIndex = findAnimationIndex(animationId, &m_m2Data->sequence_lookups, &m_m2Data->sequences);
    if (animationIndex < 0) return;

    AnimCacheRecord animCacheRecord;
    animCacheRecord.animationId = animationId;
    animCacheRecord.variationId = variationId;
    auto it = loadedAnimationMap.find(animCacheRecord);
    if (it != loadedAnimationMap.end()) return;


    while (m_m2Data->sequences[animationIndex]->variationIndex != variationId) {
        animationIndex = m_m2Data->sequences[animationIndex]->variationNext;

        if (animationIndex <= 0) return;
    }

    if ((m_m2Data->sequences[animationIndex]->flags & 0x20) > 0) return;

    int animationFileDataId = 0;
    if (animationFileDataIDs.size() > 0) {
        for (const auto &record : animationFileDataIDs) {
            if (record.anim_id == animationId && record.sub_anim_id == variationId) {
                animationFileDataId = record.file_id;
            }
        }
    }
    std::shared_ptr<AnimFile> animFile = nullptr;
    if (animationFileDataId != 0) {
        animFile = m_api->cacheStorage->getAnimCache()->getFileId(animationFileDataId);
    } else if (!useFileId) {
        char buffer[1024];
        std::snprintf(buffer, 1024, "%s%04d-%02d.anim", m_nameTemplate.c_str(), animationId, variationId);

        animFile = m_api->cacheStorage->getAnimCache()->get(buffer);
    }
    if (animFile == nullptr) return;

    animFile->setPostLoad([this, animationIndex, animFile]() -> void {
        CM2SequenceLoad cm2SequenceLoad;
        cm2SequenceLoad.animFileDataBlob = animFile->m_animFileDataBlob;
        cm2SequenceLoad.animationIndex = animationIndex;

        initTracks(&cm2SequenceLoad);
        m_m2Data->sequences[animationIndex]->flags |= 0x20;
    });
    loadedAnimationMap[animCacheRecord] = animFile;
}

M2Geom::~M2Geom() {
    m2SizeLoaded.fetch_add(-m2File->size());
}
