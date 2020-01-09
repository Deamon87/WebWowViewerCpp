//
// Created by deamon on 19.06.17.
//

#include "m2Geom.h"
#include "skinGeom.h"
#include "../shader/ShaderDefinitions.h"
#include "../../gapi/interface/IDevice.h"
#include "../../include/wowScene.h"

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
                    chunkData.readValue(file.exp2Records);
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
        }
    }
};


//    /*
//     {name: "pos",           type : "vector3f"},           0+12 = 12
//     {name: "bonesWeight",   type : "uint8Array", len: 4}, 12+4 = 16
//     {name: "bones",         type : "uint8Array", len: 4}, 16+4 = 20
//     {name: "normal",        type : "vector3f"},           20+12 = 32
//     {name: "textureX",      type : "float32"},            32+4 = 36
//     {name: "textureY",      type : "float32"},            36+4 = 40
//     {name : "textureX2",    type : "float32"},            40+4 = 44
//     {name : "textureY2",    type : "float32"}             44+4 = 48
//     */
static GBufferBinding staticM2Bindings[6] = {
        {+m2Shader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, 48, 0 },
        {+m2Shader::Attribute::boneWeights, 4, GBindingType::GUNSIGNED_BYTE, true, 48, 12},  // bonesWeight
        {+m2Shader::Attribute::bones, 4, GBindingType::GUNSIGNED_BYTE, false, 48, 16},  // bones
        {+m2Shader::Attribute::aNormal, 3, GBindingType::GFLOAT, false, 48, 20}, // normal
        {+m2Shader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, 48, 32}, // texcoord
        {+m2Shader::Attribute::aTexCoord2, 2, GBindingType::GFLOAT, false, 48, 40} // texcoord
};

void initEXP2(M2Array<Exp2Record> *exp2Records) {
    exp2Records->initM2Array(exp2Records);
    for (int i = 0; i < exp2Records->size; i++) {
        Exp2Record *exp2Record = exp2Records->getElement(i);
        exp2Record->unk3.timestamps.initM2Array(exp2Records);
        exp2Record->unk3.values.initM2Array(exp2Records);
    }
}

