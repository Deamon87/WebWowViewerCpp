//
// Created by deamon on 14.12.17.
//

#include "particleEmitter.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/animate.h"
#include "../../shaderDefinitions.h"

void ParticleEmitter::resizeParticleBuffer() {
    int newCount = this->generator->GetMaxEmissionRate() * this->generator->GetMaxLifeSpan() * 1.15;
//    if (newCount > (int)this->particles.size()) {
//        this->particles.reserve(newCount);
//    }
}

void ParticleEmitter::Update(animTime_t delta, mathfu::mat4 boneModelMat) {
    if (getGenerator() == nullptr) return;

    this->resizeParticleBuffer();


    mathfu::vec3 lastPos = -transform.TranslationVector3D();
    mathfu::vec3 currPos = -boneModelMat.TranslationVector3D();
    this->transform = boneModelMat;

    this->inheritedScale = this->transform.GetColumn(0).Length();
    mathfu::vec3 dPos = lastPos - currPos;
    if ((this->m_data->old.flags & 0x4000) > 0) {
        float x = this->followMult * (dPos.Length() / delta) + this->followBase;
        if (x < 0)
            x = 0;
        if (x > 1)
            x = 1;
        this->deltaPosition =  dPos * x;
    }
    if (this->m_data->old.flags & 0x40) {
        this->burstTime += delta;
        animTime_t frameTime = 30.0 / 1000.0;
        if (this->burstTime > frameTime) {
            if (this->particles.size() == 0) {
                animTime_t frameAmount = frameTime / this->burstTime;
                this->burstTime = 0;
                this->burstVec = dPos * frameAmount * this->m_data->old.BurstMultiplier;
            }
            else {
                this->burstVec = mathfu::vec3(0, 0, 0);
            }
        }
    }
    if (this->particles.size() > 0 && 0 == (this->flags & 16)) {
        delta += 5.0;
        this->flags |= 16;
    }
    if (delta > 0.1) {
        animTime_t clamped = delta;
        if (delta > 5.0f) {
            clamped = 5.0f;
        }
        for (float i = 0; i < clamped; i += 0.05) {
            animTime_t d = 0.05;
            if (clamped - i < 0.05) {
                d = clamped - i;
            }
            this->Simulate(d);
        }
    }
    else {
        this->Simulate(delta);
    }
}

void ParticleEmitter::Simulate(animTime_t delta) {
    ParticleForces forces;

    this->CalculateForces(forces, delta);
    this->EmitNewParticles(delta);
    int i = 0;
    while (i < this->particles.size()) {
        auto &p = this->particles[i];
        p.age = p.age + delta;
        auto life = p.lifespan;
        if (p.age > (this->generator->GetLifeSpan(life))) {
            this->KillParticle(i);
            i--;
        } else {
            if (!this->UpdateParticle(p, delta, forces)) {
                this->KillParticle(i);
                i--;
            }
        }
        i++;
    }
}
void ParticleEmitter::EmitNewParticles(animTime_t delta) {
    float rate = this->generator->GetEmissionRate();
    if (6 == (this->flags & 6)) {
        int count = rate;
        for (int i = 0; i < count; i++) {
            this->CreateParticle(0);
        }
    }
    if (3 == (this->flags & 3)) {
        this->emission += delta * rate;
        while (this->emission > 1) {
            this->CreateParticle(delta);
            this->emission -= 1;
        }
    }
}
void ParticleEmitter::CreateParticle(animTime_t delta) {
    CParticle2 &p = this->BirthParticle();

    this->generator->CreateParticle(p, delta);
    if (!(this->m_data->old.flags & 0x10)) {
        p.position = (this->transform * mathfu::vec4(p.position, 1.0f)).xyz();
        p.velocity = (this->transform * mathfu::vec4(p.velocity, 0.0f)).xyz();
        if (this->m_data->old.flags & 0x2000) {
            // Snap to ground; set z to 0:
            p.position.z = 0.0;
        }
    }
    if (this->m_data->old.flags & 0x40) {
        float speedMul = 1.0f + this->generator->getAniProp()->speedVariation * this->m_seed.Uniform();
        mathfu::vec3 r0 = this->burstVec * speedMul;
        p.velocity += r0;
    }
    if (this->particleType >= 2) {
        for (int i = 0; i < 2; i++) {
            p.texPos[i].x = this->m_seed.UniformPos();
            p.texPos[i].y = this->m_seed.UniformPos();

            mathfu::vec2 v2 = MathHelper::convertV69ToV2(this->m_data->multiTextureParam1[i]) * this->m_seed.Uniform();
            p.texVel[i] = v2  + MathHelper::convertV69ToV2(this->m_data->multiTextureParam0[i]);
        }
    }
}


