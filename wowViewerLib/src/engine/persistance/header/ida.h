//
// Created by Deamon on 8/29/2017.
//

#define uint16_t _WORD;
#define char _BYTE;

struct CAaBox
{
    float min[3];
    float max[3];
};
struct M2Array {
    int32_t size;
    uint32_t offset; // pointer to T, relative to begin of m2 data block (i.e. MD21 chunk content or begin of file)
};

struct M2Data {
    uint32_t magic;                                       // "MD20". Legion uses a chunked file format starting with MD21.
    uint32_t version;
    M2Array name;                                   // should be globally unique, used to reload by name in internal clients

    struct
    {
        uint32_t flag_tilt_x : 1;
        uint32_t flag_tilt_y : 1;
        uint32_t : 1;
        uint32_t flag_has_blend_maps : 1;                   // add BlendMaps fields in header
        uint32_t : 1;
        uint32_t flag_load_phys_data : 1;
        uint32_t : 1;
        uint32_t flag_unk_0x80 : 1;                         // with this flag unset, demon hunter tattoos stop glowing
        uint32_t flag_camera_related : 1;                   // TODO: verify version
        uint32_t flag_new_particle_record : 1;              // In CATA: new version of ParticleEmitters. By default, length of M2ParticleOld is 476.
        uint32_t flag_unk_0x400 : 1;
        uint32_t flag_unk_0x800 : 1;
        uint32_t flag_unk_0x1000 : 1;
        uint32_t flag_unk_0x2000 : 1;                       // seen in UI_MainMenu_Legion
        uint32_t flag_unk_0x4000 : 1;
        uint32_t flag_unk_0x8000 : 1;                       // seen in UI_MainMenu_Legion
    } global_flags;

    M2Array global_loops;                        // Timestamps used in global looping animations.
    M2Array sequences;                       // Information about the animations in the model.
    M2Array sequence_lookups;                  // Mapping of sequence IDs to the entries in the Animation sequences block.
    M2Array bones;                           // MAX_BONES = 0x100
    M2Array key_bone_lookup;                   // Lookup table for key skeletal bones.
    M2Array vertices;
    uint32_t num_skin_profiles;                           // Views (LOD) are now in .skins.
    M2Array colors;                             // Color and alpha animations definitions.
    M2Array textures;
    M2Array texture_weights;            // Transparency of textures.
    M2Array texture_transforms;
    M2Array replacable_texture_lookup;
    M2Array materials;                       // Blending modes / render flags.
    M2Array bone_lookup_table;
    M2Array texture_lookup_table;
    M2Array tex_unit_lookup_table;             // ≥ Cata : unused
    M2Array transparency_lookup_table;
    M2Array texture_transforms_lookup_table;

    CAaBox bounding_box;                                 // min/max( [1].z, 2.0277779f ) - 0.16f seems to be the maximum camera height
    float bounding_sphere_radius;                         // detail doodad draw dist = clamp (bounding_sphere_radius * detailDoodadDensityFade * detailDoodadDist, …)
    CAaBox collision_box;
    float collision_sphere_radius;

    M2Array collision_triangles;
    M2Array collision_vertices;
    M2Array collision_normals;
    M2Array attachments;                   // position of equipped weapons or effects
    M2Array attachment_lookup_table;
    M2Array events;                             // Used for playing sounds when dying and a lot else.
    M2Array lights;                             // Lights are mainly used in loginscreens but in wands and some doodads too.
    M2Array cameras;                           // The cameras are present in most models for having a model in the character tab.
    M2Array camera_lookup_table;
    M2Array ribbon_emitters;                   // Things swirling around. See the CoT-entrance for light-trails.
    M2Array particle_emitters;
    M2Array blend_map_overrides;             // When set, textures blending is overriden by the associated array.
};