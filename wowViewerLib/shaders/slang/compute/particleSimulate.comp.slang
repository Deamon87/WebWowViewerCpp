//
// GPU particle simulation: one thread per emitter, a full port of
// ParticleEmitter::Update / InternalUpdate / StepUpdate (particleEmitter.cpp:386-630)
// including the exact CRndSeed call order (see crndSeed.slang), so the GPU path
// produces the same particles as the CPU path.
//
// Ping-pong: state.curBuf selects which of particlesA/B holds the live set; each
// substep reads the current buffer and writes the other one (compacted), then flips.
//

import glsl;

#include "../common/particle/particleGpu.slang"
#include "../common/particle/crndSeed.slang"

layout(std430, set=0, binding=0) StructuredBuffer<uint, ScalarDataLayout> emitterDispatchList;
layout(std430, set=0, binding=1) RWStructuredBuffer<GpuParticleState, ScalarDataLayout> particleStates;
layout(std430, set=0, binding=2) StructuredBuffer<GpuM2ParticleStatic, ScalarDataLayout> particleStatics;
layout(std430, set=0, binding=3) StructuredBuffer<GpuM2ParticleFrameProps, ScalarDataLayout> particleProps;
layout(std430, set=0, binding=4) RWStructuredBuffer<GpuParticle, ScalarDataLayout> particlesA;
layout(std430, set=0, binding=5) RWStructuredBuffer<GpuParticle, ScalarDataLayout> particlesB;
layout(std430, set=0, binding=6) StructuredBuffer<mat4, ScalarDataLayout> boneMatrix;
struct PlacementMatStruct { mat4 placementMat; mat4 invPlacementMat; };
layout(std430, set=0, binding=7) StructuredBuffer<PlacementMatStruct, ScalarDataLayout> placementMats;

struct ParticleSimPushConsts {
    mat4 viewMatrix;
    float deltaSeconds;
    float pad0;
    float pad1;
    float pad2;
};
[[vk::push_constant]] ParticleSimPushConsts pushConsts;

// -90 degree axis swap (M2Object::particleCoordinatesFix)
const static mat4 particleCoordinatesFix = mat4(
     0,1,0,0,
    -1,0,0,0,
     0,0,1,0,
     0,0,0,1
);

bool feq(float a, float b) {
    return abs(a - b) <= 0.001;
}

GpuParticle readParticle(int buf, uint baseA, uint baseB, int i) {
    // The object's regions in particlesA/B may start at different pool offsets
    uint idx = ((buf == 0) ? baseA : baseB) + uint(i);
    return (buf == 0) ? particlesA[idx] : particlesB[idx];
}
void writeParticle(int buf, uint baseA, uint baseB, int i, in GpuParticle p) {
    uint idx = ((buf == 0) ? baseA : baseB) + uint(i);
    if (buf == 0) particlesA[idx] = p; else particlesB[idx] = p;
}

// CParticleGenerator::GetLifeSpan — note the kill check passes the particle SEED
// (uint16 reinterpreted as int16, wrapping) while creation uses p.state.
float getLifeSpan(float lifespan, float lifespanVary, int stateOrSeed) {
    int s16 = stateOrSeed & 0xFFFF;
    if (s16 >= 32768) s16 -= 65536;
    return lifespan + (float(s16) * 0.000030518509) * lifespanVary;
}