void ParticleEmitter:: CalculateForces(ParticleForces &forces, animTime_t delta) {
    if (false && (this->m_data->old.flags & 0x80000000)) {
        forces.drift = mathfu::vec3(0.707, 0.707, 0);
        forces.drift = forces.drift * delta;
    }
    else {
        forces.drift = mathfu::vec3(this->m_data->old.WindVector) * delta;
    }

    auto g = this->generator->GetGravity();
    forces.velocity = g * delta;
    forces.position = g * delta * delta * 0.5f;
    forces.drag = this->m_data->old.drag * delta;
}

bool ParticleEmitter::UpdateParticle(CParticle2 &p, animTime_t delta, ParticleForces &forces) {

    if (this->particleType >= 2) {
        for (int i = 0; i < 2; i++) {
            // s = frac(s + v * dt)
            float val = (float) (p.texPos[i].x + delta * p.texVel[i].x);
            p.texPos[i].x = (float) (val - floor(val));

            val = (float) (p.texPos[i].y + delta * p.texVel[i].y);
            p.texPos[i].y = (float) (val - floor(val));
        }
    }

    p.velocity = p.velocity + forces.drift;
    if ((this->m_data->old.flags & 0x4000) && 2 * delta < p.age) {
        p.position = p.position + this->deltaPosition;
    }

    mathfu::vec3 r0 = p.velocity * delta; // v*dt

    p.velocity = p.velocity + forces.velocity;
    p.velocity = p.velocity *  (1.0f - forces.drag);

    p.position = p.position + r0;
    p.position = p.position + forces.position;

    if (this->m_data->old.emitterType == 2 && (this->m_data->old.flags & 0x80)) {
        mathfu::vec3 r1 = p.position;
        if (!(this->m_data->old.flags & 0x10)) {

            this->transform.GetColumn(3) = mathfu::vec4(r1, 1.0);
            r1 = p.position - r1;
        }
        if (mathfu::vec3::DotProduct(r1, r0) > 0) {
            return false;
        }
    }

    return true;
}

void ParticleEmitter::KillParticle(int index) {
    particles.erase(particles.begin()+index);
}
CParticle2& ParticleEmitter::BirthParticle() {
    CParticle2 p1;
    particles.push_back(p1);

    CParticle2 &p = *(particles.end()-1);
    return p;
}

void ParticleEmitter::prepearBuffers(mathfu::mat4 &viewMatrix) {
    if (particles.size() == 0 && this->generator != nullptr) {
        return;
    }

    // Load textures at top so we can bail out early
     if (this->m_data->old.flags & 0x10) {
        // apply the model transform
        this->particleToView = viewMatrix * this->transform;
    }
    else {
        this->particleToView = viewMatrix;
    }

    this->viewMatrix = viewMatrix;
    // Mat34.Col(this.particleNormal, viewMatrix, 2);
    // Build vertices for each particle

    // Vertex format: {float3 pos; float3 norm; float4 col; float2 texcoord} = 12
    // Multitex format: {float3 pos; float4 col; float2 texcoord[3]} = 13

    szVertexBuf = std::vector<uint8_t>(0);
    szIndexBuff = std::vector<uint16_t>(0);
    // TODO: z-sort
    int vo = 0;
    for (int i = 0; i < particles.size(); i++) {
        CParticle2 &p = this->particles[i];
        int rendered = this->RenderParticle(p, szVertexBuf);
        if ((rendered) > 0) {
            for (int j = 0; j < rendered; j++) {
                // 0 2
                // 1 3
                // 0, 1, 2; 3, 2, 1
                szIndexBuff.push_back(vo + 0);
                szIndexBuff.push_back(vo + 1);
                szIndexBuff.push_back(vo + 2);
                szIndexBuff.push_back(vo + 3);
                szIndexBuff.push_back(vo + 2);
                szIndexBuff.push_back(vo + 1);
                vo += 4;
            }
        }
    }
}

