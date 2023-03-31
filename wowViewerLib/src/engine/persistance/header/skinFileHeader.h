//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_SKINFILEHEADER_H
#define WOWVIEWERLIB_SKINFILEHEADER_H

#include "commonFileStructs.h"

struct M2SkinSection
{
    uint16_t skinSectionId;       // Mesh part ID, see below.
    uint16_t Level;               // (level << 16) is added (|ed) to startTriangle and alike to avoid having to increase those fields to uint32s.
    uint16_t vertexStart;         // Starting vertex number.
    uint16_t vertexCount;         // Number of vertices.
    uint16_t indexStart;          // Starting triangle index (that's 3* the number of triangles drawn so far).
    uint16_t indexCount;          // Number of triangle indices.
    uint16_t boneCount;           // Number of elements in the bone lookup table.
    uint16_t boneComboIndex;      // Starting index in the bone lookup table.
    uint16_t boneInfluences;      // <= 4
    // from <=BC documentation: Highest number of bones needed at one time in this Submesh --Tinyn (wowdev.org)
    // In 2.x this is the amount of of bones up the parent-chain affecting the submesh --NaK
    uint16_t centerBoneIndex;
    C3Vector centerPosition;     // Average position of all the vertices in the sub mesh.

    C3Vector sortCenterPosition; // The center of the box when an axis aligned box is built around the vertices in the submesh.
    float sortRadius;             // Distance of the vertex farthest from CenterBoundingBox.
};

struct M2Batch
{
    uint8_t flags;                       // Usually 16 for static textures, and 0 for animated textures. &0x1: materials invert something; &0x2: transform &0x4: projected texture; &0x10: something batch compatible; &0x20: projected texture?; &0x40: use textureWeights
    int8_t priorityPlane;
    uint16_t shader_id;                  // See below.
    uint16_t skinSectionIndex;           // A duplicate entry of a submesh from the list above.
    uint16_t geosetIndex;                // See below.
    int16_t colorIndex;                 // A Color out of the Colors-Block or -1 if none.
    uint16_t materialIndex;              // The renderflags used on this texture-unit.
    uint16_t materialLayer;              // Capped at 7 (see CM2Scene::BeginDraw)
    uint16_t textureCount;               // 1 to 4. See below. Also seems to be the number of textures to load, starting at the texture lookup in the next field (0x10).
    uint16_t textureComboIndex;          // Index into Texture lookup table
    uint16_t textureCoordComboIndex;     // Index into the texture unit lookup table.
    uint16_t textureWeightComboIndex;    // Index into transparency lookup table.
    uint16_t textureTransformComboIndex; // Index into uvanimation lookup table.
};

struct M2ShadowBatch
{
    uint8_t flags;              // if auto-generated: M2Batch.flags & 0xFF
    uint8_t flags2;             // if auto-generated: (renderFlag[i].flags & 0x04 ? 0x01 : 0x00)
    //                  | (!renderFlag[i].blendingmode ? 0x02 : 0x00)
    //                  | (renderFlag[i].flags & 0x80 ? 0x04 : 0x00)
    //                  | (renderFlag[i].flags & 0x400 ? 0x06 : 0x00)
    uint16_t _unknown1;
    uint16_t submesh_id;
    uint16_t texture_id;        // already looked-up
    uint16_t color_id;
    uint16_t transparency_id;   // already looked-up
};

struct M2SkinProfile
{
    uint32_t magic;                         // 'SKIN'
    M2Array<unsigned short> vertices ;
    M2Array<unsigned short> indices ;
    M2Array<ubyte4> bones;
    M2Array<M2SkinSection> skinSections;
    M2Array<M2Batch> batches;
    uint32_t boneCountMax;                  // WoW takes this and divides it by the number of bones in each submesh, then stores the biggest one.
    // Maximum number of bones per drawcall for each view. Related to (old) GPU numbers of registers.
    // Values seen : 256, 64, 53, 21
//#if ≥ Cata
//    M2Array<M2ShadowBatch> shadow_batches;
//#endif
};

#endif //WOWVIEWERLIB_SKINFILEHEADER_H
