//
// Created by deamon on 16.06.17.
//

#ifndef WOWVIEWERLIB_M2FILEHEADER_H
#define WOWVIEWERLIB_M2FILEHEADER_H

#include "commonFileStructs.h"

struct M2Loop {
    uint32_t timestamp;
};


struct M2CompBone                 // probably M2Bone  ≤ Vanilla
{
    int32_t key_bone_id;            // Back-reference to the key bone lookup table. -1 if this is no key bone.
    union {
        uint32_t flags_raw;
        struct {
            uint32_t unk_0x1 : 1;//0x1
            uint32_t unk_0x2 : 1;//0x2
            uint32_t unk_0x4 : 1;//0x4
            uint32_t spherical_billboard : 1;// = 0x8,
            uint32_t cylindrical_billboard_lock_x : 1;// = 0x10,
            uint32_t cylindrical_billboard_lock_y : 1; // 0x20,
            uint32_t cylindrical_billboard_lock_z : 1; // 0x40,
            uint32_t unk_0x80 : 1; //0x80
            uint32_t unk_0x100 : 1; //0x100
            uint32_t transformed : 1;//= 0x200,

            uint32_t kinematic_bone: 1;// = 0x400,       // MoP+: allow physics to influence this bone
            uint32_t unk_0x800 : 1; //0x800
            uint32_t helmet_anim_scaled
                : 1; //= 0x1000,  // set blend_modificator to helmetAnimScalingRec.m_amount for this bone
        } flags;
    };
    int16_t parent_bone;            // Parent bone ID or -1 if there is none.
    uint16_t submesh_id;            // Mesh part ID OR uDistToParent?
    union {                         // only ≥ BC ?
        struct {
            uint16_t uDistToFurthDesc;
            uint16_t uZRatioOfChain;
        } CompressData;
        uint32_t boneNameCRC;
    };
    M2Track<C3Vector> translation;
    M2Track<M2CompQuat> rotation;   // compressed values, default is (32767,32767,32767,65535) == (0,0,0,1) == identity
    M2Track<C3Vector> scaling;
    C3Vector pivot;                 // The pivot point of that bone.
};

const int MAX_BONES_PER_VERTEX = 4;

struct M2Vertex
{
    C3Vector pos;
    uint8_t bone_weights[MAX_BONES_PER_VERTEX];
    uint8_t bone_indices[MAX_BONES_PER_VERTEX];
    C3Vector normal;
    C2Vector tex_coords[2];  // two textures, depending on shader used
};
struct M2Color
{
    M2Track<C3Vector> color; // vertex colors in rgb order
    M2Track<fixed16> alpha; // 0 - transparent, 0x7FFF - opaque. Normaly NonInterp
};
struct M2TextureWeight
{
    M2Track<fixed16> weight;
};
struct M2Texture
{
    uint32_t type;          // see below
    uint32_t flags;         // see below
    M2Array<char> filename; // for non-hardcoded textures (type != 0), this still points to a zero-sized string
};
struct M2TextureTransform
{
    M2Track<C3Vector> translation;
    M2Track<C4Quaternion> rotation;    // rotation center is texture center (0.5, 0.5)
    M2Track<C3Vector> scaling;
};
struct M2Material
{
    uint16_t flags;
    uint16_t blending_mode; // apparently a bitfield
};
struct M2Attachment
{
    uint32_t id;                      // Referenced in the lookup-block below.
    uint16_t bone;                    // attachment base
    uint16_t unknown;                 // see BogBeast.m2 in vanilla for a model having values here
    C3Vector position;               // relative to bone; Often this value is the same as bone's pivot point
    M2Track<unsigned char> animate_attached;  // whether or not the attached model is animated when this model is. only a bool is used. default is true.
};
struct M2Event
{
    uint32_t identifier;  // mostly a 3 character name prefixed with '$'.
    uint32_t data;        // This data is passed when the event is fired.
    uint32_t bone;        // Somewhere it has to be attached.
    C3Vector position;   // Relative to that bone of course, animated. Pivot without animating.
    M2TrackBase enabled;  // This is a timestamp-only animation block. It is built up the same as a normal AnimationBlocks, but is missing values, as every timestamp is an implicit "fire now".
};
struct M2Light
{
    uint16_t type;                      // Types are listed below.
    int16_t bone;                       // -1 if not attached to a bone
    C3Vector position;                 // relative to bone, if given
    M2Track<C3Vector> ambient_color;
    M2Track<float> ambient_intensity;   // defaults to 1.0
    M2Track<C3Vector> diffuse_color;
    M2Track<float> diffuse_intensity;   // defaults to 1.0
    M2Track<float> attenuation_start;
    M2Track<float> attenuation_end;
    M2Track<unsigned char> visibility;  // enabled?
};

