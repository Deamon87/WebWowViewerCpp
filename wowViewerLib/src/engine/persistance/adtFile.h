//
// Created by deamon on 10.07.17.
//

#ifndef WOWVIEWERLIB_ADTFILE_H
#define WOWVIEWERLIB_ADTFILE_H

#include <stdint-gcc.h>
#include "commonFileStructs.h"

struct SMMapHeader {
    enum MHDRFlags {
        mhdr_MFBO = 1,                // contains a MFBO chunk.
        mhdr_northrend = 2,           // is set for some northrend ones.
    };
    uint32_t flags;
    uint32_t mcinOffset;            // Cata+: obviously gone. probably all offsets gone, except mh2o(which remains in root file).
    uint32_t mtexOffset;
    uint32_t mmdxOffset;
    uint32_t mmidOffset;
    uint32_t mwmoOffset;
    uint32_t mwidOffset;
    uint32_t mddfOffset;
    uint32_t modfOffset;
    uint32_t mfboOffset;                     // this is only set if flags & mhdr_MFBO.
    uint32_t mh2oOffset;
    uint32_t mtxfOffset;
    uint8_t mamp_value;             // Cata+, explicit MAMP chunk overrides data
    uint8_t padding[3];
    uint32_t unused[3];
};

struct SMChunkInfo {
    uint32_t offset;              // absolute offset.
    uint32_t size;                // the size of the MCNK chunk, this is refering to.
    uint32_t flags;               // these two are always 0. only set in the client., FLAG_LOADED = 1
    uint32_t asyncId;
};

typedef char* MTEX;
typedef char* MMDX;
typedef char* MWMO;
typedef uint32_t* MMID;
typedef uint32_t* MWID;


struct SMDoodadDef {
    uint32_t mmidEntry;           // references an entry in the MMID chunk, specifying the model to use.
    uint32_t uniqueId;            // this ID should be unique for all ADTs currently loaded. Best, they are unique for the whole map. Blizzard has
                                  //these unique for the whole game.
    C3Vector position;            // This is relative to a corner of the map. Subtract 17066 from the non vertical values and you should start to see
    // something that makes sense. You'll then likely have to negate one of the non vertical values in whatever coordinate
    // system you're using to finally move it into place.
    C3Vector rotation;            // degrees. This is not the same coordinate system orientation like the ADT itself! (see history.)
    uint16_t scale;               // 1024 is the default size equaling 1.0f.

    struct MDDFFlags {
        uint16_t mddf_biodome : 1;             // this sets internal flags to | 0x800 (WDOODADDEF.var0xC).
        uint16_t mddf_shrubbery : 1;           // the actual meaning of these is unknown to me. maybe biodome is for really big M2s. 6.0.1.18179 seems not to check
        // for this flag
        uint16_t mddf_unk_4 : 1;             // Legion+?
        uint16_t mddf_unk_8 : 1;             // Legion+?
        uint16_t mddf_unk_20 = 1;            // Legion+?
        uint16_t mddf_entry_is_filedata_id : 1; // mmidEntry is a file data id to directly load
        uint16_t unused : 10;
    } flags;               // values from enum MDDFFlags.
};

;
struct SMMapObjDef {
    uint32_t nameId;              // references an entry in the MWID chunk, specifying the model to use.
    uint32_t uniqueId;            // this ID should be unique for all ADTs currently loaded. Best, they are unique for the whole map.
    C3Vector position;
    C3Vector rotation;           // same as in MDDF.
    CAaBox extents;              // position plus the transformed wmo bounding box. used for defining if they are rendered as well as collision.
    struct MODFFlags {
        uint16_t modf_destroyable : 1;         // set for destroyable buildings like the tower in DeathknightStart. This makes it a server-controllable game object.
        uint16_t modf_use_lod : 1;             // WoD(?)+: also load _LOD1.WMO for use dependent on distance
        uint16_t modf_unk_4 : 1;               // Legion(?)+: unknown
        uint16_t unused : 13;
    } flags;               // values from enum MODFFlags.
    uint16_t doodadSet;           // which WMO doodad set is used.
    uint16_t nameSet;             // which WMO name set is used. Used for renaming goldshire inn to northshire inn while using the same model.
    uint16_t unk;                 // Legion(?)+: has data finally!
} ;

