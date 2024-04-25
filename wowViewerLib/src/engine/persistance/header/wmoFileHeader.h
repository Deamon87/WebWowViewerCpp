//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_WMOFILEHEADER_H
#define WOWVIEWERLIB_WMOFILEHEADER_H
#include "commonFileStructs.h"

struct SMOHeader
{
/*000h*/  uint32_t nTextures;
/*004h*/  uint32_t nGroups;
/*008h*/  uint32_t nPortals;
/*00Ch*/  uint32_t nLights;
/*010h*/  uint32_t nDoodadNames;
/*014h*/  uint32_t nDoodadDefs;                                    // *
/*018h*/  uint32_t nDoodadSets;
/*01Ch*/  CArgb ambColor;                        // Color settings for base (ambient) color. See the flag at /*03Ch*/.
/*020h*/  int32_t wmoID;
/*024h*/  CAaBox bounding_box;
/*03Ch*/
          struct {
/*03Ch*/      uint16_t flag_attenuate_vertices_based_on_distance_to_portal : 1;
/*03Ch*/      uint16_t flag_skip_base_color : 1;                       // do not add base (ambient) color (of MOHD) to MOCVs. apparently does more, e.g. required for multiple MOCVs
/*03Ch*/      uint16_t flag_use_liquid_type_dbc_id : 1;                // use real liquid type ID from DBCs instead of local one. See MLIQ for further reference.
/*03Ch*/      uint16_t flag_lighten_interiors : 1;                     // makes iterior groups much brighter, effects MOCV rendering. Used e.g.in Stormwind for having shiny bright interiors,
/*03Ch*/      uint16_t Flag_Lod : 1;                                   // ≥ Legion (20740)
/*03Ch*/      uint16_t maybeMohdColor : 1; // unused as of Legion (20994)
/*03Ch*/      uint16_t unknown : 10; // unused as of Legion (20994)
/*03Ch*/  } flags;
/*03Eh*/  uint16_t numLod;                                         // ≥ Legion (21108)  includes base lod (→ numLod = 3 means '.wmo', 'lod0.wmo' and 'lod1.wmo')
};


struct SMOMaterial {
    struct {
        uint32_t F_UNLIT : 1;                    // disable lighting logic in shader (but can still use vertex colors)
        uint32_t F_UNFOGGED : 1;                 // disable fog shading (rarely used)
        uint32_t F_UNCULLED : 1;                 // two-sided
        uint32_t F_EXTLIGHT : 1;                 // darkened, the intern face of windows are flagged 0x08
        uint32_t F_SIDN : 1;                     // (bright at night, unshaded) (used on windows and lamps in Stormwind, for example) (see emissive color)
        uint32_t F_WINDOW : 1;                   // lighting related (flag checked in CMapObj::UpdateSceneMaterials)
        uint32_t F_CLAMP_S : 1;                  // tex clamp S (force this material's textures to use clamp s addressing)
        uint32_t F_CLAMP_T : 1;                  // tex clamp T (force this material's textures to use clamp t addressing)
        uint32_t flag_0x100 : 1;
        uint32_t unused: 22; // unused as of 7.0.1.20994
    } flags;

/*004h*/  uint32_t shader;                 // Index into CMapObj::s_wmoShaderMetaData. See below (shader types).
/*008h*/  uint32_t blendMode;              // Blending: see Blend_State_Table
/*00Ch*/  uint32_t diffuseNameIndex;       // offset into MOTX
/*010h*/  CImVector emissive_color;        // emissive color; see below (emissive color)
/*014h*/  CImVector sidn_emissive_color;   // set at runtime; gets sidn-manipulated emissive color; see below (emissive color)
/*018h*/  uint32_t envNameIndex;
/*01Ch*/  CArgb diffColor;
/*020h*/  int32_t ground_type;            // according to CMapObjDef::GetGroundType
/*024h*/  uint32_t texture_2;
/*028h*/  uint32_t color_2;
/*02Ch*/  uint32_t flags_2;
/*030h*/  uint32_t runTimeData[4];         // This data is explicitly nulled upon loading. Contains textures or similar stuff.
};