struct M2Camera
{
    int32_t type; // 0: portrait, 1: characterinfo; -1: else (flyby etc.); referenced backwards in the lookup table.
//#if  < Cata
//    float fov; // rad_fov = fov * 0.6 -- No radians, no degrees. Multiply by 35 to get degrees. -- the default fov is 0.94247788rad
//#endif
    float far_clip;
    float near_clip;
    M2Track<M2SplineKey<C3Vector>> positions; // How the camera's position moves. Should be 3*3 floats.
    C3Vector position_base;
    M2Track<M2SplineKey<C3Vector>> target_position; // How the target moves. Should be 3*3 floats.
    C3Vector target_position_base;
    M2Track<M2SplineKey<float>> roll; // The camera can have some roll-effect. Its 0 to 2*Pi.
//#if ≥ Cata
    M2Track<M2SplineKey<float>> FoV; // Units are not radians. Multiplying by 35 seems to be the correct # of degrees. This is incredibly important, as otherwise e.g. loading screen models will look totally wrong.
//#endif
};

struct M2Ribbon
{
    uint32_t ribbonId;                  // Always (as I have seen): -1.
    uint32_t boneIndex;                 // A bone to attach to.
    C3Vector position;                  // And a position, relative to that bone.
    M2Array<uint16_t> textureIndices;   // into textures
    M2Array<uint16_t> materialIndices;  // into materials
    M2Track<C3Vector> colorTrack;
    M2Track<fixed16> alphaTrack;        // And an alpha value in a short, where: 0 - transparent, 0x7FFF - opaque.
    M2Track<float> heightAboveTrack;
    M2Track<float> heightBelowTrack;    // do not set to same!
    float edgesPerSecond;               // this defines how smooth the ribbon is. A low value may produce a lot of edges.
    float edgeLifetime;                 // the length aka Lifespan. in seconds
    float gravity;                      // use arcsin(val) to get the emission angle in degree
    uint16_t textureRows;               // tiles in texture
    uint16_t textureCols;
    M2Track<uint16_t> texSlotTrack;
    M2Track<unsigned char> visibilityTrack;

    int16_t priorityPlane;
    uint16_t padding;

};


struct CompressedParticleGravity {
    int8_t x, y;
    int16_t z;
};

