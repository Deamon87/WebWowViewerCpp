struct RndSeed {
    uint state;
};

uint RndSeed__uint32t(inout RndSeed rnd) {
    uint x = rnd.state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    rnd.state = x;

    return x;
}

//Return  [-1, 1]
float RndSeed__Uniform(inout RndSeed rnd) {
    const uint mantissaMask = 0x7fffff;
    const uint one = 0x3f800000;

    uint u = RndSeed__uint32t(rnd);
    u &= mantissaMask;
    u |= one;

    float r2 = uintBitsToFloat(u);
    if (int(u) < 0) {
        return 2.0f - r2;
    } else {
        return r2 - 2.0f;
    }
}

//Return  [0, 1)
float RndSeed__UniformPos(inout RndSeed rnd) {
    const uint mantissaMask = 0x7fffff;
    const uint one = 0x3f800000;

    uint u = RndSeed__uint32t(rnd);
    u &= mantissaMask;
    u |= one;

    float r2 = uintBitsToFloat(u);
    return r2 - 1.0;
}