// Generators' CreateParticle + the emitter's CreateParticle tail
// (CPlaneGenerator / CSphereGenerator + ParticleEmitter::CreateParticle)
GpuParticle createParticle(in GpuM2ParticleStatic st,
                           in GpuM2ParticleFrameProps props,
                           in GpuParticleState state,
                           float delta,
                           inout CRndSeed genSeed,
                           inout CRndSeed emitSeed) {
    GpuParticle p;
    p.age = 0.0;
    p.texPos0[0] = 0.0; p.texPos0[1] = 0.0;
    p.texPos1[0] = 0.0; p.texPos1[1] = 0.0;
    p.texVel0[0] = 0.0; p.texVel0[1] = 0.0;
    p.texVel1[0] = 0.0; p.texVel1[1] = 0.0;
    p.isDead = 0;

    vec3 pos = vec3(0.0);
    vec3 vel = vec3(0.0);

    float dvary = delta * CRndSeed_UniformPos(genSeed);
    float life = CRndSeed_Uniform(genSeed);

    int pstate;
    if (life < 1.0) {
        pstate = (life > -1.0) ? int(trunc(life * 32767.0 + 0.5)) : -32767;
    } else {
        pstate = 32767;
    }
    p.state = pstate;

    float lifespan = getLifeSpan(props.lifespan, st.lifespanVary, pstate);
    if (lifespan < 0.001) {
        lifespan = 0.001;
    }
    p.age = fract(dvary / lifespan) * lifespan; // fmod(dvary, lifespan)
    p.seed = 0xffffu & CRndSeed_uint32t(genSeed);

    float zSource = props.zSource;

    if (st.emitterType == 1) {
        // ---- CPlaneGenerator ----
        pos = vec3(
            CRndSeed_Uniform(genSeed) * props.emissionAreaX * 0.5,
            CRndSeed_Uniform(genSeed) * props.emissionAreaY * 0.5,
            0.0
        );

        // CalcVelocity — after the position draws, matching the CPU RNG call order
        float velocity = props.emissionSpeed * (1.0 + props.speedVariation * CRndSeed_Uniform(genSeed));

        if (zSource < 0.001) {
            float polar = props.verticalRange * CRndSeed_Uniform(genSeed);
            float azimuth = props.horizontalRange * CRndSeed_Uniform(genSeed);
            float sinPolar = sin(polar);
            float sinAzimuth = sin(azimuth);
            float cosPolar = cos(polar);
            float cosAzimuth = cos(azimuth);
            vel = vec3(
                cosAzimuth * sinPolar * velocity,
                sinAzimuth * sinPolar * velocity,
                cosPolar * velocity
            );
        } else {
            vec3 r0 = pos - vec3(0, 0, zSource);
            if (length(r0) > 0.0001) {
                vel = normalize(r0) * velocity;
            }
            // CPU leaves velocity unassigned (zero) in the degenerate case
        }
    } else {
        // ---- CSphereGenerator (emitterType == 2) ----
        float emissionArea = props.emissionAreaY - props.emissionAreaX;
        float radius = props.emissionAreaX + emissionArea * CRndSeed_UniformPos(genSeed);
        float polar = props.verticalRange * CRndSeed_Uniform(genSeed);
        float azimuth = props.horizontalRange * CRndSeed_Uniform(genSeed);
        float cosPolar = cos(polar);
        vec3 emissionDirection = vec3(
            cosPolar * cos(azimuth),
            cosPolar * sin(azimuth),
            sin(polar)
        );

        pos = emissionDirection * radius;

        // CalcVelocity — after the radius/angle draws, matching the CPU RNG call order
        float velocity = props.emissionSpeed * (1.0 + props.speedVariation * CRndSeed_Uniform(genSeed));

        vec3 r0 = vec3(0.5, 0.5, 0.5);
        if (feq(zSource, 0.0)) {
            bool particlesGoUp = (st.flags & 0x100) != 0;
            r0 = particlesGoUp ? vec3(0, 0, 1) : emissionDirection;
        } else {
            r0 = pos - vec3(0, 0, zSource);
            if (length(r0) > 0.0001) {
                r0 = normalize(r0);
            }
        }
        vel = r0 * velocity;
    }

    // ---- ParticleEmitter::CreateParticle tail ----
    if ((st.flags & 0x10) == 0) {
        mat4 m = matFromArray16(state.emitterModelMatrix);
        pos = (m * vec4(pos, 1.0)).xyz;
        vel = (m * vec4(vel, 0.0)).xyz;
        if ((st.flags & 0x2000) != 0) {
            pos.z = 0.0;
        }
    }
    if ((st.flags & 0x40) != 0) {
        float speedMul = 1.0 + props.speedVariation * CRndSeed_Uniform(emitSeed);
        vel += arrToVec3(state.burstVec) * speedMul;
    }
    if ((st.flags & 0x10000000) != 0) {
        // multitex
        float2 mp0[2] = { arrToVec2(st.multiTextureParam0_0), arrToVec2(st.multiTextureParam0_1) };
        float2 mp1[2] = { arrToVec2(st.multiTextureParam1_0), arrToVec2(st.multiTextureParam1_1) };
        for (int i = 0; i < 2; i++) {
            float2 texPos;
            texPos.x = CRndSeed_UniformPos(emitSeed);
            texPos.y = CRndSeed_UniformPos(emitSeed);
            float2 v2 = mp1[i] * CRndSeed_Uniform(emitSeed);
            float2 texVel = v2 + mp0[i];
            if (i == 0) { vec2ToArr(texPos, p.texPos0); vec2ToArr(texVel, p.texVel0); }
            else        { vec2ToArr(texPos, p.texPos1); vec2ToArr(texVel, p.texVel1); }
        }
    }

    vec3ToArr(pos, p.position);
    vec3ToArr(vel, p.velocity);
    return p;
}

