//
// Created by Deamon on 8/7/2019.
//

#ifndef AWEBWOWVIEWERCPP_SKELFILEHEADER_H
#define AWEBWOWVIEWERCPP_SKELFILEHEADER_H

#include "commonFileStructs.h"
#include "M2FileHeader.h"

struct skeleton_l_header {
    uint32_t _0x00;                       // Version? Flags? Always 0x100 as of Legion (7.3.2.25079).
    M2Array<char> name;
    uint8_t _0x0c[4];                     // Is this already part of the data? Always 0 as of Legion (7.3.2.25079).
} ;

struct skeleton_attachment_header {
    M2Array<M2Attachment> attachments;
    M2Array<uint16_t> attachment_lookup_table;
} ;

struct skeleton_bone_header {
    M2Array<M2CompBone> bones;
    M2Array<uint16_t> key_bone_lookup;
} ;

struct skeleton_sequence_header {
    M2Array<M2Loop> global_loops;
    M2Array<M2Sequence> sequences;
    M2Array<uint16_t> sequence_lookups;
    uint8_t _0x18[8];                     // Is this already part of the data? Always 0 as of Legion (7.3.2.25079).
} ;

struct skeleton_parent_data {
    uint8_t _0x00[8];                     // Always 0 as of Legion (7.3.2.25079).
    uint32_t parent_skel_file_id;
    uint8_t _0x0c[4];                     // Always 0 as of Legion (7.3.2.25079).
} ;

#endif //AWEBWOWVIEWERCPP_SKELFILEHEADER_H
