//
// Created by deamon on 19.06.17.
//

#include "m2Geom.h"
#include "../persistance/M2File.h"

void M2Geom::loadTextures() {

}

void M2Geom::createVBO() {

}

void M2Geom::setupAttributes(void *skinObject) {

}


void initCompBones(M2Data *m2Header){
    int32_t bonesSize = m2Header->bones.size;
    for (int i = 0; i < bonesSize; i++) {
        M2CompBone *compBone = m2Header->bones.getElement(i);
        compBone->translation.initTrack(m2Header);
        compBone->rotation.initTrack(m2Header);
        compBone->scale.initTrack(m2Header);
    }
}
void initM2Color(M2Data *m2Header) {
    int32_t m2ColorSize = m2Header->colors.size;
    for (int i = 0; i < m2ColorSize; i++) {
        M2Color * m2Color = m2Header->colors.getElement(i);
        m2Color->alpha.initTrack(m2Header);
        m2Color->color.initTrack(m2Header);
    }
}
void initM2TextureWeight(M2Data *m2Header) {
    int32_t textureWeightSize = m2Header->texture_weights.size;
    for (int i = 0; i < textureWeightSize; i++) {
        M2TextureWeight * textureWeight = m2Header->texture_weights.getElement(i);
        textureWeight->weight.initTrack(m2Header);
    }
}
void initM2TextureTransform(M2Data *m2Header) {
    int32_t textureTransformSize = m2Header->texture_transforms.size;
    for (int i = 0; i < textureTransformSize; i++) {
        M2TextureTransform * textureTransform = m2Header->texture_transforms.getElement(i);
        textureTransform->translation.initTrack(m2Header);
        textureTransform->rotation.initTrack(m2Header);
        textureTransform->scaling.initTrack(m2Header);
    }
}
void initM2Attachment(M2Data *m2Header) {
    int32_t attachmentCount = m2Header->attachments.size;
    for (int i = 0; i < attachmentCount; i++) {
        M2Attachment *attachment = m2Header->attachments.getElement(i);
        attachment->animate_attached.initTrack(m2Header);
    }
}
void initM2Event(M2Data *m2Header) {
    int32_t eventCount = m2Header->events.size;
    for (int i = 0; i < eventCount; i++) {
        M2Event *event = m2Header->events.getElement(i);
        event->enabled.initTrack(m2Header);
    }
}
void initM2Light(M2Data *m2Header) {
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
}void initM2Camera(M2Data *m2Header) {
    int32_t cameraCount = m2Header->cameras.size;
    for (int i = 0; i < cameraCount; i++) {
        M2Camera *light = m2Header->cameras.getElement(i);
        light->positions.initTrack(m2Header);
        light->target_position.initTrack(m2Header);
        light->roll.initTrack(m2Header);
    }
}

void M2Geom::initM2File(void *m2File, int fileLength) {
    M2Data *m2Header = (M2Data *) m2File;

    static_assert(std::is_pod<M2Array<M2Loop>>::value, "M2Array<M2Loop> is not POD");

    //Step 1: Init all m2Arrays
    m2Header->global_loops.initM2Array(m2File);
    m2Header->sequences.initM2Array(m2File);
    m2Header->sequence_lookups.initM2Array(m2File);
    m2Header->bones.initM2Array(m2File);
    m2Header->key_bone_lookup.initM2Array(m2File);
    m2Header->vertices.initM2Array(m2File);
    m2Header->colors.initM2Array(m2File);
    m2Header->textures.initM2Array(m2File);
    m2Header->texture_weights.initM2Array(m2File);
    m2Header->texture_transforms.initM2Array(m2File);
    m2Header->replacable_texture_lookup.initM2Array(m2File);
    m2Header->materials.initM2Array(m2File);
    m2Header->bone_lookup_table.initM2Array(m2File);
    m2Header->texture_lookup_table.initM2Array(m2File);
    m2Header->tex_unit_lookup_table.initM2Array(m2File);
    m2Header->transparency_lookup_table.initM2Array(m2File);
    m2Header->texture_transforms_lookup_table.initM2Array(m2File);
    m2Header->collision_triangles.initM2Array(m2File);
    m2Header->collision_vertices.initM2Array(m2File);
    m2Header->collision_normals.initM2Array(m2File);
    m2Header->attachments.initM2Array(m2File);
    m2Header->attachment_lookup_table.initM2Array(m2File);
    m2Header->events.initM2Array(m2File);
    m2Header->lights.initM2Array(m2File);
    m2Header->cameras.initM2Array(m2File);
    m2Header->camera_lookup_table.initM2Array(m2File);
    m2Header->ribbon_emitters.initM2Array(m2File);
    m2Header->particle_emitters.initM2Array(m2File);

    if (m2Header->global_flags.flag_has_blend_maps) {
        m2Header->blend_map_overrides.initM2Array(m2File);
    }

    //Step 2: init tracks
    initCompBones(m2Header);
    initM2Color(m2Header);
    initM2TextureWeight(m2Header);
    initM2TextureTransform(m2Header);
    initM2Attachment(m2Header);
    initM2Event(m2Header);
    initM2Light(m2Header);
    initM2Camera(m2Header);
}