struct SMOGroupFlags {
    // 0x1;
    uint32_t hasBsp : 1;
    // 0x2;
    uint32_t hasLightMap : 1; //old: has MOLM, MOLD// new: subtract mohd.color in mocv fixing
    // 0x4;
    uint32_t hasVertexColors : 1; // MOCV chunk
    // 0x8;
    uint32_t EXTERIOR : 1; // Outdoor
    // 0x10;
    uint32_t unused1 : 1;
    // 0x20;
    uint32_t unused2 : 1;
    // 0x40;
    uint32_t EXTERIOR_LIT : 1;
    // 0x80;
    uint32_t UNREACHABLE : 1;
    // 0x100;
    uint32_t SHOW_EXTERIOR_SKYBOX : 1;
    // 0x200;
    uint32_t hasLights : 1; //has MOLR chunk
    // 0x400;
    uint32_t LOD : 1; // Also load for LoD != 0 (_lod* groups)
    // 0x800
    uint32_t hasDoodads : 1; //has MODR chunk
    // 0x1000
    uint32_t LIQUIDSURFACE : 1; // Has water (MLIQ chunk)
    // 0x2000
    uint32_t INTERIOR : 1; // Indoor
    // 0x4000
    uint32_t unused4 : 1;
    // 0x8000
    uint32_t unused5 : 1;
    // 0x10000
    uint32_t ALWAYSDRAW : 1; //clear 0x8 after CMapObjGroup::Create() in MOGP and MOGI
    // 0x20000
    uint32_t hasMORI : 1; // Has MORI and MORB chunks.
    // 0x40000
    uint32_t showSkyBox : 1; //automatically unset if MOSB not present.
    // 0x80000
    uint32_t is_not_water_but_ocean : 1; //LiquidType related, see below in the MLIQ chunk.
    // 0x100000
    uint32_t unused6 : 1; //
    // 0x200000
    uint32_t isMountAllowed : 1;
    // 0x400000
    uint32_t unused7 : 1;
    // 0x800000
    uint32_t unused8 : 1;
    // 0x1000000
    uint32_t CVERTS2 : 1; //Has two MOCV chunks
    // 0x2000000
    uint32_t TVERTS2 : 1; //Has two MOTV chunks
    // 0x4000000
    uint32_t ANTIPORTAL : 1; //requires intBatchCount == 0, extBatchCount == 0, UNREACHABLE.
    // 0x8000000
    uint32_t unk1 : 1; //requires intBatchCount == 0, extBatchCount == 0, UNREACHABLE.
    // 0x10000000
    uint32_t unused9 : 1;
    // 0x20000000
    uint32_t EXTERIOR_CULL : 1;
    // 0x40000000
    uint32_t TVERTS3 : 1; //Has three MOTV chunks
    // 0x80000000
    uint32_t unused10 : 1;
};

    struct SMOGroupFlags2 {
        //0x1
        uint32_t unk1 : 1;
        //0x2
        uint32_t unk2 : 1;
        //0x4
        uint32_t unk4 : 1;
        //0x8
        uint32_t unk8 : 1;
        //0x10
        uint32_t unk0x10 : 1;
        //0x20
        uint32_t unk0x20 : 1;
        //0x40
        uint32_t isSplitGroupParent : 1;
        //0x80
        uint32_t isSplitGroupChild : 1;
    };

struct SMOGroupInfo
{
/*000h*/  SMOGroupFlags flags; //  see information in in MOGP, they are equivalent
/*004h*/  CAaBox bounding_box;
/*01Ch*/  int32_t nameoffset;  // name in MOGN chunk (-1 for no name)
};

struct SMOPortal
{
    uint16_t base_index;
    uint16_t index_count;
    C4Plane  plane;
};
struct SMOPortalRef // 04-29-2005 By ObscuR
{
    uint16_t portal_index;  // into MOPR
    uint16_t group_index;   // the other one
    int16_t side;           // positive or negative.
    uint16_t unk;
};
struct SMOLight
{
    enum LightType
    {
        OMNI_LGT = 0,
        SPOT_LGT = 1,
        DIRECT_LGT = 2,
        AMBIENT_LGT = 3,
    };
    /*000h*/  uint8_t type;
    /*001h*/  uint8_t useAtten;
    /*002h*/  uint8_t pad[2];
    /*004h*/  CImVector color;
    /*008h*/  C3Vector position;
    /*014h*/  float intensity;
    /*018h*/  float attenStart;
    /*01Ch*/  float attenEnd;
    /*020h*/  float unk[4];
};
struct SMODoodadSet
{
/*000h*/  char     name[20];            // set name
/*014h*/  uint32_t firstinstanceindex;  // index of first doodad instance in this set
/*018h*/  uint32_t numDoodads;          // number of doodad instances in this set
/*01Ch*/  uint32_t unused;
};