int ParticleEmitter::RenderParticle(CParticle2 &p, std::vector<uint8_t> &szVertexBuf) {
    float twinkle = this->m_data->old.TwinklePercent;
    auto twinkleRange = this->m_data->old.twinkleScale;

    float twinkleMin = twinkleRange.min;
    float twinkleVary = twinkleRange.max - twinkleMin;
    uint16_t seed = p.seed;
    animTime_t age = p.age;
    mathfu::vec3 pos = p.position;
    mathfu::vec3 vel = p.velocity;
    int rndIdx = 0;
    if (twinkle < 1 || !feq(twinkleVary,0)) {
        rndIdx = 0x7f & ((int)(age * this->m_data->old.TwinkleSpeed) + seed);
    }
//    if (twinkle < ParticleEmitter.RandTable[rndIdx]) {
//        return false;
//    }
    int amountrendered = 0;
    CRndSeed rand(seed);

    mathfu::vec3 defaultColor(255.0f, 255.0f, 255.0f);
    mathfu::vec2 defaultScale(1.0f, 1.0f);
    float defaultAlpha = 1.0f;
    uint16_t defaultCell = 1;

    float percentTime = p.age /  this->getGenerator()->GetMaxLifeSpan();
    mathfu::vec3 color = animatePartTrack<C3Vector, mathfu::vec3>(percentTime, &m_data->old.colorTrack, defaultColor) / 255.0;
    mathfu::vec2 scale = animatePartTrack<C2Vector, mathfu::vec2>(percentTime, &m_data->old.scaleTrack, defaultScale);
    float alpha = animatePartTrack<fixed16, float>(percentTime, &m_data->old.alphaTrack, defaultAlpha);
//    if (alpha < 0.9) alpha = 1.0;
//    if (color.x < 0.1 && color.y < 0.1 && color.z < 0.1) {
//        color = mathfu::vec3(1.0,1.0,1.0);
//    }

    uint16_t headCell = animatePartTrack<uint16_t, uint16_t>(percentTime, &m_data->old.headCellTrack, defaultCell);
    uint16_t tailCell = animatePartTrack<uint16_t, uint16_t>(percentTime, &m_data->old.tailCellTrack, defaultCell);

    if ((this->m_data->old.flags & 0x10000 && (m_data->old.headCellTrack.values.size == 0))) {
        headCell = rand.uint32t() & this->textureIndexMask;
    } else {
        headCell = (headCell + this->textureStartIndex) & this->textureIndexMask;
    }


    float u0 = rand.Uniform();
    float vx = 1.0f + u0 * this->m_data->old.scaleVary.x;
    if (vx < 0.0001) {
        vx = 0.0001;
    }
    if (this->m_data->old.flags & 0x80000) {
        float u1 = rand.Uniform();
        float vy = 1.0f + u1 * this->m_data->old.scaleVary.y;
        if (vy < 0.0001) {
            vy = 0.0001;
        }
        scale = mathfu::vec2::HadamardProduct(scale, mathfu::vec2(vx, vy));
    }
    else {
        scale *= vx;
    }

    float baseSpin = this->m_data->old.baseSpin + rand.Uniform() * this->m_data->old.baseSpinVary;
    float deltaSpin = this->m_data->old.Spin + rand.Uniform() * this->m_data->old.spinVary;
//    float weight = twinkleVary * ParticleEmitter.RandTable[rndIdx] + twinkleMin;
    float weight = twinkleVary * rand.UniformPos() + twinkleMin;
    scale = scale * weight;
    if (this->m_data->old.flags & 0x20) {
        scale = scale * this->inheritedScale;
    }
    mathfu::vec3 viewPos = this->particleToView * pos;

    if (this->m_data->old.flags & 0x20000) {
        // head cell
        float texStartX = headCell & this->textureColMask;
        float texStartY = headCell >> this->textureColBits;
        texStartX *= this->texScaleX;
        texStartY *= this->texScaleY;
        // 4x4 affine matrix, viewPos is w column, and the z column is 0
        // this multiplies our quad coords
        // these are the column 0 and 1 vectors:
        mathfu::vec3 m0 = mathfu::vec3(0, 0, 0);
        mathfu::vec3 m1 = mathfu::vec3(0, 0, 0);
        float theta = 0;
        if (!feq(this->m_data->old.Spin, 0) || !feq(this->m_data->old.spinVary, 0)) {
            theta = baseSpin + deltaSpin * age;
            if ((this->m_data->old.flags & 0x200) && (seed & 1)) {
                theta = -theta;
            }
        }
        if ((this->m_data->old.flags & 4) && mathfu::vec3::DotProduct(vel,vel) > 0.0001) {
            // scale and rotation align to particle velocity
            mathfu::vec3 viewVel = (this->particleToView * mathfu::vec4(vel, 0)).xyz();
            mathfu::vec3 screenVel = viewVel;
            float screenVelMag = screenVel.Length();
            mathfu::vec2 screenVelNorm = screenVel.xy() * (1.0f / screenVelMag);
            float scaleMod = screenVelMag / viewVel.Length();
            float scaleModX = scale.x * scaleMod;
            float scaleModY = scale.y * scaleMod;
            m0 = mathfu::vec3(screenVelNorm.x * scaleModX, screenVelNorm.y * scaleModX, 0);
            m1 = mathfu::vec3(-screenVelNorm.y * scaleModY, screenVelNorm.x * scaleModY, 0);
        }
        else if (this->m_data->old.flags & 0x1000) {
            // quad aligns to world-space ground plane instead of view plane
            // the viewPos is still correct, but the (x, y) offsets need to
            // be transformed as world-space offsets (and scaled/rotated)
            // 1. transform the scale vector (which is in XY) to view space
            float transformInvScale = 1.0f;
            if (this->m_data->old.flags & 0x10) {
                // particleToView has the model transform in it, we need to
                // scale that away
                transformInvScale = 1.0f / this->inheritedScale;
            }
            mathfu::vec3 scaleViewX = this->particleToView.GetColumn(0).xyz();
            mathfu::vec3 scaleViewY = this->particleToView.GetColumn(1).xyz();
            scaleViewX = scaleViewX * transformInvScale * scale.x;
            scaleViewY = scaleViewY * transformInvScale * scale.y;
            // 2. rotate the transformed scale vectors around the up vector
            if (!feq(theta, 0)) {
                mathfu::vec3 viewUp = this->particleToView.GetColumn(2).xyz().Normalized();
                mathfu::quat quadRot = mathfu::quat::FromAngleAxis(theta,viewUp);

                m0 = (quadRot.ToMatrix4() * mathfu::vec4(scaleViewX, 0)).xyz();
                m1 = (quadRot.ToMatrix4() * mathfu::vec4(scaleViewY, 0)).xyz();
            }
            else {
                m0 = scaleViewX;
                m1 = scaleViewY;
            }
        }
        else {
            if (!feq(theta, 0)) {
                float cosTheta = cos(theta);
                float sinTheta = sin(theta);
                m0 = mathfu::vec3(cosTheta * scale.x, sinTheta * scale.x, 0);
                m1 = mathfu::vec3(-sinTheta * scale.y, cosTheta * scale.y, 0);
            }
            else {
                m0 = mathfu::vec3(scale.x, 0, 0);
                m1 = mathfu::vec3(0, scale.y, 0);
            }
        }


//        m0 += 0.5;
//        m1 += 0.5;

        // build vertices from coords:
        if (this->particleType >= 2) {
            this->BuildQuadT3(szVertexBuf, m0, m1, viewPos, color, alpha, texStartX, texStartY, p.texPos);
            amountrendered++;
//            return true;
        } else {
            this->BuildQuad(szVertexBuf, m0, m1, viewPos, color, alpha, texStartX, texStartY);
            amountrendered++;
//            return true;
        }
    }
    if (this->m_data->old.flags & 0x40000) {
        // tail cell
        float texStartX = tailCell & this->textureColMask;
        float texStartY = tailCell >> this->textureColBits;
        texStartX *= this->texScaleX;
        texStartY *= this->texScaleY;
        mathfu::vec3 m0 = mathfu::vec3(0, 0, 0);
        mathfu::vec3 m1 = mathfu::vec3(0, 0, 0);
        // as above, scale and rotation align to particle velocity
        float trailTime = this->m_data->old.tailLength;
        if ((this->m_data->old.flags & 0x400) && trailTime > age) {
            trailTime = age;
        }
        mathfu::vec3 viewVel = vel * -1;
        viewVel = (this->particleToView *mathfu::vec4(viewVel, 0)).xyz() * trailTime;
        mathfu::vec3 screenVel = mathfu::vec3(viewVel.xy(), 0);
        if (mathfu::vec3::DotProduct(screenVel, screenVel) > 0.0001) {
            float invScreenVelMag = 1.0f / screenVel.Length();
            scale = scale * invScreenVelMag;
            screenVel = mathfu::vec3(mathfu::vec2::HadamardProduct(screenVel.xy(), scale), 0);
            m1 = mathfu::vec3(-screenVel.y, screenVel.x, 0);
            m0 = viewVel * 0.5f;
            viewPos = viewPos + m0;
        }
        else {
            m0 = mathfu::vec3(scale, 0);
        }
        if (this->particleType >= 2) {
            this->BuildQuadT3(szVertexBuf, m0, m1, viewPos, color, alpha, texStartX, texStartY,  p.texPos);
            amountrendered++;
//            return true;
        }
        else {
            this->BuildQuad(szVertexBuf, m0, m1, viewPos, color, alpha, texStartX, texStartY);
            amountrendered++;
//            return true;
        }
    }

    return amountrendered;
}

