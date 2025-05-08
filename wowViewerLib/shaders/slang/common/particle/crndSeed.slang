//
// Exact GPU port of the CPU particle RNG (engine/managers/particles/CRndSeed.cpp).
// State is two uint32s (currentValue, accumulator). The 256-entry table holds the
// little-endian uint32 read of the byte noise array at each byte offset, i.e.
// noiseTableU32[i] == *(const uint32_t*)&noiseArray[i] on a little-endian CPU.
//
#ifndef PARTICLE_SHADER_CRNDSEED_SLANG
#define PARTICLE_SHADER_CRNDSEED_SLANG

struct CRndSeed {
    uint currentValue;
    uint accumulator;
};

static const uint noiseTableU32[256] = {
    0x9927148Eu, 0xFD992714u, 0xAAFD9927u, 0xC7AAFD99u, 0x08C7AAFDu, 0xD508C7AAu, 0xE6D508C7u, 0x3EE6D508u,
    0x1F3EE6D5u, 0xF61F3EE6u, 0xBBF61F3Eu, 0x55BBF61Fu, 0xDA55BBF6u, 0x75DA55BBu, 0xA075DA55u, 0x4AA075DAu,
    0x6A4AA075u, 0xE86A4AA0u, 0xBDE86A4Au, 0x97BDE86Au, 0xFF97BDE8u, 0xDEFF97BDu, 0x9BDEFF97u, 0xBC9BDEFFu,
    0x9FBC9BDEu, 0x819FBC9Bu, 0x8A819FBCu, 0xA18A819Fu, 0x46A18A81u, 0x6E46A18Au, 0x0B6E46A1u, 0xE30B6E46u,
    0x63E30B6Eu, 0x7663E30Bu, 0x7A7663E3u, 0x6C7A7663u, 0x5D6C7A76u, 0x885D6C7Au, 0xD3885D6Cu, 0x69D3885Du,
    0xCA69D388u, 0xC3CA69D3u, 0x47C3CA69u, 0xB947C3CAu, 0x25B947C3u, 0x8325B947u, 0xAB8325B9u, 0xA2AB8325u,
    0x3FA2AB83u, 0xA63FA2ABu, 0x41A63FA2u, 0x7C41A63Fu, 0xBA7C41A6u, 0xE5BA7C41u, 0xACE5BA7Cu, 0x95ACE5BAu,
    0x0195ACE5u, 0x7E0195ACu, 0xCF7E0195u, 0x09CF7E01u, 0xC109CF7Eu, 0xD9C109CFu, 0x62D9C109u, 0x7062D9C1u,
    0x717062D9u, 0x8D717062u, 0xDB8D7170u, 0x05DB8D71u, 0x0205DB8Du, 0x240205DBu, 0x87240205u, 0xEF872402u,
    0x54EF8724u, 0xC654EF87u, 0xD4C654EFu, 0x37D4C654u, 0x3037D4C6u, 0xD03037D4u, 0x1BD03037u, 0xCB1BD030u,
    0x7BCB1BD0u, 0xB87BCB1Bu, 0xE4B87BCBu, 0xD8E4B87Bu, 0xECD8E4B8u, 0x49ECD8E4u, 0xCE49ECD8u, 0xADCE49ECu,
    0xDCADCE49u, 0x13DCADCEu, 0xA913DCADu, 0x94A913DCu, 0xC494A913u, 0x8FC494A9u, 0x398FC494u, 0xAE398FC4u,
    0x0DAE398Fu, 0x180DAE39u, 0x52180DAEu, 0xDD52180Du, 0x0EDD5218u, 0x780EDD52u, 0xFA780EDDu, 0xF5FA780Eu,
    0x85F5FA78u, 0x5885F5FAu, 0xD25885F5u, 0xAFD25885u, 0x6DAFD258u, 0xA46DAFD2u, 0xB2A46DAFu, 0x53B2A46Du,
    0x3B53B2A4u, 0x513B53B2u, 0xA5513B53u, 0x50A5513Bu, 0xBE50A551u, 0xFCBE50A5u, 0x2DFCBE50u, 0xF42DFCBEu,
    0x11F42DFCu, 0x4811F42Du, 0x984811F4u, 0x16984811u, 0xF1169848u, 0x86F11698u, 0xDF86F116u, 0x3DDF86F1u,
    0x663DDF86u, 0x5E663DDFu, 0x445E663Du, 0x2E445E66u, 0x2F2E445Eu, 0x362F2E44u, 0x07362F2Eu, 0x6B07362Fu,
    0x176B0736u, 0x8B176B07u, 0x298B176Bu, 0x4C298B17u, 0xB64C298Bu, 0xE2B64C29u, 0x89E2B64Cu, 0x5F89E2B6u,
    0xE75F89E2u, 0xCDE75F89u, 0xA7CDE75Fu, 0x21A7CDE7u, 0xE121A7CDu, 0x4DE121A7u, 0xC94DE121u, 0x65C94DE1u,
    0xED65C94Du, 0xFEED65C9u, 0xEEFEED65u, 0x9CEEFEEDu, 0x239CEEFEu, 0x33239CEEu, 0x7D33239Cu, 0xB77D3323u,
    0x04B77D33u, 0x9E04B77Du, 0x9A9E04B7u, 0x2A9A9E04u, 0x402A9A9Eu, 0xB3402A9Au, 0x10B3402Au, 0x5B10B340u,
    0xF35B10B3u, 0x82F35B10u, 0x7782F35Bu, 0x1C7782F3u, 0x921C7782u, 0x20921C77u, 0x4E20921Cu, 0x1E4E2092u,
    0x571E4E20u, 0x22571E4Eu, 0x7222571Eu, 0x06722257u, 0x8C067222u, 0x678C0672u, 0x2C678C06u, 0x732C678Cu,
    0xFB732C67u, 0x59FB732Cu, 0xC259FB73u, 0x0AC259FBu, 0xBF0AC259u, 0x79BF0AC2u, 0x5C79BF0Au, 0xF95C79BFu,
    0x0CF95C79u, 0x280CF95Cu, 0x1A280CF9u, 0x121A280Cu, 0x68121A28u, 0x7468121Au, 0x34746812u, 0x19347468u,
    0x42193474u, 0xB1421934u, 0xC0B14219u, 0x84C0B142u, 0xF884C0B1u, 0x38F884C0u, 0xF038F884u, 0x15F038F8u,
    0x9D15F038u, 0x609D15F0u, 0xF2609D15u, 0x3AF2609Du, 0x6F3AF260u, 0xB46F3AF2u, 0x90B46F3Au, 0xEB90B46Fu,
    0x91EB90B4u, 0x1D91EB90u, 0x7F1D91EBu, 0x357F1D91u, 0x61357F1Du, 0x5A61357Fu, 0x325A6135u, 0x03325A61u,
    0x5603325Au, 0xA3560332u, 0xC5A35603u, 0x2BC5A356u, 0x932BC5A3u, 0x80932BC5u, 0x0F80932Bu, 0x4B0F8093u,
    0x434B0F80u, 0xF7434B0Fu, 0xA8F7434Bu, 0xE0A8F743u, 0x3CE0A8F7u, 0x963CE0A8u, 0xD1963CE0u, 0x64D1963Cu,
    0x2664D196u, 0xD72664D1u, 0x45D72664u, 0xCC45D726u, 0x4FCC45D7u, 0xC84FCC45u, 0xB0C84FCCu, 0xE9B0C84Fu,
    0xB5E9B0C8u, 0x00B5E9B0u, 0xD600B5E9u, 0x31D600B5u, 0xEA31D600u, 0x00EA31D6u, 0x0000EA31u, 0x000000EAu,
};