struct M2ParticleOld {
    uint32_t particleId;                        // Always (as I have seen): -1.
    union {
        uint32_t flags;                             // See Below
        struct {
            uint8_t hex_1 : 1;
            uint8_t hex_2 : 1;
            uint8_t hex_4 : 1;
            uint8_t hex_8 : 1;
            uint8_t hex_10 : 1;
            uint8_t hex_20 : 1;
            uint8_t hex_40 : 1;
            uint8_t hex_80 : 1;
            uint8_t hex_100 : 1;
            uint8_t hex_200 : 1;
            uint8_t hex_400 : 1;
            uint8_t hex_800 : 1;
            uint8_t hex_1000 : 1;
            uint8_t hex_2000 : 1;
            uint8_t hex_4000 : 1;
            uint8_t hex_8000 : 1;
            uint8_t hex_10000 : 1;
            uint8_t hex_20000 : 1;
            uint8_t hex_40000 : 1;
            uint8_t hex_80000 : 1;
            uint8_t hex_100000 : 1;
            uint8_t hex_200000 : 1;
            uint8_t hex_400000 : 1;
            uint8_t hex_800000 : 1;
            uint8_t hex_1000000 : 1;
            uint8_t hex_2000000 : 1;
            uint8_t hex_4000000 : 1;
            uint8_t hex_8000000 : 1;
            uint8_t hex_10000000 : 1;
            uint8_t hex_20000000 : 1;
            uint8_t hex_40000000 : 1;
            uint8_t hex_80000000 : 1;
        } flags_per_number;
    };
    C3Vector Position;                       // The position. Relative to the following bone.
    uint16_t bone;                              // The bone its attached to.
    PACK(union
    {
        uint16_t texture;                         // And the textures that are used.
//#if ≥ Cata
    PACK(struct  // For multi-textured particles actually three ids
    {
      uint16_t texture_0 : 5;
      uint16_t texture_1 : 5;
      uint16_t texture_2 : 5;
      uint16_t pad : 1;
    });
//#endif
    });
    M2Array<char> geometry_model_filename;    // if given, this emitter spawns models
    M2Array<char> recursion_model_filename;   // if given, this emitter is an alias for the (maximum 4) emitters of the given model

//#if >= 263 (late Burning Crusade)
    uint8_t blendingType;                       // A blending type for the particle. See Below
    uint8_t emitterType;                        // 1 - Plane (rectangle), 2 - Sphere, 3 - Spline, 4 - Bone
    uint16_t particleColorIndex;                // This one is used for ParticleColor.dbc. See below.
//#else
//    uint16_t blendingType;                      // A blending type for the particle. See Below
//    uint16_t emitterType;                       // 1 - Plane (rectangle), 2 - Sphere, 3 - Spline, 4 - Bone
//#endif
//#if ≥ Cata
    uint8_t multiTextureParamX[2];
//#else
//    uint8_t particleType;                      // Found below.
//    uint8_t headorTail;                         // 0 - Head, 1 - Tail, 2 - Both
//#endif
    uint16_t textureTileRotation;               // Rotation for the texture tile. (Values: -1,0,1) -- priorityPlane
    uint16_t textureDimensions_rows;            // for tiled textures
    uint16_t textureDimensions_columns;
    M2Track<float> emissionSpeed;             // Base velocity at which particles are emitted.
    M2Track<float> speedVariation;            // Random variation in particle emission speed. (range: 0 to 1)
    M2Track<float> verticalRange;             // Drifting away vertically. (range: 0 to pi) For plane generators, this is the maximum polar angle of the initial velocity;
    // 0 makes the velocity straight up (+z). For sphere generators, this is the maximum elevation of the initial position;
    // 0 makes the initial position entirely in the x-y plane (z=0).
    M2Track<float> horizontalRange;           // They can do it horizontally too! (range: 0 to 2*pi) For plane generators, this is the maximum azimuth angle of the initial velocity;
    // 0 makes the velocity have no sideways (y-axis) component.
    // For sphere generators, this is the maximum azimuth angle of the initial position.
    union {
        M2Track<float> gravity;                   // Not necessarily a float; see below.
        M2Track<CompressedParticleGravity> gravityCompr;
    };
    M2Track<float> lifespan;
//#if ≥ Wrath
    float lifespanVary;                       // An individual particle's lifespan is added to by lifespanVary * random(-1, 1)
//#endif
    M2Track<float> emissionRate;
//#if ≥ Wrath
    float emissionRateVary;                   // This adds to the base emissionRate value the same way as lifespanVary. The random value is different every update.
//#endif
    M2Track<float> emissionAreaLength;        // For plane generators, this is the width of the plane in the x-axis.
    // For sphere generators, this is the minimum radius.
    M2Track<float> emissionAreaWidth;         // For plane generators, this is the width of the plane in the y-axis.
    // For sphere generators, this is the maximum radius.
    M2Track<float> zSource;                   // When greater than 0, the initial velocity of the particle is (particle.position - C3Vector(0, 0, zSource)).Normalize()
//#if ≥ Wrath
    M2PartTrack<C3Vector> colorTrack;             // Most likely they all have 3 timestamps for {start, middle, end}.
    M2PartTrack<fixed16> alphaTrack;
    M2PartTrack<C2Vector> scaleTrack;
    C2Vector scaleVary;                      // A percentage amount to randomly vary the scale of each particle
    M2PartTrack<uint16_t> headCellTrack;             // Some kind of intensity values seen: 0,16,17,32 (if set to different it will have high intensity)
    M2PartTrack<uint16_t> tailCellTrack;
//#else
//    float midPoint; // Middle point in lifespan (0 to 1).
//    byte[4][3] colorValues; // 3*BGRA
//    float[3] scaleValues;
//    uint16[2] headCellBegin;
//    uint16 between1; // Always 1
//    uint16[2] headCellEnd;
//    uint16 between1; // Always 1
//    short[4] tiles; // Indices into the tiles on the texture ? Or tailCell maybe ?
//#endif
    float tailLength;                         // TailCellTime?
    float TwinkleSpeed;                       // has something to do with the spread
    float TwinklePercent;                     // has something to do with the spread
    CRange twinkleScale;
    float BurstMultiplier;                    // ivelScale
    float drag;                               // For a non-zero values, instead of travelling linearly the particles seem to slow down sooner. Speed is multiplied by exp( -drag * t ).
//#if ≥ Wrath
    float baseSpin;                           // Initial rotation of the particle quad
    float baseSpinVary;
    float Spin;                               // Rotation of the particle quad per second
    float spinVary;
//#else
//    float spin;                               // 0.0 for none, 1.0 to rotate the particle 360 degrees throughout its lifetime.
//#endif

