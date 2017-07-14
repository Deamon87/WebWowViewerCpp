//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_WMOFILE_H
#define WOWVIEWERLIB_WMOFILE_H
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
/*03Ch*/      uint16_t unknown : 11; // unused as of Legion (20994)
/*03Ch*/  } flags;
/*03Eh*/  uint16_t numLod;                                         // ≥ Legion (21108)  includes base lod (→ numLod = 3 means '.wmo', 'lod0.wmo' and 'lod1.wmo')
};


struct SMOMaterial {
    struct {
        uint32_t F_UNLIT : 1;                    // disable lighting logic in shader (but can still use vertex colors)
        uint32_t F_UNFOGGED : 1;                 // disable fog shading (rarely used)
        uint32_t F_UNCULLED : 1;                 // two-sided
        uint32_t F_EXTLIGHT : 1;                 // darkened, the intern face of windows are flagged 0x08
        uint32_t F_SIDN : 1;                    // (bright at night, unshaded) (used on windows and lamps in Stormwind, for example) (see emissive color)
        uint32_t F_WINDOW : 1;                   // lighting related (flag checked in CMapObj::UpdateSceneMaterials)
        uint32_t F_CLAMP_S : 1;                  // tex clamp S (force this material's textures to use clamp s addressing)
        uint32_t F_CLAMP_T : 1;                  // tex clamp T (force this material's textures to use clamp t addressing)
        uint32_t flag_0x100 : 1;
        uint32_t unused: 23; // unused as of 7.0.1.20994
    } flags;

/*004h*/  uint32_t shader;                 // Index into CMapObj::s_wmoShaderMetaData. See below (shader types).
/*008h*/  uint32_t blendMode;              // Blending: see Blend_State_Table
/*00Ch*/  uint32_t diffuseNameIndex;       // offset into MOTX
/*010h*/  CImVector emissive_color;        // emissive color; see below (emissive color)
/*014h*/  CImVector sidn_emissive_color;   // set at runtime; gets sidn-manipulated emissive color; see below (emissive color)
/*018h*/  uint32_t envNameIndex;
/*01Ch*/  uint32_t diffColor;
/*020h*/  int32_t ground_type;            // according to CMapObjDef::GetGroundType
/*024h*/  uint32_t texture_2;
/*028h*/  uint32_t color_2;
/*02Ch*/  uint32_t flags_2;
/*030h*/  uint32_t runTimeData[4];         // This data is explicitly nulled upon loading. Contains textures or similar stuff.
};

struct SMOGroupInfo
{
/*000h*/  uint32_t flags;      //  see information in in MOGP, they are equivalent
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
struct SMOFog
{
    /*000h*/  uint32_t flag_infinite_radius : 1; // F_IEBLEND: Ignore radius in CWorldView::QueryCameraFog
    /*000h*/  uint32_t unused1: 3;                      // unused as of 7.0.1.20994
    /*000h*/  uint32_t flag_0x10 : 1;
    /*000h*/  uint32_t unused2: 27;                     // unused as of 7.0.1.20994
    /*004h*/  C3Vector pos;
    /*010h*/  float smaller_radius;              // start
    /*014h*/  float larger_radius;               // end
    struct
    {
        /*018h*/    float end;
        /*01Ch*/    float start_scalar;              // (0..1)
        /*020h*/    CImVector color;                // The back buffer is also cleared to this colour
    } fog;
    struct
    {
        /*024h*/    float end;
        /*028h*/    float start_scalar;              // (0..1)
        /*02Ch*/    CImVector color;
    } underwater_fog;
};

struct MOGP {
    uint32_t 		groupName; //(offset into MOGN chunk)
    uint32_t 		descriptiveGroupName;// (offset into MOGN chunk)
    uint32_t 		flags;
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
    uint32_t 		unused1;//&1: WoD(?)+ CanCutTerrain (by MOPL planes), others (UNUSED: 20740)
    uint32_t 		unused2;//(UNUSED: 20740)
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

#endif //WOWVIEWERLIB_WMOFILE_H
