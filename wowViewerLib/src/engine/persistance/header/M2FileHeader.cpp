//
// Created by Deamon on 9/15/2020.
//

#include "M2FileHeader.h"

void initEXP2(EXP2 *exp2) {
    exp2->content.initM2Array(exp2);
    for (int i = 0; i < exp2->content.size; i++) {
        Exp2Record *exp2Record = exp2->content.getElement(i);
        exp2Record->unk3.timestamps.initM2Array(exp2);
        exp2Record->unk3.values.initM2Array(exp2);
    }
}

void initM2Textures(void *sectionStart, M2Array<M2Texture> &textures) {
    int32_t texturesSize = textures.size;
    for (int i = 0; i < texturesSize; i++) {
        M2Texture *texture = textures.getElement(i);
        texture->filename.initM2Array(sectionStart);
    }
}

void initCompBones(void *sectionStart, M2Array<M2CompBone> *bones, M2Array<M2Sequence> *sequences,
                   CM2SequenceLoad *cm2SequenceLoad) {
    int32_t bonesSize = bones->size;
    for (int i = 0; i < bonesSize; i++) {
        M2CompBone *compBone = bones->getElement(i);
        compBone->translation.initTrack(sectionStart, sequences, cm2SequenceLoad);
        compBone->rotation.initTrack(sectionStart, sequences, cm2SequenceLoad);
        compBone->scaling.initTrack(sectionStart, sequences, cm2SequenceLoad);
    }
}

void initM2Color(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t m2ColorSize = m2Header->colors.size;
    for (int i = 0; i < m2ColorSize; i++) {
        M2Color * m2Color = m2Header->colors.getElement(i);
        m2Color->alpha.initTrack(m2Header, sequences, cm2SequenceLoad);
        m2Color->color.initTrack(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2TextureWeight(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t textureWeightSize = m2Header->texture_weights.size;
    for (int i = 0; i < textureWeightSize; i++) {
        M2TextureWeight * textureWeight = m2Header->texture_weights.getElement(i);
        textureWeight->weight.initTrack(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2TextureTransform(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t textureTransformSize = m2Header->texture_transforms.size;
    for (int i = 0; i < textureTransformSize; i++) {
        M2TextureTransform * textureTransform = m2Header->texture_transforms.getElement(i);
        textureTransform->translation.initTrack(m2Header, sequences, cm2SequenceLoad);
        textureTransform->rotation.initTrack(m2Header, sequences, cm2SequenceLoad);
        textureTransform->scaling.initTrack(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2Attachment(void *sectionStart, M2Array<M2Attachment> *attachments, M2Array<M2Sequence> *sequences,
                      CM2SequenceLoad *cm2SequenceLoad) {
    int32_t attachmentCount = attachments->size;
    for (int i = 0; i < attachmentCount; i++) {
        M2Attachment *attachment = attachments->getElement(i);
        attachment->animate_attached.initTrack(sectionStart, sequences, cm2SequenceLoad);
    }
}

void initM2Event(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t eventCount = m2Header->events.size;
    for (int i = 0; i < eventCount; i++) {
        M2Event *event = m2Header->events.getElement(i);
        event->enabled.initTrackBase(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2Light(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t lightCount = m2Header->lights.size;
    for (int i = 0; i < lightCount; i++) {
        M2Light *light = m2Header->lights.getElement(i);
        light->ambient_color.initTrack(m2Header, sequences, cm2SequenceLoad);
        light->ambient_intensity.initTrack(m2Header, sequences, cm2SequenceLoad);
        light->diffuse_color.initTrack(m2Header, sequences, cm2SequenceLoad);
        light->diffuse_intensity.initTrack(m2Header, sequences, cm2SequenceLoad);
        light->attenuation_start.initTrack(m2Header, sequences, cm2SequenceLoad);
        light->attenuation_end.initTrack(m2Header, sequences, cm2SequenceLoad);
        light->visibility.initTrack(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2Particle(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    assert(sizeof(M2Particle) == 492);
    int32_t particleEmitterCount = m2Header->particle_emitters.size;
    for (int i = 0; i < particleEmitterCount; i++) {
        M2Particle *particleEmitter = m2Header->particle_emitters.getElement(i);

        particleEmitter->old.emissionSpeed.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.speedVariation.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.verticalRange.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.horizontalRange.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.gravity.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.lifespan.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.emissionRate.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.emissionAreaLength.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.emissionAreaWidth.initTrack(m2Header, sequences, cm2SequenceLoad);
        particleEmitter->old.zSource.initTrack(m2Header, sequences, cm2SequenceLoad);

        particleEmitter->old.enabledIn.initTrack(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2Ribbon(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t ribbonEmitterCount = m2Header->ribbon_emitters.size;
    for (int i = 0; i < ribbonEmitterCount; i++) {
        M2Ribbon *ribbonEmitter = m2Header->ribbon_emitters.getElement(i);

        ribbonEmitter->textureIndices.initM2Array(m2Header);
        ribbonEmitter->materialIndices.initM2Array(m2Header);

        ribbonEmitter->colorTrack.initTrack(m2Header, sequences, cm2SequenceLoad);
        ribbonEmitter->alphaTrack.initTrack(m2Header, sequences, cm2SequenceLoad);
        ribbonEmitter->heightAboveTrack.initTrack(m2Header, sequences, cm2SequenceLoad);
        ribbonEmitter->heightBelowTrack.initTrack(m2Header, sequences, cm2SequenceLoad);
        ribbonEmitter->texSlotTrack.initTrack(m2Header, sequences, cm2SequenceLoad);
        ribbonEmitter->visibilityTrack.initTrack(m2Header, sequences, cm2SequenceLoad);
    }
}

void initM2Camera(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
    int32_t cameraCount = m2Header->cameras.size;
    for (int i = 0; i < cameraCount; i++) {
        M2Camera *camera = m2Header->cameras.getElement(i);
        camera->positions.initTrack(m2Header, sequences, cm2SequenceLoad);
        camera->target_position.initTrack(m2Header, sequences, cm2SequenceLoad);
        camera->roll.initTrack(m2Header, sequences, cm2SequenceLoad);
        camera->FoV.initTrack(m2Header, sequences, cm2SequenceLoad);
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

int findAnimationIndex(uint32_t anim_id, M2Array<int16_t> *sequence_lookupsPtr, M2Array<M2Sequence> *sequencesPtr) {
    auto &sequences = *sequencesPtr;
    auto &sequence_lookups = *sequence_lookupsPtr;

    if (sequence_lookups.size == 0) {
        int animationIndex = -1;
        if ((sequences.size > 0)) {
            for (int i = 0; i < sequences.size; i++) {
                const M2Sequence* animationRecord = sequences[i];
                if (animationRecord->id == anim_id) {
                    animationIndex = i;
                    break;
                }
            }
        }
        return animationIndex;
    }

    size_t i (anim_id % sequence_lookups.size);

    for (size_t stride (1); true; ++stride)
    {
        if (*sequence_lookups[i] == -1)
        {
            return -1;
        }
        if (sequences[*sequence_lookups[i]]->id == anim_id)
        {
            return *sequence_lookups[i];
        }

        i = (i + stride * stride) % sequence_lookups.size;
        // so original_i + 1, original_i + 1 + 4, original_i + 1 + 4 + 9, …
    }
}