    M2Box tumble;
    C3Vector WindVector;
    float WindTime;

    float followSpeed1;
    float followScale1;
    float followSpeed2;
    float followScale2;
    M2Array<C3Vector> splinePoints;                   //Set only for spline praticle emitter. Contains array of points for spline
    M2Track<unsigned char> enabledIn;                 // (boolean) Appears to be used sparely now, probably there's a flag that links particles to animation sets where they are enabled.
};



struct M2Particle
{
    M2ParticleOld old;
    vector_2fp_6_9 multiTextureParam0[2];
    vector_2fp_6_9 multiTextureParam1[2];
};

struct M2Data {
    uint32_t magic;                                       // "MD20". Legion uses a chunked file format starting with MD21.
    uint32_t version;
    M2Array<char> name;                                   // should be globally unique, used to reload by name in internal clients

    struct
    {
        uint32_t flag_tilt_x : 1;
        uint32_t flag_tilt_y : 1;
        uint32_t : 1;
        uint32_t flag_has_blend_maps : 1;                   // add BlendMaps fields in header
        uint32_t flag_sort_on_bones: 1;
        uint32_t flag_load_phys_data : 1;
        uint32_t : 1;
        uint32_t flag_unk_0x80 : 1;                         // with this flag unset, demon hunter tattoos stop glowing
        uint32_t flag_camera_related : 1;                   // TODO: verify version
        uint32_t flag_new_particle_record : 1;              // In CATA: new version of ParticleEmitters. By default, length of M2ParticleOld is 476.
        uint32_t flag_unk_0x400 : 1;
        uint32_t flag_texture_transforms_use_bone_sequences  : 1;
        uint32_t flag_unk_0x1000 : 1;
        uint32_t flag_unk_0x2000 : 1;                       // seen in UI_MainMenu_Legion
        uint32_t flag_unk_0x4000 : 1;
        uint32_t flag_unk_0x8000 : 1;                       // seen in UI_MainMenu_Legion
    } global_flags;

    M2Array<M2Loop> global_loops;                        // Timestamps used in global looping animations.
    M2Array<M2Sequence> sequences;                       // Information about the animations in the model.
    M2Array<int16_t> sequence_lookups;                  // Mapping of sequence IDs to the entries in the Animation sequences block.
    M2Array<M2CompBone> bones;                           // MAX_BONES = 0x100
    M2Array<int16_t> key_bone_lookup;                   // Lookup table for key skeletal bones.
    M2Array<M2Vertex> vertices;
    uint32_t num_skin_profiles;                           // Views (LOD) are now in .skins.
    M2Array<M2Color> colors;                             // Color and alpha animations definitions.
    M2Array<M2Texture> textures;
    M2Array<M2TextureWeight> texture_weights;            // Transparency of textures.
    M2Array<M2TextureTransform> texture_transforms;
    M2Array<int16_t> replacable_texture_lookup;
    M2Array<M2Material> materials;                       // Blending modes / render flags.
    M2Array<int16_t> bone_lookup_table;
    M2Array<int16_t> texture_lookup_table;
    M2Array<int16_t> tex_unit_lookup_table;             // ≥ Cata : unused
    M2Array<int16_t> transparency_lookup_table;
    M2Array<int16_t> texture_transforms_lookup_table;