struct SMChunk
{
    struct
    {
        uint32_t has_mcsh : 1;
        uint32_t impass : 1;
        uint32_t lq_river : 1;
        uint32_t lq_ocean : 1;
        uint32_t lq_magma : 1;
        uint32_t lq_slime : 1;
        uint32_t has_mccv : 1;
        uint32_t unknown_0x80 : 1;
        uint32_t unused1: 7;                                         // not set in 6.2.0.20338
        uint32_t do_not_fix_alpha_map : 1;                    // "fix" alpha maps in MCAL (4 bit alpha maps are 63*63 instead of 64*64).
        // Note that this also means that it *has* to be 4 bit alpha maps, otherwise UnpackAlphaShadowBits will assert.
        uint32_t high_res_holes : 1;                          // Since ~5.3 WoW uses full 64-bit to store holes for each tile if this flag is set.
        uint32_t unused2: 15;                                        // not set in 6.2.0.20338
    } flags;

/*0x004*/  uint32_t IndexX;
/*0x008*/  uint32_t IndexY;
//#if version < ?
//    float radius;
//#endif
/*0x00C*/  uint32_t nLayers;                              // maximum 4
/*0x010*/  uint32_t nDoodadRefs;
//#if version >= ~5.3
//    uint64_t holes_high_res;                                // only used with flags.high_res_holes
//#else
/*0x014*/  uint32_t ofsHeight;
/*0x018*/  uint32_t ofsNormal;
//#endif
/*0x01C*/  uint32_t ofsLayer;
/*0x020*/  uint32_t ofsRefs;
/*0x024*/  uint32_t ofsAlpha;
/*0x028*/  uint32_t sizeAlpha;
/*0x02C*/  uint32_t ofsShadow;                            // only with flags.has_mcsh
/*0x030*/  uint32_t sizeShadow;
/*0x034*/  uint32_t areaid;                              // in alpha: both zone id and sub zone id, as uint16s.
/*0x038*/  uint32_t nMapObjRefs;
/*0x03C*/  uint16_t holes_low_res;
/*0x03E*/  uint16_t unknown_but_used;                    // in alpha: padding
/*0x040*/  uint16_t ReallyLowQualityTextureingMap[8];  // "predTex", It is used to determine which detail doodads to show. Values are an array of two bit unsigned integers, naming the layer.
/*0x050*/  uint64_t noEffectDoodad;                      // WoD: may be an explicit MCDD chunk
/*0x058*/  uint32_t ofsSndEmitters;
/*0x05C*/  uint32_t nSndEmitters;                        // will be set to 0 in the client if ofsSndEmitters doesn't point to MCSE!
/*0x060*/  uint32_t ofsLiquid;
/*0x064*/  uint32_t sizeLiquid;                          // 8 when not used; only read if >8.

// in alpha, remainder is padding but unused.

/*0x068*/  C3Vector position;
/*0x074*/  uint32_t ofsMCCV;                             // only with flags.has_mccv, had uint32_t textureId; in ObscuR's structure.
/*0x078*/  uint32_t ofsMCLV;                             // introduced in Cataclysm
/*0x07C*/  uint32_t unused;                              // currently unused
/*0x080*/
};

struct MCVT {
    float height[9*9 + 8*8];
};

struct MCLV {
    CArgb values[9*9 + 8*8]; // or rgba?
};

struct MCCV {
    struct MCCVEntry {
        uint8_t blue;                 // these values range from 0x00 to 0xFF with 0x7F being the default.
        uint8_t green;                // you can interpret the values as 0x7F being 1.0 and these values being multiplicated with the vertex colors.
        uint8_t red;                  // setting all values to 0x00 makes a chunk completely black.
        uint8_t alpha;                // seems not to have any effect.
    } entries[9*9+8*8];
};

struct SMNormal {
    struct MCNREntry {
        int8_t normal[3];             // normalized. X, Z, Y. 127 == 1.0, -127 == -1.0.
    } entries[9*9+8*8];
    //uint8_t unknown[3*3+2*2];       // this data is not included in the MCNR chunk but additional data which purpose is unknown. 0.5.3.3368 lists this as padding
    // always 0 112 245  18 0  8 0 0  0 84  245 18 0. Nobody yet found a different pattern. The data is not derived from the normals.
    // It also does not seem that the client reads this data. --Schlumpf (talk) 23:01, 26 July 2015 (UTC)
    // While stated that this data is not "included in the MCNR chunk", the chunk-size defined for the MCNR chunk does cover this data. --Kruithne Feb 2016
    // ... from Cataclysm only (on LK files and before, MCNR defined size is 435 and not 448) Mjollna (talk)
};

struct SMLayer
{
    uint32_t textureId;
    struct
    {
        uint32_t animation_rotation : 3;        // each tick is 45°
        uint32_t animation_speed : 3;
        uint32_t animation_enabled : 1;
        uint32_t overbright : 1;                // This will make the texture way brighter. Used for lava to make it "glow".
        uint32_t use_alpha_map : 1;             // set for every layer after the first
        uint32_t alpha_map_compressed : 1;      // see MCAL chunk description
        uint32_t use_cube_map_reflection : 1;   // This makes the layer behave like its a reflection of the skybox. See below
        uint32_t unknown_0x800 : 1;             // WoD?+ if either of 0x800 or 0x1000 is set, texture effects' texture_scale is applied
        uint32_t unknown_0x1000 : 1;            // WoD?+ see 0x800
        uint32_t unused: 19;
    } flags;
    uint32_t offsetInMCAL;
    uint32_t effectId;     // 0xFFFFFFFF for none, in alpha: uint16_t + padding
};



#endif //WOWVIEWERLIB_ADTFILE_H
