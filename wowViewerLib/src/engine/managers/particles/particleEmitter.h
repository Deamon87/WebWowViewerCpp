//
// Created by deamon on 14.12.17.
//

#ifndef WEBWOWVIEWERCPP_PARTICLEEMITTER_H
#define WEBWOWVIEWERCPP_PARTICLEEMITTER_H

#include <cstdlib>
#include "../../persistance/header/M2FileHeader.h"
#include "generators/CParticleGenerator.h"
#include "generators/CSphereGenerator.h"
#include "generators/CPlaneGenerator.h"

class ParticleEmitter {
public:
    ParticleEmitter(M2Particle *particle) : m_seed(0) {

        m_data = particle;

        switch (m_data->old.emitterType) {
            case 1:
                this->generator = new CPlaneGenerator(this->m_seed);
                break;
            case 2:
                this->generator = new CSphereGenerator(this->m_seed, 0 != (m_data->old.flags & 0x100));
                break;
            default:
                this->generator = nullptr;
                break;
        }

        const float followDen = m_data->old.followSpeed2 - m_data->old.followSpeed1;
        if (followDen != 0) {
            this->followMult = (m_data->old.followScale2 - m_data->old.followScale1) / followDen;
            this->followBase = m_data->old.followScale1 - m_data->old.followSpeed1 * this->followMult;
        }
        else {
            this->followMult = 0;
            this->followBase = 0;
        }

        uint16_t cols = m_data->old.textureDimensions_columns;
        if (cols <= 0) {
            cols = 1;
        }
        uint16_t rows = m_data->old.textureDimensions_rows;
        if (rows <= 0) {
            rows = 1;
        }
        this->textureIndexMask = cols * rows - 1;
        this->textureStartIndex = 0;
        this->textureColBits = _bit_scan_reverse(cols);
        this->textureColMask = cols - 1;
        if (m_data->old.flags & 0x200000) {
            this->textureStartIndex = this->m_seed.uint32t() & this->textureIndexMask;
        }
        this->texScaleX = 1 / cols;
        this->texScaleY = 1 / rows;
        if (m_data->old.flags & 0x10100000) {
            const isMultitex = 0 != (1 & (m_data->old.flags >> 0x1c));
            if (isMultitex) {
                this->particleType = 2;
            }
            else {
                this->particleType = 3;
            }
            this->particleSize = 17;
        }
    }

public:
    void resizeParticleBuffer() {
        int newCount = this->generator->GetMaxEmissionRate() * this->generator->GetMaxLifeSpan() * 1.15;
        if (newCount > this->particles.size()) {
            const newBuffer = mem.Malloc(this.particleSize * newCount);
            const oldCount = this.numLiveParticles;
            if (oldCount > 0) {
                mem.Copy(newBuffer, oldBuffer, this.particleSize * oldCount);
            }
            mem.Free(oldBuffer);
            this->particleBufferCount = newCount;
            this->particleBuffer = newBuffer;
        }
    }

private:
    M2Particle *m_data;
    CRndSeed m_seed;

    CParticleGenerator *generator;

    std::vector<CParticle2> particles;


    int particleType;

    float followMult;
    float followBase;

    int burstTime = 0;
    float inheritedScale = 1;
    float emission = 0;
    bool emittingLastFrame = false;

    int32_t textureIndexMask;
    int32_t textureStartIndex;
    int32_t textureColBits;
    int32_t textureColMask;

    float texScaleX;
    float texScaleY;

    int particleSize;
};


#endif //WEBWOWVIEWERCPP_PARTICLEEMITTER_H