    CAaBox bounding_box;                                 // min/max( [1].z, 2.0277779f ) - 0.16f seems to be the maximum camera height
    float bounding_sphere_radius;                         // detail doodad draw dist = clamp (bounding_sphere_radius * detailDoodadDensityFade * detailDoodadDist, …)
    CAaBox collision_box;
    float collision_sphere_radius;

    M2Array<uint16_t> collision_triangles;
    M2Array<C3Vector> collision_vertices;
    M2Array<C3Vector> collision_normals;
    M2Array<M2Attachment> attachments;                   // position of equipped weapons or effects
    M2Array<uint16_t> attachment_lookup_table;
    M2Array<M2Event> events;                             // Used for playing sounds when dying and a lot else.
    M2Array<M2Light> lights;                             // Lights are mainly used in loginscreens but in wands and some doodads too.
    M2Array<M2Camera> cameras;                           // The cameras are present in most models for having a model in the character tab.
    M2Array<uint16_t> camera_lookup_table;
    M2Array<M2Ribbon> ribbon_emitters;                   // Things swirling around. See the CoT-entrance for light-trails.
    M2Array<M2Particle> particle_emitters;
    M2Array<uint16_t> blend_map_overrides;             // When set, textures blending is overriden by the associated array.
};

struct M2LightResult {
public:
    mathfu::vec4 ambient_color;
    float ambient_intensity;
    mathfu::vec4 diffuse_color;
    float diffuse_intensity;
    float attenuation_start;
    float attenuation_end;
    mathfu::vec4 position;
    unsigned char visibility;
};
struct M2CameraResult {
    float far_clip;
    float near_clip;
    mathfu::vec4 position;
    mathfu::vec4 target_position;
    float roll; // The camera can have some roll-effect. Its 0 to 2*Pi.
    float diagFov;
};

struct M2_AFID {
    uint16_t anim_id;
    uint16_t sub_anim_id;
    uint32_t file_id;
};

struct TXAC {
    union {
        uint16_t value;
        uint8_t perByte[2];
    };
};

struct Exp2Record
{
    float zSource;
    uint32_t unk1;
    uint32_t unk2;
    M2PartTrack<fixed16> unk3;
};

struct EXP2
{
    M2Array<Exp2Record> content;
};

PACK(
struct WaterFallDataV3 {
    float bumpScale;
    float values0_x;
    float values0_y;
    float values0_z;
    float value1_w;
    float values0_w;
    float value1_x;
    float value1_y;
    float value2_w;
    float value3_y;
    float value3_x;
    CImVector basecolor;
    uint16_t flags;
    uint16_t unk0;
    float values3_w;
    float values3_z;
    float values4_y;
    float unk1;
    float unk2;
    float unk3;
    float unk4;
});

void initEXP2(EXP2 *exp2);

void initM2Textures(void* sectionStart, M2Array<M2Texture> &textures);

void initCompBones(void* sectionStart, M2Array<M2CompBone> *bones, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Color(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2TextureWeight(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2TextureTransform(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Attachment(void* sectionStart, M2Array<M2Attachment> *attachments, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Event(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Light(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Particle(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Ribbon(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2Camera(M2Data *m2Header, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad);
void initM2ParticlePartTracks(M2Data *m2Header);

int findAnimationIndex(uint32_t anim_id, M2Array<int16_t> *sequence_lookups, M2Array<M2Sequence> *sequences);

template <typename ToCheck, std::size_t ExpectedSize, std::size_t RealSize = sizeof(ToCheck)>
void check_size() {
    static_assert(ExpectedSize == RealSize, "Size is off!");
}
template <std::size_t RealOffset, std::size_t ExpectedOffset>
void check_offset() {
    static_assert(ExpectedOffset == RealOffset, "Offset is off!");
}

template <typename T, typename M> M get_member_type(M T::*);
template <typename T, typename M> T get_class_type(M T::*);

template <typename T,
        typename R,
        R T::*M
>
constexpr std::size_t offset_of()
{
    return reinterpret_cast<std::size_t>(&(((T*)0)->*M));
}

#define OFFSET_OF(m) offset_of<decltype(get_class_type(m)), \
                     decltype(get_member_type(m)), m>()





#endif //WOWVIEWERLIB_M2FILEHEADER_H