uint crnd_rotl32(uint val, uint shift) {
    return (val << shift) | (val >> (32u - shift));
}

[mutating]
uint CRndSeed_uint32t(inout CRndSeed s) {
    uint byte0 = s.currentValue & 0xFFu;
    uint byte1 = (s.currentValue >> 8) & 0xFFu;
    uint byte2 = (s.currentValue >> 16) & 0xFFu;
    uint byte3 = (s.currentValue >> 24) & 0xFFu;

    int idx4 = int(byte0) - 28;
    if (idx4 < 0) idx4 += 244;

    int idx1 = int(byte3) - 4;
    if (idx1 < 0) idx1 += 188;

    int idx2 = int(byte2) - 12;
    if (idx2 < 0) idx2 += 212;

    int idx3 = int(byte1) - 24;
    if (idx3 < 0) idx3 += 236;

    uint val1 = noiseTableU32[idx1];
    uint val2 = noiseTableU32[idx2];
    uint val3 = noiseTableU32[idx3];
    uint val4 = noiseTableU32[idx4];

    // C++: accumulator + val4 ^ ROTL32(val1,1) ^ ... — '+' binds tighter than '^'
    uint newAccumulator = ((s.accumulator + val4) ^ crnd_rotl32(val1, 1u) ^ crnd_rotl32(val2, 2u) ^ crnd_rotl32(val3, 3u));

    s.currentValue = (uint(idx1) << 24) | (uint(idx2) << 16) | (uint(idx3) << 8) | uint(idx4);
    s.accumulator = newAccumulator;

    return newAccumulator;
}

// [-1, 1]
[mutating]
float CRndSeed_Uniform(inout CRndSeed s) {
    uint u = CRndSeed_uint32t(s);
    uint bits = 0x3f800000u | (0x7fffffu & u);
    float f = asfloat(bits);
    if ((u & 0x80000000u) != 0u) {
        return 2.0f - f;
    }
    return f - 2.0f;
}

// [0, 1)
[mutating]
float CRndSeed_UniformPos(inout CRndSeed s) {
    uint u = CRndSeed_uint32t(s);
    uint bits = 0x3f800000u | (0x7fffffu & u);
    return asfloat(bits) - 1.0f;
}

CRndSeed CRndSeed_create(uint seed) {
    CRndSeed s;
    s.currentValue = seed;
    s.accumulator = 0xFEEFDFAu ^ seed;
    return s;
}

#endif // PARTICLE_SHADER_CRNDSEED_SLANG