void initM2Textures(M2Data *m2Header){
    int32_t texturesSize = m2Header->textures.size;
    for (int i = 0; i < texturesSize; i++) {
        M2Texture *texture = m2Header->textures.getElement(i);
        texture->filename.initM2Array(m2Header);
    }
}
void initCompBones(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad){
    int32_t bonesSize = m2Header->bones.size;
    for (int i = 0; i < bonesSize; i++) {
        M2CompBone *compBone = m2Header->bones.getElement(i);
        compBone->translation.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        compBone->rotation.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        compBone->scaling.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2Color(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t m2ColorSize = m2Header->colors.size;
    for (int i = 0; i < m2ColorSize; i++) {
        M2Color * m2Color = m2Header->colors.getElement(i);
        m2Color->alpha.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        m2Color->color.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2TextureWeight(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t textureWeightSize = m2Header->texture_weights.size;
    for (int i = 0; i < textureWeightSize; i++) {
        M2TextureWeight * textureWeight = m2Header->texture_weights.getElement(i);
        textureWeight->weight.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2TextureTransform(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t textureTransformSize = m2Header->texture_transforms.size;
    for (int i = 0; i < textureTransformSize; i++) {
        M2TextureTransform * textureTransform = m2Header->texture_transforms.getElement(i);
        textureTransform->translation.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        textureTransform->rotation.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        textureTransform->scaling.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2Attachment(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t attachmentCount = m2Header->attachments.size;
    for (int i = 0; i < attachmentCount; i++) {
        M2Attachment *attachment = m2Header->attachments.getElement(i);
        attachment->animate_attached.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2Event(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t eventCount = m2Header->events.size;
    for (int i = 0; i < eventCount; i++) {
        M2Event *event = m2Header->events.getElement(i);
        event->enabled.initTrackBase(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2Light(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t lightCount = m2Header->lights.size;
    for (int i = 0; i < lightCount; i++) {
        M2Light *light = m2Header->lights.getElement(i);
        light->ambient_color.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        light->ambient_intensity.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        light->diffuse_color.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        light->diffuse_intensity.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        light->attenuation_start.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        light->attenuation_end.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        light->visibility.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}

void initM2Particle(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    assert(sizeof(M2Particle) == 492);
    int32_t particleEmitterCount = m2Header->particle_emitters.size;
    for (int i = 0; i < particleEmitterCount; i++) {
        M2Particle *particleEmitter = m2Header->particle_emitters.getElement(i);

        particleEmitter->old.emissionSpeed.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.speedVariation.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.verticalRange.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.horizontalRange.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.gravity.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.lifespan.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.emissionRate.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.emissionAreaLength.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.emissionAreaWidth.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        particleEmitter->old.zSource.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);

        particleEmitter->old.enabledIn.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2Ribbon(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t ribbonEmitterCount = m2Header->ribbon_emitters.size;
    for (int i = 0; i < ribbonEmitterCount; i++) {
        M2Ribbon *ribbonEmitter = m2Header->ribbon_emitters.getElement(i);

        ribbonEmitter->textureIndices.initM2Array(m2Header);
        ribbonEmitter->materialIndices.initM2Array(m2Header);

        ribbonEmitter->colorTrack.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        ribbonEmitter->alphaTrack.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        ribbonEmitter->heightAboveTrack.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        ribbonEmitter->heightBelowTrack.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        ribbonEmitter->texSlotTrack.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        ribbonEmitter->visibilityTrack.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}
void initM2Camera(M2Data *m2Header, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t cameraCount = m2Header->cameras.size;
    for (int i = 0; i < cameraCount; i++) {
        M2Camera *camera = m2Header->cameras.getElement(i);
        camera->positions.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        camera->target_position.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        camera->roll.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
        camera->FoV.initTrack(m2Header, m2Header->sequences, cm2SequenceLoad);
    }
}


void initM2ParticlePartTracks(M2Data *m2Header) {
    assert(sizeof(M2Particle) == 492);
    int32_t particleEmitterCount = m2Header->particle_emitters.size;
    for (int i = 0; i < particleEmitterCount; i++) {
        M2Particle *particleEmitter = m2Header->particle_emitters.getElement(i);

        particleEmitter->old.geometry_model_filename.initM2Array(m2Header);
        particleEmitter->old.recursion_model_filename.initM2Array(m2Header);

        particleEmitter->old.alphaTrack.initPartTrack(m2Header);
        particleEmitter->old.colorTrack.initPartTrack(m2Header);
        particleEmitter->old.scaleTrack.initPartTrack(m2Header);
        particleEmitter->old.headCellTrack.initPartTrack(m2Header);
        particleEmitter->old.tailCellTrack.initPartTrack(m2Header);

        particleEmitter->old.splinePoints.initM2Array(m2Header);
    }
}

void M2Geom::process(HFileContent m2File, const std::string &fileName) {
    this->m2File = m2File;

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
        M2Data *m2Header = (M2Data *) this->m2File.get();
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

    if (m2Header->global_flags.flag_has_blend_maps) {
        m2Header->blend_map_overrides.initM2Array(m2Header);
    }
    initM2ParticlePartTracks(m2Header);
    initM2Textures(m2Header);

    if (exp2Records != nullptr) {
        initEXP2(exp2Records);
    }

    initTracks(nullptr);

    //Step 2: init tracks
    fsStatus = FileStatus::FSLoaded;
}

void M2Geom::initTracks(CM2SequenceLoad * cm2SequenceLoad) {

    M2Data *m2Header = this->m_m2Data;

    initCompBones(m2Header, cm2SequenceLoad);
    initM2Color(m2Header, cm2SequenceLoad);
    initM2TextureWeight(m2Header, cm2SequenceLoad);
    initM2TextureTransform(m2Header, cm2SequenceLoad);
    initM2Attachment(m2Header, cm2SequenceLoad);
    initM2Event(m2Header, cm2SequenceLoad);
    initM2Light(m2Header, cm2SequenceLoad);
    initM2Particle(m2Header, cm2SequenceLoad);
    initM2Ribbon(m2Header, cm2SequenceLoad);
    initM2Camera(m2Header, cm2SequenceLoad);
}

HGVertexBuffer M2Geom::getVBO(IDevice &device) {
    if (vertexVbo.get() == nullptr) {
        if (m_m2Data->vertices.size == 0) {
            return nullptr;
        }

        vertexVbo = device.createVertexBuffer();
        vertexVbo->uploadData(
            m_m2Data->vertices.getElement(0),
            m_m2Data->vertices.size*sizeof(M2Vertex));
    }

    return vertexVbo;
}

HGVertexBufferBindings M2Geom::getVAO(IDevice &device, SkinGeom *skinGeom) {
    HGVertexBufferBindings bufferBindings = nullptr;
    if (vaoMap.find(skinGeom) != vaoMap.end()) {
        bufferBindings = vaoMap.at(skinGeom);
    } else {
        HGVertexBuffer vboBuffer = this->getVBO(device);
        if (vboBuffer == nullptr) {
            vaoMap[skinGeom] = nullptr;
            return nullptr;
        }

        HGIndexBuffer iboBuffer = skinGeom->getIBO(device);

        //2. Create buffer binding and fill it
        bufferBindings = device.createVertexBufferBindings();
        bufferBindings->setIndexBuffer(iboBuffer);

        GVertexBufferBinding vertexBinding;
        vertexBinding.vertexBuffer = vboBuffer;
        vertexBinding.bindings = std::vector<GBufferBinding>(&staticM2Bindings[0], &staticM2Bindings[6]);

        bufferBindings->addVertexBufferBinding(vertexBinding);
        bufferBindings->save();

        vaoMap[skinGeom] = bufferBindings;
    }

    return bufferBindings;
}

int M2Geom::findAnimationIndex(uint32_t anim_id) {
    if (m_m2Data->sequence_lookups.size == 0)
        return -1;

    size_t i (anim_id % m_m2Data->sequence_lookups.size);

    for (size_t stride (1); true; ++stride)
    {
        if (*m_m2Data->sequence_lookups[i] == -1)
        {
            return -1;
        }
        if (m_m2Data->sequences[*m_m2Data->sequence_lookups[i]]->id == anim_id)
        {
            return *m_m2Data->sequence_lookups[i];
        }

        i = (i + stride * stride) % m_m2Data->sequence_lookups.size;
        // so original_i + 1, original_i + 1 + 4, original_i + 1 + 4 + 9, …
    }
}

void M2Geom::loadLowPriority(IWoWInnerApi *m_api, uint32_t animationId, uint32_t variationId) {
    int animationIndex = findAnimationIndex(animationId);
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
        animFile = m_api->getAnimCache()->getFileId(animationFileDataId);
    } else if (!useFileId) {
        char buffer[1024];
        std::snprintf(buffer, 1024, "%s%04d-%02d.anim", m_nameTemplate.c_str(), animationId, variationId);

        animFile = m_api->getAnimCache()->get(buffer);
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