struct SMODoodadDef
{
    /*000h*/  uint32_t name_offset : 24;        // reference offset into MODN
    /*003h*/  uint32_t flag_AcceptProjTex : 1;
    /*003h*/  uint32_t flag_0x2 : 1;            // MapStaticEntity::field_34 |= 1 (if set, MapStaticEntity::AdjustLighting is _not_ called)
    /*003h*/  uint32_t flag_0x4 : 1;
    /*003h*/  uint32_t flag_0x8 : 1;
    /*003h*/  uint32_t unused: 4;               // unused as of 7.0.1.20994
    /*004h*/  C3Vector position;                // (X,Z,-Y)
    /*010h*/  C4Quaternion orientation;         // (X, Y, Z, W)
    /*020h*/  float scale;                      // scale factor
    /*024h*/  CImVector color;                 // (B,G,R,A) diffuse lighting color, used in place of global diffuse from DBCs
};

struct SMOFog_Data {
    /*018h*/    float end;
    /*01Ch*/    float start_scalar;              // (0..1)
    /*020h*/    CImVector color;
};
struct SMOFog
{
    /*000h*/  uint32_t flag_infinite_radius : 1; // F_IEBLEND: Ignore radius in CWorldView::QueryCameraFog
    /*000h*/  uint32_t unused1: 3;                      // unused as of 7.0.1.20994
    /*000h*/  uint32_t flag_0x10 : 1;
    /*000h*/  uint32_t unused2: 27;                     // unused as of 7.0.1.20994
    /*004h*/  C3Vector pos;
    /*010h*/  float smaller_radius;              // start
    /*014h*/  float larger_radius;               // end
    SMOFog_Data fog;
    SMOFog_Data underwater_fog;
};
static_assert(sizeof(SMOFog) == 0x30);


struct MOGP {
    uint32_t 		groupName; //(offset into MOGN chunk)
    uint32_t 		descriptiveGroupName;// (offset into MOGN chunk)
    SMOGroupFlags   flags;
    CAaBox          boundingBox;
    uint16_t 		moprIndex;  //Index into the MOPR chunk
    uint16_t 		moprCount;  //Number of items used from the MOPR chunk
    uint16_t 		transBatchCount;
    uint16_t 		intBatchCount;
    uint16_t 		extBatchCount;
    uint16_t 		unk;        //(padding? batch type D?)
    uint8_t         fogIndicies[4];// 	Up to four indices into the WMO fog list
    uint32_t 		liquidType;    //LiquidType, not always directly used: see below in the MLIQ chunk.
    uint32_t        wmoGroupID;
    SMOGroupFlags2 	flags2;//&1: WoD(?)+ CanCutTerrain (by MOPL planes), others (UNUSED: 20740)
    int16_t 		parentSplitOrFirstChildGroupIndex;
    int16_t 		nextSplitGroupChildIndex;
};

struct MAVG {
    /*000h*/ C3Vector pos;
    /*00Ch*/ float start;
    /*010h*/ float end;
    /*014h*/ CImVector color1;
    /*018h*/ CImVector color2;
    /*01Ch*/ CImVector color3;
    /*020h*/ uint32_t flags;    // &1: use color1 and color3
    /*024h*/ uint16_t doodadSetID;
    /*026h*/ char _0x26[10];
};

struct SMOPoly
{
    struct
    {
        uint8_t F_UNK_0x01: 1;
        uint8_t F_NOCAMCOLLIDE : 1;
        uint8_t F_DETAIL : 1;
        uint8_t F_COLLISION : 1; // Turns off rendering of water ripple effects. May also do more. Should be used for ghost material triangles.
        uint8_t F_HINT : 1;
        uint8_t F_RENDER : 1;
        uint8_t F_UNK_0x40 : 1;
        uint8_t F_COLLIDE_HIT : 1;

        bool isTransFace() { return this->F_UNK_0x01 && (this->F_DETAIL || this->F_RENDER); }
        bool isColor() { return !this->F_COLLISION; }
        bool isRenderFace() { return this->F_RENDER && !this->F_DETAIL; }
        bool isCollidable() { return this->F_COLLISION || this->isRenderFace(); }
    } flags;

    uint8_t material_id;           // index into MOMT, 0xff for collision faces
};

struct SMOBatch
{
    union {
        struct {
            /*0x00*/ int16_t unknown_box_min[3];              // -2,-2,-1, 2,2,3 in cameron -> seems to be a bounding box for culling
            /*0x06*/ int16_t unknown_box_max[3];
        } preLegion;
        struct {
            /*0x00*/ uint8_t unknown[0xA];
            /*0x0A*/ uint16_t material_id_large;              // used if flag_use_uint16_t_material is set.
        } postLegion;
    };