struct ParticleForces {
    vec3 drift;
    vec3 velocity;
    vec3 position;
    float drag;
};

ParticleForces calculateForces(in GpuM2ParticleStatic st, in GpuM2ParticleFrameProps props, float delta) {
    ParticleForces forces;
    forces.drift = arrToVec3(st.windVector) * delta;
    vec3 g = arrToVec3(props.gravity);
    forces.velocity = g * delta;
    forces.position = g * (delta * delta * 0.5);
    forces.drag = min(st.drag * delta, 1.0);
    return forces;
}

// ParticleEmitter::UpdateParticle; returns false when the particle dies (shell rule)
bool updateParticle(in GpuM2ParticleStatic st,
                    in GpuM2ParticleFrameProps props,
                    in GpuParticleState state,
                    inout GpuParticle p,
                    float delta,
                    in ParticleForces forces) {
    if ((st.flags & 0x10000000) != 0) {
        // s = frac(s + v * dt)
        p.texPos0[0] = fract(p.texPos0[0] + delta * p.texVel0[0]);
        p.texPos0[1] = fract(p.texPos0[1] + delta * p.texVel0[1]);
        p.texPos1[0] = fract(p.texPos1[0] + delta * p.texVel1[0]);
        p.texPos1[1] = fract(p.texPos1[1] + delta * p.texVel1[1]);
    }

    vec3 pos = arrToVec3(p.position);
    vec3 vel = arrToVec3(p.velocity);

    vel = vel + forces.drift;

    // MoveParticle
    if ((st.flags & 0x4000) != 0 && (2.0 * delta < p.age)) {
        pos = pos + arrToVec3(state.deltaPosition);
    }

    vec3 r0 = vel * delta;

    vel = vel + forces.velocity;
    vel = vel * (1.0 - forces.drag);

    pos = pos + r0 + forces.position;

    if (st.emitterType == 2 && (st.flags & 0x80) != 0) {
        vec3 r1 = pos;
        if ((st.flags & 0x10) != 0) {
            if (dot(r1, r0) > 0.0) {
                return false;
            }
        } else {
            mat4 m = matFromArray16(state.emitterModelMatrix);
            r1 = pos - m[3].xyz;
            if (dot(r1, r0) > 0.0) {
                return false;
            }
        }
    }

    vec3ToArr(pos, p.position);
    vec3ToArr(vel, p.velocity);
    return true;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    uint emitterIdx = emitterDispatchList[gl_WorkGroupID.x];

    GpuParticleState state = particleStates[emitterIdx];
    GpuM2ParticleStatic st = particleStatics[state.staticsIndex];
    GpuM2ParticleFrameProps props = particleProps[state.propsIndex];

    float delta = pushConsts.deltaSeconds;
    mat4 viewMatrix = pushConsts.viewMatrix;

    uint baseA = uint(state.particleOffset);
    uint baseB = uint(state.particleOffsetB);

    // ---------------- ParticleEmitter::Update ----------------
    float3 prevPosition = matFromArray16(state.emitterModelMatrix)[3].xyz;

    mat4 transformMat =
        (placementMats[state.placementMatIndex].placementMat *
         boneMatrix[state.boneOutOffset + st.bone] *
         mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, st.position[0], st.position[1], st.position[2], 1)) *
        particleCoordinatesFix;

    // UpdateXform (frameOfReference is always null from M2Object)
    state.emitterModelMatrix[0]  = transformMat[0][0];
    state.emitterModelMatrix[1]  = transformMat[0][1];
    state.emitterModelMatrix[2]  = transformMat[0][2];
    state.emitterModelMatrix[3]  = transformMat[0][3];
    state.emitterModelMatrix[4]  = transformMat[1][0];
    state.emitterModelMatrix[5]  = transformMat[1][1];
    state.emitterModelMatrix[6]  = transformMat[1][2];
    state.emitterModelMatrix[7]  = transformMat[1][3];
    state.emitterModelMatrix[8]  = transformMat[2][0];
    state.emitterModelMatrix[9]  = transformMat[2][1];
    state.emitterModelMatrix[10] = transformMat[2][2];
    state.emitterModelMatrix[11] = transformMat[2][3];
    state.emitterModelMatrix[12] = transformMat[3][0];
    state.emitterModelMatrix[13] = transformMat[3][1];
    state.emitterModelMatrix[14] = transformMat[3][2];
    state.emitterModelMatrix[15] = transformMat[3][3];

    state.inheritedScale = length(transformMat[0].xyz);

    int cur = state.curBuf;
    int n = state.particleCount;

    if (delta > 0.0) {
        if ((st.flags & 0x4000) != 0) {
            vec3ToArr(transformMat[3].xyz - prevPosition, state.fullPosDelta);
            float x = st.followMult * (length(arrToVec3(state.fullPosDelta)) / delta) + st.followBase;
            if (x >= 0.0)
                x = min(x, 1.0);

            vec3ToArr(arrToVec3(state.fullPosDelta) * x, state.deltaPosition);
        }

        if ((st.flags & 0x40) != 0) {
            state.burstTime += delta;
            const float frameTime = 30.0 / 1000.0;
            float burstTime = state.burstTime;
            if (burstTime > frameTime) {
                state.burstTime = 0.0;
                if (n == 0) {
                    float frameAmount = frameTime / burstTime;
                    float3 dPos = transformMat[3].xyz - prevPosition;
                    vec3ToArr(dPos * (frameAmount * st.burstMultiplier), state.burstVec);
                } else {
                    vec3ToArr(float3(0, 0, 0), state.burstVec);
                }
            }
        }

        // ---------------- InternalUpdate ----------------
        float d = max(delta, 0.0);
        float remainder;
        int stepUpdates;
        if (d < 0.1) {
            state.deltaPosition[0] = state.fullPosDelta[0];
            state.deltaPosition[1] = state.fullPosDelta[1];
            state.deltaPosition[2] = state.fullPosDelta[2];
            stepUpdates = 0;
            remainder = d;
        } else {
            float temp0 = floor(d / 0.1);
            remainder = temp0 * -0.1 + d;
            stepUpdates = min(int(floor(props.lifespan / 0.1)), int(temp0));
            vec3ToArr(arrToVec3(state.fullPosDelta) * (1.0 / float(stepUpdates + 1)), state.deltaPosition);
        }

        for (int step = 0; step <= stepUpdates; step++) {
            float dt = (step < stepUpdates) ? 0.1 : remainder;
            int w = 1 - cur;
            int outCount = 0;

            // ---- age & kill ----
            for (int i = 0; i < n; i++) {
                GpuParticle p = readParticle(cur, baseA, baseB, i);
                p.age += dt;

                float ls = max(getLifeSpan(props.lifespan, st.lifespanVary, int(p.seed)), 0.001);
                p.isDead = (p.age > ls) ? 1 : 0;
                if (p.isDead == 0) {
                    writeParticle(w, baseA, baseB, outCount, p);
                    outCount++;
                }
            }

            ParticleForces forces = calculateForces(st, props, dt);

            // ---- emit ----
            if (props.isEnabled != 0) {
                CRndSeed genSeed = CRndSeed_create(0);
                genSeed.currentValue = state.generatorSeedValue;
                genSeed.accumulator = state.generatorSeedAccum;
                CRndSeed emitSeed = CRndSeed_create(0);
                emitSeed.currentValue = state.emitterSeedValue;
                emitSeed.accumulator = state.emitterSeedAccum;

                float rate = props.emissionRate + CRndSeed_Uniform(genSeed) * st.emissionRateVary;
                state.emission += dt * rate;
                while (state.emission > 1.0) {
                    if (outCount < state.capacity) {
                        GpuParticle np = createParticle(st, props, state, dt, genSeed, emitSeed);
                        writeParticle(w, baseA, baseB, outCount, np);
                        outCount++;
                    }
                    state.emission -= 1.0;
                }

                state.generatorSeedValue = genSeed.currentValue;
                state.generatorSeedAccum = genSeed.accumulator;
                state.emitterSeedValue = emitSeed.currentValue;
                state.emitterSeedAccum = emitSeed.accumulator;
            }

            // ---- integrate ----
            // NB: the updated particle must be written back unconditionally —
            // updateParticle edits a local copy (CPU StepUpdate edits in place
            // through a reference). Writing back only dead particles freezes
            // position/velocity at their creation values.
            for (int i = 0; i < outCount; i++) {
                GpuParticle p = readParticle(w, baseA, baseB, i);
                if (!updateParticle(st, props, state, p, dt, forces)) {
                    p.isDead = 1;
                }
                writeParticle(w, baseA, baseB, i, p);
            }

            n = outCount;
            cur = w;
        }

        state.curBuf = cur;
        state.particleCount = n;
    }

    // ---- calculateQuadToViewEtc (runs every frame, drives the vertex shader) ----
    mat4 s_particleToView = ((st.flags & 0x10) != 0)
        ? (viewMatrix * matFromArray16(state.emitterModelMatrix))
        : viewMatrix;
    for (int c = 0; c < 4; c++) {
        state.s_particleToView[c * 4 + 0] = s_particleToView[c][0];
        state.s_particleToView[c * 4 + 1] = s_particleToView[c][1];
        state.s_particleToView[c * 4 + 2] = s_particleToView[c][2];
        state.s_particleToView[c * 4 + 3] = s_particleToView[c][3];
    }

    if ((st.flags & 0x1000) != 0) {
        mat3 qtv = mat3(
            s_particleToView[0].xyz,
            s_particleToView[1].xyz,
            s_particleToView[2].xyz
        );
        if ((st.flags & 0x10) != 0 && abs(state.inheritedScale) > 0.0) {
            qtv = qtv * (1.0 / state.inheritedScale);
        }
        state.p_quadToView0 = vec4(qtv[0], 0.0);
        state.p_quadToView1 = vec4(qtv[1], 0.0);
        state.p_quadToView2 = vec4(qtv[2], 0.0);

        vec3 zvec = qtv[2];
        if ((zvec.x * zvec.x + zvec.y * zvec.y + zvec.z * zvec.z) <= 0.00000023841858) {
            state.p_quadToViewZVector = vec4(0, 0, 1.0, 0);
        } else {
            state.p_quadToViewZVector = vec4(normalize(zvec), 0.0);
        }
    }

    particleStates[emitterIdx] = state;
}
