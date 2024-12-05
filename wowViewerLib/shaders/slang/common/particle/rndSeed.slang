#ifndef PARTICLE_SHADER_CRNDSEED_SLANG
#define PARTICLE_SHADER_CRNDSEED_SLANG

struct CRndSeed {
    uint state;

    [mutating]
    uint uint32t() {
        uint x = this.state;

        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;

        this.state = x;

        return x;
    }


    //Return  [-1, 1]
    [mutating]
    float Uniform() {
        const uint mantissaMask = 0x7fffff;
        const uint one = 0x3f800000;

        uint u = this.uint32t();
        u &= mantissaMask;
        u |= one;

        float r2 = asfloat(u);
        if (int(u) < 0) {
            return 2.0f - r2;
        } else {
            return r2 - 2.0f;
        }
    }

    //Return  [0, 1)
    [mutating]
    float UniformPos() {
        const uint mantissaMask = 0x7fffff;
        const uint one = 0x3f800000;

        uint u = this.uint32t();
        u &= mantissaMask;
        u |= one;

        float r2 = asfloat(u);
        return r2 - 1.0;
    }
};

#endif // PARTICLE_SHADER_CRNDSEED_SLANG