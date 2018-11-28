//
// Created by deamon on 14.12.17.
//

#include "particleEmitter.h"
#include "../../../gapi/interface/meshes/IParticleMesh.h"
#include "../../algorithms/mathHelper.h"
#include "../../algorithms/animate.h"
#include "../../shader/ShaderDefinitions.h"
#include "generators/CParticleGenerator.h"
#include "generators/CSphereGenerator.h"
#include "generators/CPlaneGenerator.h"
#include "../../../gapi/interface/IDevice.h"


static GBufferBinding staticM2ParticleBindings[5] = {
    {+m2ParticleShader::Attribute::aPosition, 3, GL_FLOAT, false, 13*4, 0 },
    {+m2ParticleShader::Attribute::aColor, 4, GL_FLOAT, false, 13*4, 12},
    {+m2ParticleShader::Attribute::aTexcoord0, 2, GL_FLOAT, false, 13*4, 28},
    {+m2ParticleShader::Attribute::aTexcoord1, 2, GL_FLOAT, false, 13*4, 36},
    {+m2ParticleShader::Attribute::aTexcoord2, 2, GL_FLOAT, false, 13*4, 44},
};


ParticleEmitter::ParticleEmitter(IWoWInnerApi *api, M2Particle *particle, M2Object *m2Object) : m_seed(0), m_api(api), m2Object(m2Object) {

    if (!randTableInited) {
        for (int i = 0; i < 128; i++) {
            RandTable[i] = (float)std::rand() / (float)RAND_MAX;
        }
        randTableInited = true;
    }

    m_data = particle;
    createMesh();

    switch (m_data->old.emitterType) {
        case 1:
            this->generator = new CPlaneGenerator(this->m_seed, particle);
            break;
        case 2:
            this->generator = new CSphereGenerator(this->m_seed, particle, 0 != (m_data->old.flags & 0x100));
            break;
        default:
            this->generator = nullptr;
            std::cout << "Found unimplemented generator " << m_data->old.emitterType << std::flush;
            break;
    }

//        std::cout << "particleId = " << m_data->old.particleId
//                  << "Mentioned models :" << std::endl
//                  << "geometry_model_filename = " << m_data->old.geometry_model_filename.toString() << std::endl
//                  << "recursion_model_filename = " << m_data->old.recursion_model_filename.toString()
//                  << std::endl << std::endl << std::flush;

    const float followDen = m_data->old.followSpeed2 - m_data->old.followSpeed1;
    if (!feq(followDen, 0)) {
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

    int colsVal = cols;
    int colBitsCount = -1;
    do
    {
        ++colBitsCount;
        colsVal >>= 1;
    }
    while ( colsVal );

    this->textureColBits = colBitsCount;
    this->textureColMask = cols - 1;
    if (m_data->old.flags & 0x200000) {
        this->textureStartIndex = this->m_seed.uint32t() & this->textureIndexMask;
    }
    this->texScaleX = 1.0f / cols;
    this->texScaleY = 1.0f / rows;

    if (m_data->old.flags & 0x10100000) {
        const bool isMultitex = (0 != (1 & (m_data->old.flags >> 0x1c)));
        if (isMultitex) {
            this->particleType = 2;
        }
        else {
            this->particleType = 3;
        }
    }
}
PACK(
struct meshParticleWideBlockPS {
    float uAlphaTest;
    float padding[3]; // according to std140
    int uPixelShader;
    float padding2[3];
});

EGxBlendEnum PaticleBlendingModeToEGxBlendEnum1 [14] =
    {
        EGxBlendEnum::GxBlend_Opaque,
        EGxBlendEnum::GxBlend_AlphaKey,
        EGxBlendEnum::GxBlend_Alpha,
        EGxBlendEnum::GxBlend_NoAlphaAdd,
        EGxBlendEnum::GxBlend_Add,
        EGxBlendEnum::GxBlend_Mod,
        EGxBlendEnum::GxBlend_Mod2x,
        EGxBlendEnum::GxBlend_BlendAdd
    };


void ParticleEmitter::createMesh() {
    IDevice *device = m_api->getDevice();

    //Create Buffers
    for (int i = 0; i < 4; i++) {
        frame[i].m_indexVBO = device->createIndexBuffer();
        frame[i].m_bufferVBO = device->createVertexBuffer();

        frame[i].m_bindings = device->createVertexBufferBindings();
        frame[i].m_bindings->setIndexBuffer(frame[i].m_indexVBO);

        GVertexBufferBinding vertexBinding;
        vertexBinding.vertexBuffer = frame[i].m_bufferVBO;
        vertexBinding.bindings = std::vector<GBufferBinding>(&staticM2ParticleBindings[0],
                                                             &staticM2ParticleBindings[5]);

        frame[i].m_bindings->addVertexBufferBinding(vertexBinding);
        frame[i].m_bindings->save();


        //Get shader
        HGShaderPermutation shaderPermutation = m_api->getDevice()->getShader("m2ParticleShader", nullptr);

        //Create mesh
        gMeshTemplate meshTemplate(frame[i].m_bindings, shaderPermutation);


        uint8_t blendMode = m_data->old.blendingType;

        meshTemplate.depthWrite = blendMode <= 1;
        meshTemplate.depthCulling = true;
        meshTemplate.backFaceCulling = true;

        if (blendMode == 4)
            blendMode = 3;

//    meshTemplate.blendMode = static_cast<EGxBlendEnum>(blendMode);
        meshTemplate.blendMode = static_cast<EGxBlendEnum>(blendMode);//M2BlendingModeToEGxBlendEnum1[blendMode];

        meshTemplate.start = 0;
        meshTemplate.end = 0;
        meshTemplate.element = GL_TRIANGLES;

        bool multitex = this->particleType >= 2;
        HBlpTexture tex0 = nullptr;
        if (multitex) {
            tex0 = m2Object->getBlpTextureData(this->m_data->old.texture_0);
        } else {
            tex0 = m2Object->getBlpTextureData(this->m_data->old.texture);
        }
        meshTemplate.texture[0] = m_api->getDevice()->createBlpTexture(tex0, true, true);
        if (multitex) {
            HBlpTexture tex1 = m2Object->getBlpTextureData(this->m_data->old.texture_1);
            HBlpTexture tex2 = m2Object->getBlpTextureData(this->m_data->old.texture_2);

            meshTemplate.texture[1] = m_api->getDevice()->createBlpTexture(tex1, true, true);
            meshTemplate.texture[2] = m_api->getDevice()->createBlpTexture(tex2, true, true);
        }

        meshTemplate.textureCount = (multitex) ? 3 : 1;

        meshTemplate.vertexBuffers[0] = m_api->getSceneWideUniformBuffer();
        meshTemplate.vertexBuffers[1] = nullptr;
        meshTemplate.vertexBuffers[2] = nullptr;

        meshTemplate.fragmentBuffers[0] = m_api->getSceneWideUniformBuffer();
        meshTemplate.fragmentBuffers[1] = nullptr;
        meshTemplate.fragmentBuffers[2] = m_api->getDevice()->createUniformBuffer(sizeof(meshParticleWideBlockPS));

        meshParticleWideBlockPS &blockPS = meshTemplate.fragmentBuffers[2]->getObject<meshParticleWideBlockPS>();
        blockPS.uAlphaTest = 0.0039215689f;
        int uPixelShader = -1;
        if (multitex) {
            if (this->m_data->old.flags & 0x20000000) {
                uPixelShader = 0;
            }
            if (this->m_data->old.flags & 0x40000000) {
                uPixelShader = 1;
            }
        }
        blockPS.uPixelShader = uPixelShader;

        meshTemplate.fragmentBuffers[2]->save(true);

        frame[i].m_mesh = m_api->getDevice()->createParticleMesh(meshTemplate);
    }
}


bool ParticleEmitter::randTableInited = false;
float ParticleEmitter::RandTable[128] = {};

void ParticleEmitter::resizeParticleBuffer() {
    int newCount = this->generator->GetMaxEmissionRate() * this->generator->GetMaxLifeSpan() * 1.15;
//    if (newCount > (int)this->particles.size()) {
//        this->particles.reserve(newCount);
//    }
}

void ParticleEmitter::Update(animTime_t delta, mathfu::mat4 &boneModelMat, mathfu::vec3 invMatTransl) {
    if (getGenerator() == nullptr) return;

    if (this->particles.size() <= 0 && !isEnabled) return;

    this->resizeParticleBuffer();

    mathfu::vec3 lastPos = -transform.TranslationVector3D();
    mathfu::vec3 currPos = -boneModelMat.TranslationVector3D();
    this->transform = boneModelMat;

    this->inheritedScale = this->transform.GetColumn(0).xyz().Length();
    m_invMatTransl = invMatTransl;


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
                this->burstVec = dPos * mathfu::vec3(frameAmount * this->m_data->old.BurstMultiplier);
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
    if (!isEnabled) return;

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

    mathfu::mat4 rotateMat = MathHelper::RotationY(toRadian(-90));

    this->generator->CreateParticle(p, delta);
    p.velocity = (rotateMat.Inverse().Transpose() * mathfu::vec4(p.velocity, 0.0f)).xyz();

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
        forces.drift = forces.drift * mathfu::vec3(delta);
    }
    else {
        forces.drift = mathfu::vec3(this->m_data->old.WindVector) * mathfu::vec3(delta);
    }

    auto g = this->generator->GetGravity();
    forces.velocity = g * mathfu::vec3(delta);
    forces.position = g * mathfu::vec3(delta * delta * 0.5f);
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
    if ((this->m_data->old.flags & 0x4000) && (2 * delta < p.age)) {
        p.position = p.position + this->deltaPosition;
    }

    mathfu::vec3 r0 = p.velocity * mathfu::vec3(delta); // v*dt

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
    

    CParticle2 &p = particles[particles.size() - 1];
    return p;
}

