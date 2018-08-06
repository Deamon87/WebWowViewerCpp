//
// Created by deamon on 19.06.17.
//

#include "m2Geom.h"
#include "skinGeom.h"
#include "../shader/ShaderDefinitions.h"
#include "../opengl/header.h"

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
                {[](M2Geom &file, ChunkData &chunkData) {
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
        {+m2Shader::Attribute::aPosition, 3, GL_FLOAT, false, 48, 0 },
        {+m2Shader::Attribute::boneWeights, 4, GL_UNSIGNED_BYTE, true, 48, 12},  // bonesWeight
        {+m2Shader::Attribute::bones, 4, GL_UNSIGNED_BYTE, false, 48, 16},  // bones
        {+m2Shader::Attribute::aNormal, 3, GL_FLOAT, false, 48, 20}, // normal
        {+m2Shader::Attribute::aTexCoord, 2, GL_FLOAT, false, 48, 32}, // texcoord
        {+m2Shader::Attribute::aTexCoord2, 2, GL_FLOAT, false, 48, 40} // texcoord
};

void initM2Textures(M2Data *m2Header, void *m2File){
    int32_t texturesSize = m2Header->textures.size;
    for (int i = 0; i < texturesSize; i++) {
        M2Texture *texture = m2Header->textures.getElement(i);
        texture->filename.initM2Array(m2Header);
    }
}
void initCompBones(M2Data *m2Header, void *m2File){
    int32_t bonesSize = m2Header->bones.size;
    for (int i = 0; i < bonesSize; i++) {
        M2CompBone *compBone = m2Header->bones.getElement(i);
        compBone->translation.initTrack(m2Header);
        compBone->rotation.initTrack(m2Header);
        compBone->scaling.initTrack(m2Header);
    }
}
void initM2Color(M2Data *m2Header, void *m2File) {
    int32_t m2ColorSize = m2Header->colors.size;
    for (int i = 0; i < m2ColorSize; i++) {
        M2Color * m2Color = m2Header->colors.getElement(i);
        m2Color->alpha.initTrack(m2Header);
        m2Color->color.initTrack(m2Header);
    }
}
void initM2TextureWeight(M2Data *m2Header, void *m2File) {
    int32_t textureWeightSize = m2Header->texture_weights.size;
    for (int i = 0; i < textureWeightSize; i++) {
        M2TextureWeight * textureWeight = m2Header->texture_weights.getElement(i);
        textureWeight->weight.initTrack(m2Header);
    }
}
void initM2TextureTransform(M2Data *m2Header, void *m2File) {
    int32_t textureTransformSize = m2Header->texture_transforms.size;
    for (int i = 0; i < textureTransformSize; i++) {
        M2TextureTransform * textureTransform = m2Header->texture_transforms.getElement(i);
        textureTransform->translation.initTrack(m2Header);
        textureTransform->rotation.initTrack(m2Header);
        textureTransform->scaling.initTrack(m2Header);
    }
}
void initM2Attachment(M2Data *m2Header, void *m2File) {
    int32_t attachmentCount = m2Header->attachments.size;
    for (int i = 0; i < attachmentCount; i++) {
        M2Attachment *attachment = m2Header->attachments.getElement(i);
        attachment->animate_attached.initTrack(m2Header);
    }
}
void initM2Event(M2Data *m2Header, void *m2File) {
    int32_t eventCount = m2Header->events.size;
    for (int i = 0; i < eventCount; i++) {
        M2Event *event = m2Header->events.getElement(i);
        event->enabled.initTrackBase(m2Header);
    }
}
void initM2Light(M2Data *m2Header, void *m2File) {
    int32_t lightCount = m2Header->lights.size;
    for (int i = 0; i < lightCount; i++) {
        M2Light *light = m2Header->lights.getElement(i);
        light->ambient_color.initTrack(m2Header);
        light->ambient_intensity.initTrack(m2Header);
        light->diffuse_color.initTrack(m2Header);
        light->diffuse_intensity.initTrack(m2Header);
        light->attenuation_start.initTrack(m2Header);
        light->attenuation_end.initTrack(m2Header);
        light->visibility.initTrack(m2Header);
    }
}

void initM2Particle(M2Data *m2Header, void *m2File) {
    assert(sizeof(M2Particle) == 492);
    int32_t particleEmitterCount = m2Header->particle_emitters.size;
    for (int i = 0; i < particleEmitterCount; i++) {
        M2Particle *particleEmitter = m2Header->particle_emitters.getElement(i);

        particleEmitter->old.geometry_model_filename.initM2Array(m2Header);
        particleEmitter->old.recursion_model_filename.initM2Array(m2Header);

        particleEmitter->old.emissionSpeed.initTrack(m2Header);
        particleEmitter->old.speedVariation.initTrack(m2Header);
        particleEmitter->old.verticalRange.initTrack(m2Header);
        particleEmitter->old.horizontalRange.initTrack(m2Header);
        particleEmitter->old.gravity.initTrack(m2Header);
        particleEmitter->old.lifespan.initTrack(m2Header);
        particleEmitter->old.emissionRate.initTrack(m2Header);
        particleEmitter->old.emissionAreaLength.initTrack(m2Header);
        particleEmitter->old.emissionAreaWidth.initTrack(m2Header);
        particleEmitter->old.zSource.initTrack(m2Header);
        particleEmitter->old.alphaTrack.initPartTrack(m2Header);
        particleEmitter->old.colorTrack.initPartTrack(m2Header);
        particleEmitter->old.scaleTrack.initPartTrack(m2Header);
        particleEmitter->old.headCellTrack.initPartTrack(m2Header);
        particleEmitter->old.tailCellTrack.initPartTrack(m2Header);

        particleEmitter->old.splinePoints.initM2Array(m2Header);

        particleEmitter->old.enabledIn.initTrack(m2Header);
    }
}

void initM2Camera(M2Data *m2Header, void *m2File) {
    int32_t cameraCount = m2Header->cameras.size;
    for (int i = 0; i < cameraCount; i++) {
        M2Camera *camera = m2Header->cameras.getElement(i);
        camera->positions.initTrack(m2Header);
        camera->target_position.initTrack(m2Header);
        camera->roll.initTrack(m2Header);
        camera->FoV.initTrack(m2Header);
    }
}

void M2Geom::process(std::vector<unsigned char> &m2File, std::string &fileName) {
    this->m2File = m2File;

    if (
        m2File[0] == 'M' &&
        m2File[1] == 'D' &&
        m2File[2] == '2' &&
        m2File[3] == '1'

            ) {
        CChunkFileReader reader(this->m2File);
        reader.processFile(*this, &M2Geom::m2FileTable);
    } else {
        M2Data *m2Header = (M2Data *) &this->m2File[0];
        this->m_m2Data = m2Header;
    }
    M2Data *m2Header = this->m_m2Data;
    void *m2FileP = &this->m2File[0];

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
        m2Header->blend_map_overrides.initM2Array(m2FileP);
    }

    //Step 2: init tracks
    initCompBones(m2Header, m2FileP);
    initM2Color(m2Header, m2FileP);
    initM2TextureWeight(m2Header, m2FileP);
    initM2TextureTransform(m2Header, m2FileP);
    initM2Attachment(m2Header, m2FileP);
    initM2Event(m2Header, m2FileP);
    initM2Light(m2Header, m2FileP);
    initM2Particle(m2Header, m2FileP);
    initM2Camera(m2Header, m2FileP);

    initM2Textures(m2Header, m2FileP);

    m_loaded = true;
}

HGVertexBuffer M2Geom::getVBO(GDevice &device) {
    if (vertexVbo.get() == nullptr) {
        vertexVbo = device.createVertexBuffer();
        vertexVbo->uploadData(
            m_m2Data->vertices.getElement(0),
            m_m2Data->vertices.size*sizeof(M2Vertex));
    }

    return vertexVbo;
}

HGVertexBufferBindings M2Geom::getVAO(GDevice &device, SkinGeom *skinGeom) {
    HGVertexBufferBindings bufferBindings = nullptr;
    if (vaoMap.find(skinGeom) != vaoMap.end()) {
        bufferBindings = vaoMap.at(skinGeom);
    } else {
        HGVertexBuffer vboBuffer = this->getVBO(device);
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

