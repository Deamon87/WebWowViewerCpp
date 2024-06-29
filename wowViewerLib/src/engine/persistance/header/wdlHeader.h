//
// Created by Deamon on 15.05.2020.
//

#ifndef AWEBWOWVIEWERCPP_WDLHEADER_H
#define AWEBWOWVIEWERCPP_WDLHEADER_H

#include <cstdint>
#include "commonFileStructs.h"

struct msso_t
{
    uint32_t sceneModelId;
    union {
        uint32_t flags_raw;
        struct {
            uint32_t unk_0x1: 1; //0x1
            uint32_t unk_0x2: 1; //0x2
            uint32_t animateWithTimeOfDay: 1; //0x4
        } flags;
    };
    uint32_t fileDataID;
    C3Vector translateVec;
    C3Vector rotationInDegree;
    float scale;
    uint16_t mssf_index;
    uint16_t unk_10;
    uint32_t unk_11;
};

struct mssn_t
{
    uint32_t SkySceneID;
    uint32_t unk_1;        // Number of records in SkySceneXPlayerCondition?
    int16_t msscIndex;
    int16_t msscRecordsNum;
    uint16_t mssoIndex;
    int16_t mssoRecordsNum;
    uint32_t unk_4;
    uint32_t unk_5;
    uint32_t unk_6;
    uint32_t unk_7;
};

struct mssc_t
{
    uint32_t unk_0;
    uint32_t unk_1;
    uint32_t conditionType;
    uint32_t unk_3;
    uint32_t unk_4;
    uint32_t unk_5;
    uint32_t conditionValue;
};

struct msld_t
{
  uint32_t unk_0;
  uint32_t unk_1;
  uint32_t unk_2;
  uint32_t unk_3;
  uint32_t timeStart0;
  uint32_t timeStart1;
  uint32_t timeEnd0;
  uint32_t timeEnd1;
};



#endif //AWEBWOWVIEWERCPP_WDLHEADER_H
