//
// Created by deamon on 03.05.18.
//

#ifndef WEBWOWVIEWERCPP_TEXFILEHEADER_H
#define WEBWOWVIEWERCPP_TEXFILEHEADER_H

#include "commonFileStructs.h"

struct SBlobTexture {
    uint32_t filenameOffset;
    uint32_t txmdOffset; // TXMD-chunk offset relative to end of TXFN-chunk.
    uint8_t sizex;
    uint8_t sizey; // special case: sizex = sizey * 6
    uint8_t m_numLevels : 7; // mipmaps
    uint8_t loaded : 1;
    uint8_t dxt_type : 4; // 0  = Dxt1, 1 = Dxt3, 2 = Dxt5
    uint8_t flags : 4; // 1 = if dxt_type = Dxt1, prefer Argb1555 over Rgb565, if Dxt1 unavailable
};

#endif //WEBWOWVIEWERCPP_TEXFILEHEADER_H