void
ParticleEmitter::BuildQuadT3(
    std::vector<uint8_t> &szVertexBuf,
    mathfu::vec3 &m0, mathfu::vec3 &m1,
    mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
    float texStartX, float texStartY, mathfu::vec2 *texPos) {

    static const float vxs[4] = {-1, -1, 1, 1};
    static const float vys[4] = {1, -1, 1, -1};
    static const float txs[4] = {0, 0, 1, 1};
    static const float tys[4] = {0, 1, 0, 1};

    struct ParticleBuffStruct {
        C3Vector position; //0
        C4Vector color;    //12
        C2Vector textCoord0; //28
        C2Vector textCoord1; //36
        C2Vector textCoord2; //44
    };

    std::vector<ParticleBuffStruct> buffer;
    buffer.reserve(4);
    mathfu::mat4 inverseLookAt = this->viewMatrix.Inverse();

    for (int i = 0; i < 4; i++) {
        ParticleBuffStruct record;
        record.position = ( inverseLookAt * mathfu::vec4(
                m0.x * vxs[i] + m1.x * vys[i] + viewPos.x,
                m0.y * vxs[i] + m1.y * vys[i] + viewPos.y,
                m0.z * vxs[i] + m1.z * vys[i] + viewPos.z,
                1.0
        )).xyz();
        record.color = mathfu::vec4_packed(mathfu::vec4(color, alpha));

        record.textCoord0 =
            mathfu::vec2(txs[i] * this->texScaleX + texStartX,
                         tys[i] * this->texScaleY + texStartY);

        record.textCoord1 =
            mathfu::vec2(
                txs[i] * (this->m_data->old.multiTextureParamX[0]/32.0f) + texPos[0].x,
                tys[i] * (this->m_data->old.multiTextureParamX[0]/32.0f) + texPos[0].y);
        record.textCoord2 =
            mathfu::vec2(
                txs[i] * (this->m_data->old.multiTextureParamX[1]/32.0f) + texPos[1].x,
                tys[i] * (this->m_data->old.multiTextureParamX[1]/32.0f) + texPos[1].y);

        buffer.emplace_back(record);
    }

    std::copy((uint8_t *)&buffer[0], (uint8_t *)&buffer[0] + buffer.size()*sizeof(ParticleBuffStruct), std::back_inserter(szVertexBuf));
}