void ParticleEmitter::prepearBuffers(mathfu::mat4 &viewMatrix) {
    if (particles.size() == 0 && this->generator != nullptr) {
        return;
    }

    if (this->m_data->old.flags & 0x10 ) {
        // apply the model transform
        this->particleToView = viewMatrix * this->transform;
    }
    else {
        this->particleToView = viewMatrix ;
    }

    this->viewMatrix = viewMatrix;
    this->inverseViewMatrix = viewMatrix.Inverse();
    // Mat34.Col(this.particleNormal, viewMatrix, 2);
    // Build vertices for each particle

    // Vertex format: {float3 pos; float3 norm; float4 col; float2 texcoord} = 12
    // Multitex format: {float3 pos; float4 col; float2 texcoord[3]} = 13

    szVertexBuf = std::vector<ParticleBuffStructQuad>(0);
    szIndexBuff = std::vector<uint16_t>(0);
    // TODO: z-sort
    int vo = 0;
    for (int i = 0; i < particles.size(); i++) {
        CParticle2 &p = this->particles[i];
        int rendered = this->RenderParticle(p, szVertexBuf);
        if (rendered > 0) {
//            for (int j = 0; j < rendered; j++) {
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
//            }
        }
    }
}

int ParticleEmitter::RenderParticle(CParticle2 &p, std::vector<ParticleBuffStructQuad> &szVertexBuf) {
    float twinkle = this->m_data->old.TwinklePercent;
    auto &twinkleRange = this->m_data->old.twinkleScale;

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

    if (twinkle < ParticleEmitter::RandTable[rndIdx]) {
        return 0;
    }

    int amountrendered = 0;
    CRndSeed rand(seed);

    mathfu::vec3 defaultColor(255.0f, 255.0f, 255.0f);
    mathfu::vec2 defaultScale(1.0f, 1.0f);
    float defaultAlpha = 1.0f;
    uint16_t defaultCell = 1;

    float percentTime = p.age /  this->getGenerator()->GetMaxLifeSpan();
    mathfu::vec3 color = animatePartTrack<C3Vector, mathfu::vec3>(percentTime, &m_data->old.colorTrack, defaultColor) / 255.0f;
    mathfu::vec2 scale = animatePartTrack<C2Vector, mathfu::vec2>(percentTime, &m_data->old.scaleTrack, defaultScale);
    float alpha = animatePartTrack<fixed16, float>(percentTime, &m_data->old.alphaTrack, defaultAlpha);
//    if (alpha < 0.9) alpha = 1.0;
//    if (color.x < 0.1 && color.y < 0.1 && color.z < 0.1) {
//        color = mathfu::vec3(1.0,1.0,1.0);
//    }

    uint16_t headCell = animatePartTrack<uint16_t, uint16_t>(percentTime, &m_data->old.headCellTrack, defaultCell);
    uint16_t tailCell = animatePartTrack<uint16_t, uint16_t>(percentTime, &m_data->old.tailCellTrack, defaultCell);

    if ((this->m_data->old.flags & 0x10000 && (m_data->old.headCellTrack.values.size == 0))) {
        headCell = (uint16_t) ((rand.uint32t() & this->textureIndexMask) & 0xFFFF);
    } else {
        headCell = (uint16_t) (((headCell + this->textureStartIndex) & this->textureIndexMask) & 0xFFFF);
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
    float weight = twinkleVary * ParticleEmitter::RandTable[rndIdx] + twinkleMin;
    scale = scale * weight;
    if (this->m_data->old.flags & 0x20) {
        scale = scale * this->inheritedScale;
    }
    mathfu::vec3 viewPos = (this->particleToView * mathfu::vec4(pos, 1.0)).xyz();

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
        mathfu::vec3 viewVel = vel * -1.0f;
        viewVel = (this->particleToView * mathfu::vec4(viewVel, 0)).xyz() * trailTime;
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
    std::vector<ParticleBuffStructQuad> &szVertexBuf,
    mathfu::vec3 &m0, mathfu::vec3 &m1,
    mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
    float texStartX, float texStartY, mathfu::vec2 *texPos) {

    static const float vxs[4] = {-1, -1, 1, 1};
    static const float vys[4] = {1, -1, 1, -1};
    static const float txs[4] = {0, 0, 1, 1};
    static const float tys[4] = {0, 1, 0, 1};
    ParticleBuffStructQuad record;

    mathfu::mat4 &inverseLookAt = this->inverseViewMatrix;

    for (int i = 0; i < 4; i++) {

        record.particle[i].position = ( inverseLookAt * mathfu::vec4(
                m0 * vxs[i] + m1 * vys[i] + viewPos,
                1.0
        )).xyz();
        record.particle[i].color = mathfu::vec4_packed(mathfu::vec4(color, alpha));

        record.particle[i].textCoord0 =
            mathfu::vec2(txs[i] * this->texScaleX + texStartX,
                         tys[i] * this->texScaleY + texStartY);

        record.particle[i].textCoord1 =
            mathfu::vec2(
                txs[i] * (this->m_data->old.multiTextureParamX[0]/32.0f) + texPos[0].x,
                tys[i] * (this->m_data->old.multiTextureParamX[0]/32.0f) + texPos[0].y);
        record.particle[i].textCoord2 =
            mathfu::vec2(
                txs[i] * (this->m_data->old.multiTextureParamX[1]/32.0f) + texPos[1].x,
                tys[i] * (this->m_data->old.multiTextureParamX[1]/32.0f) + texPos[1].y);

    }

    szVertexBuf.push_back(record);
}

void
ParticleEmitter::BuildQuad(
    std::vector<ParticleBuffStructQuad> &szVertexBuf,
    mathfu::vec3 &m0, mathfu::vec3 &m1,
    mathfu::vec3 &viewPos, mathfu::vec3 &color, float alpha,
    float texStartX, float texStartY) {

        static const float vxs[4] = {-1, -1, 1, 1};
        static const float vys[4] = {1, -1, 1, -1};
        static const float txs[4] = {0, 0, 1, 1};
        static const float tys[4] = {0, 1, 0, 1};

        mathfu::mat4 &inverseLookAt = this->inverseViewMatrix;

    static ParticleBuffStructQuad record;
    const C4Vector colorCombined = mathfu::vec4_packed(mathfu::vec4(color, alpha));
    //Unroll 1
    record.particle[0].position = ( inverseLookAt * mathfu::vec4(
                m0 * vxs[0] + m1 * vys[0] + viewPos,
                1.0
            )).xyz();
    record.particle[0].color = colorCombined;

    record.particle[0].textCoord0 =
        mathfu::vec2(txs[0] * this->texScaleX + texStartX,
                     tys[0] * this->texScaleY + texStartY);



    //Unroll 2
    record.particle[1].position = ( inverseLookAt * mathfu::vec4(
            m0 * vxs[1] + m1 * vys[1] + viewPos,
            1.0
    )).xyz();
    record.particle[1].color = colorCombined;
    record.particle[1].textCoord0 =
            mathfu::vec2(txs[1] * this->texScaleX + texStartX,
                         tys[1] * this->texScaleY + texStartY);

    //Unroll 3
    record.particle[2].position = ( inverseLookAt * mathfu::vec4(
            m0 * vxs[2] + m1 * vys[2] + viewPos,
            1.0
    )).xyz();
    record.particle[2].color = colorCombined;
    record.particle[2].textCoord0 =
            mathfu::vec2(txs[2] * this->texScaleX + texStartX,
                         tys[2] * this->texScaleY + texStartY);


    //Unroll 4
    record.particle[3].position = ( inverseLookAt * mathfu::vec4(
            m0 * vxs[3] + m1 * vys[3] + viewPos,
            1.0
    )).xyz();
    record.particle[3].color = colorCombined;
    record.particle[3].textCoord0 =
            mathfu::vec2(txs[3] * this->texScaleX + texStartX,
                         tys[3] * this->texScaleY + texStartY);


    szVertexBuf.push_back(record);
}

void ParticleEmitter::collectMeshes(std::vector<HGMesh> &meshes, int renderOrder) {
    return;
    if (this->szVertexBuf.size() <= 1) return;

    HGParticleMesh mesh = frame[m_api->getDevice()->getUpdateFrameNumber()].m_mesh;
    mesh->setRenderOrder(renderOrder);
    meshes.push_back(mesh);
}

void ParticleEmitter::updateBuffers() const {
    return;
    if (szVertexBuf.size() == 0 ) return;
    auto &currentFrame = frame[m_api->getDevice()->getUpdateFrameNumber()];
    currentFrame.m_indexVBO->uploadData((void *) szIndexBuff.data(), (int) (szIndexBuff.size() * sizeof(uint16_t)));
    currentFrame.m_bufferVBO->uploadData((void *) szVertexBuf.data(), (int) (szVertexBuf.size() * sizeof(ParticleBuffStructQuad)));

    currentFrame.m_mesh->setEnd(szIndexBuff.size());
}
