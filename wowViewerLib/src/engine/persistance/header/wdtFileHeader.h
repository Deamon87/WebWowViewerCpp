//
// Created by deamon on 28.11.17.
//

#ifndef WEBWOWVIEWERCPP_WDTFILEHEADER_H_H
#define WEBWOWVIEWERCPP_WDTFILEHEADER_H_H

#include "commonFileStructs.h"

struct MPHDFlags {
    //0x1
    uint32_t wdt_uses_global_map_obj : 1;
    //0x2
    uint32_t adt_has_mccv : 1;
    //0x4
    uint32_t adt_has_big_alpha : 1;
    //0x8
    uint32_t adt_has_doodadrefs_sorted_by_size_cat : 1;
    //0x10
    uint32_t adt_has_mclv : 1; //adds second color: ADT.MCNK.MCLV
    //0x20
    uint32_t adt_has_upside_down_ground : 1; //Flips the ground display upside down to create a ceiling
    //0x40
    uint32_t unk_0x0040 : 1;
    //0x80
    uint32_t adt_has_height_texturing : 1; //shader = 6. Decides whether to influence alpha maps by _h+MTXP: (without with)
    //0x100
    uint32_t unk_0x0100 : 1; //implicitly sets 0x8000
    //0x200
    uint32_t unk_0x0200 : 1;
    //0x400
    uint32_t unk_0x0400 : 1;
    //0x800
    uint32_t unk_0x0800 : 1;
    //0x1000
    uint32_t unk_0x1000 : 1;
    //0x2000
    uint32_t unk_0x2000 : 1;
    //0x4000
    uint32_t unk_0x4000 : 1;
    //0x4000
    uint32_t unk_0x8000 : 1; //implicitly set for map ids 0, 1, 571, 870, 1116 (continents). Affects the rendering of _lod.adt

};

struct MPHD {
    MPHDFlags flags;
    uint32_t something;
    uint32_t unused[6];
};

struct MAIN {
    uint32_t Flag_AllWater : 1;
    uint32_t Flag_Loaded : 1;
    uint32_t asyncId;    // only set during runtime.
};

#endif //WEBWOWVIEWERCPP_WDTFILEHEADER_H_H