void
ParticleEmitter::BuildQuad(
    std::vector<uint8_t> &szVertexBuf,
    mathfu::vec3 &m0, mathfu::vec3 &m1,
    mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
    float texStartX, float texStartY) {

        static const float vxs[4] = {-1, -1, 1, 1};
        static const float vys[4] = {1, -1, 1, -1};
        static const float txs[4] = {0, 0, 1, 1};
        static const float tys[4] = {0, 1, 0, 1};

        struct ParticleBuffStruct {
            C3Vector position; //0
            C4Vector color;    //12
            C2Vector textCoord0; //28
            C2Vector textCoord1; //36
            C2Vector textCoord2; //44
        };

        std::vector<ParticleBuffStruct> buffer;
        buffer.reserve(4);

        mathfu::mat4 inverseLookAt = this->viewMatrix.Inverse();

        for (int i = 0; i < 4; i++) {
            ParticleBuffStruct record;
            record.position = ( inverseLookAt * mathfu::vec4(
                m0.x * vxs[i] + m1.x * vys[i] + viewPos.x,
                m0.y * vxs[i] + m1.y * vys[i] + viewPos.y,
                m0.z * vxs[i] + m1.z * vys[i] + viewPos.z,
                1.0
            )).xyz();
            record.color = mathfu::vec4_packed(mathfu::vec4(color, alpha));

            record.textCoord0 =
                mathfu::vec2(txs[i] * this->texScaleX + texStartX,
                             tys[i] * this->texScaleY + texStartY);

            buffer.emplace_back(record);
        }

        std::copy((uint8_t *)&buffer[0], (uint8_t *)&buffer[0] + buffer.size()*sizeof(ParticleBuffStruct), std::back_inserter(szVertexBuf));
}