    /*0x0C*/ uint32_t first_index;                    // index of the first face index used in MOVI
    /*0x10*/ uint16_t num_indices;                    // number of MOVI indices used
    /*0x12*/ uint16_t first_vertex;                   // index of the first vertex used in MOVT
    /*0x14*/ uint16_t last_vertex;                    // index of the last vertex used (batch includes this one)
    /*0x16*/ uint8_t flag_unknown_1 : 1;
//#if ≥ Legion
    /*0x16*/ uint8_t flag_use_material_id_large : 1;  // instead of material_id use material_id_large
    /*0x16*/ uint8_t flag_unknown_3 : 1;
    /*0x16*/ uint8_t flag_unknown_4 : 1;
    /*0x16*/ uint8_t flag_unknown_5 : 1;
    /*0x16*/ uint8_t flag_unknown_6 : 1;
    /*0x16*/ uint8_t flag_unknown_7 : 1;
    /*0x16*/ uint8_t flag_unknown_8 : 1;
//#endif
//#if version >= ?
    /*0x17*/ uint8_t material_id;                     // index in MOMT
//#else
};


struct t_BSP_NODE
{
    uint16_t planeType;    // 4: leaf, 0 for YZ-plane, 1 for XZ-plane, 2 for XY-plane
    int16_t  children[2];  // index of bsp child node (right in this array)
    uint16_t numFaces;     // num of triangle faces in MOBR
    uint32_t firstFace;    // index of the first triangle index(in MOBR)
    float    fDist;
};

#pragma pack(push, 1)
struct MLIQ {
    uint32_t xverts;
    uint32_t yverts;
    uint32_t xtiles;
    uint32_t ytiles;
    C3Vector basePos;
    uint16_t materialId;
};
#pragma pack(pop)

struct SMOLVert
{
    union
    {
        struct {
            uint8_t flow1;
            uint8_t flow2;
            uint8_t flow1Pct;
            uint8_t filler;
            float height;
        }  waterVert;
        struct
        {
            int16_t s;
            int16_t t;
            float height;
        } magmaVert;
    };
};
struct SMOLTile
{
    uint8_t legacyLiquidType : 4;
    uint8_t flag_16 : 1;
    uint8_t flag_32 : 1;
    uint8_t flag_64 : 1;
    uint8_t flag_128 : 1;
    ;
};

//Light related structs

struct Light_texture_animation
{
    float flickerIntensity;
    float flickerSpeed;
    int flickerMode;
};

struct LightUnkRecord
{
    int unk0;
    int unk1;
    int unk2;
    int unk3;
    int lightTextureFileDataId;
    int unk5;
    int unk6;
    int unk7;
    int unk8;
    int unk9;
};


struct LightRecPerSet {
    uint32_t offset;
    uint32_t count;
};

//MOLP
PACK(
struct map_object_point_light {
    uint32_t lightId;
    CImVector color;
    C3Vector pos;
    float attenuationStart;
    float attenuationEnd;
    float intensity;
    C3Vector rotation;
});

//MOP2
PACK(
struct map_object_pointlight_anim
{
    map_object_point_light pointLight;
    Light_texture_animation lightTextureAnimation;
    LightUnkRecord lightUnkRecord;
});

//MOLS
PACK(
struct map_object_spot_light {
    uint32_t lightIndex;
    uint32_t color;
    C3Vector pos;
    uint32_t attenuationStart;
    float attenuationEnd;
    uint32_t intesity;
    C3Vector rotation;
    uint32_t spotlightRadius; //A
    float innerAngle;
    float outerAngle;
});

//MOS2
PACK(
struct map_object_spotlight_anim
{
    map_object_spot_light spotLight;
    Light_texture_animation lightTextureAnimation;
    LightUnkRecord lightUnkRecord;
});

//MLND - lives in main WMO
struct mapobject_new_light_def {
    int type;
    int lightIndex;
    int flags;
    int doodadSet;
    CImVector innerColor;
    C3Vector position;
    C3Vector rotation;
    float attenStart;
    float attenEnd;
    float intensity;
    CImVector outerColor;
    float falloffStart;
    float falloff;
    int field_44;
    Light_texture_animation lightTextureAnimation;
    LightUnkRecord lightUnkRecord;
    float spotlightRadius;
    float innerAngle;
    float outerAngle;
    uint16_t packedVal1;
    uint16_t field_8A;
    int field_8C;
    int field_90;
    int field_94;
    int field_98;
    int field_9C;
    int field_A0;
    int field_A4;
    int field_A8;
    int field_AC;
    int field_B0;
    int field_B4;
};

static_assert(sizeof(mapobject_new_light_def) == 0xb8);


#endif //WOWVIEWERLIB_WMOFILEHEADER_H