void ParticleEmitter::Render() {
    if (this->szVertexBuf.size() <= 0) return;

    auto particleShader = m_api->getM2ParticleShader();
    auto textureCache = m_api->getTextureCache();
    GLuint blackPixelText = m_api->getBlackPixelTexture();


    bool multitex = this->particleType >= 2;
    BlpTexture* tex0 = nullptr;
    if (multitex) {
        tex0 = textureCache->get(
                m_m2Data->textures.getElement(
                        this->m_data->old.texture_0
//                    *m_m2Data->texture_lookup_table.getElement(this->m_data->old.texture)
                )->filename.toString());
    } else {
        tex0 = textureCache->get(
                m_m2Data->textures.getElement(
                        this->m_data->old.texture
//                    *m_m2Data->texture_lookup_table.getElement(this->m_data->old.texture)
                )->filename.toString());
    }

    if (tex0 == nullptr || !tex0->getIsLoaded()) {
        return;
    }
    BlpTexture* tex1 = nullptr;
    BlpTexture* tex2 = nullptr;

    if (multitex) {
        tex1 = textureCache->get(
                m_m2Data->textures.getElement(
                        this->m_data->old.texture_1)->filename.toString());
        tex2 = textureCache->get(
                m_m2Data->textures.getElement(
                        this->m_data->old.texture_2)->filename.toString());
    }

    glActiveTexture(GL_TEXTURE0);
    if (tex0->getIsLoaded()) {
        glBindTexture(GL_TEXTURE_2D, tex0->getGlTexture());
    } else {
        glBindTexture(GL_TEXTURE_2D, blackPixelText);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glActiveTexture(GL_TEXTURE1);
    if (tex1!= nullptr && tex1->getIsLoaded()) {
        glBindTexture(GL_TEXTURE_2D, tex1->getGlTexture());
    } else {
        glBindTexture(GL_TEXTURE_2D, blackPixelText);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    if (tex2!= nullptr && tex2->getIsLoaded()) {
        glBindTexture(GL_TEXTURE_2D, tex2->getGlTexture());
    } else {
        glBindTexture(GL_TEXTURE_2D, blackPixelText);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int uPixelShader = -1;
    if (multitex) {
        if (this->m_data->old.flags & 0x20000000) {
            uPixelShader = 0;
        }
        if (this->m_data->old.flags & 0x40000000) {
            uPixelShader = 1;
        }
    }

    glUniform1i(particleShader->getUnf("uPixelShader"), uPixelShader);

    GLuint indexVBO;
    glGenBuffers(1, &indexVBO);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->szIndexBuff.size()*2, &this->szIndexBuff[0], GL_STREAM_DRAW);

    GLuint bufferVBO;
    glGenBuffers(1, &bufferVBO);
    glBindBuffer( GL_ARRAY_BUFFER, bufferVBO);
    glBufferData(GL_ARRAY_BUFFER, this->szVertexBuf.size(), &this->szVertexBuf[0], GL_STREAM_DRAW);

    int nFloats = 13;
    int stride = nFloats * 4;

    glVertexAttribPointer(+m2ParticleShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(+m2ParticleShader::Attribute::aColor, 4, GL_FLOAT, GL_FALSE, stride, (void *)12);
    glVertexAttribPointer(+m2ParticleShader::Attribute::aTexcoord0, 2, GL_FLOAT, GL_FALSE, stride, (void *)28);
    glVertexAttribPointer(+m2ParticleShader::Attribute::aTexcoord1, 2, GL_FLOAT, GL_FALSE, stride, (void *)36);
    glVertexAttribPointer(+m2ParticleShader::Attribute::aTexcoord2, 2, GL_FLOAT, GL_FALSE, stride, (void *)44);

    auto blendMode = m_data->old.blendingType;
    switch (blendMode) {
        case 0 : //Blend_Opaque
            glDisable(GL_BLEND);
            glUniform1f(particleShader->getUnf("uAlphaTest"), -1.0);
            break;
        case 1 : //Blend_AlphaKey
            glDisable(GL_BLEND);
//            glUniform1f(particleShader->getUnf("uAlphaTest"), 0.903921569);
            glUniform1f(particleShader->getUnf("uAlphaTest"), -1);
            break;
        case 2 : //Blend_Alpha
            glUniform1f(particleShader->getUnf("uAlphaTest"), -1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
            break;
        case 3 : //Blend_NoAlphaAdd
            glUniform1f(particleShader->getUnf("uAlphaTest"), -1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            //Override fog
//            glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_zero);
//            fogChanged = true;

            break;
        case 4 : //Blend_Add
            glUniform1f(particleShader->getUnf("uAlphaTest"), 0.00392157);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

//            glUniform3fv(m2Shader->getUnf("uFogColor"),  1, fog_zero);
//            fogChanged = true;
            break;

        case 5: //Blend_Mod
            glUniform1f(particleShader->getUnf("uAlphaTest"), 0.00392157);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);

//            glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_one);
//            fogChanged = true;
            break;

        case 6: //Blend_Mod2x
            glUniform1f(particleShader->getUnf("uAlphaTest"), 0.00392157);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

//            glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_half);
//            fogChanged = true;
            break;

        case 7: //Blend_Mod2x
            glUniform1f(particleShader->getUnf("uAlphaTest"), 0.00392157);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

//            glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_half);
//            fogChanged = true;
            break;
        default :
            glUniform1f(particleShader->getUnf("uAlphaTest"), -1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

            break;
    }


//    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glDrawElements(GL_TRIANGLES, this->szIndexBuff.size(), GL_UNSIGNED_SHORT, 0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer( GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &bufferVBO);
    glDeleteBuffers(1, &indexVBO);
}
