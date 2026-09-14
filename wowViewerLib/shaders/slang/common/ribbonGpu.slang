//
// GPU ribbon simulation: shared data structures.
// Mirrors engine/objects/m2/m2Helpers/M2GpuAnimData.h (C++) 1:1 — all buffers are
// ScalarDataLayout. vec3 groups use float[4] so C++ and slang layouts match exactly.
//
#ifndef RIBBON_GPU_SLANG
#define RIBBON_GPU_SLANG

import glsl;
#include "m2TrackEval.slang"

// GpuM2RibbonStatic (C++), 96 bytes
struct GpuM2RibbonStatic {
    float position[3];
    int boneIndex;
    float edgesPerSec;
    float edgeLifeSpan;
    float gravity;
    int textureRows;
    int textureCols;
    float texBoxMinX;
    float texBoxMinY;
    float texBoxMaxX;
    float texBoxMaxY;
    float tmpDU;
    float tmpDV;
    float ooLifeSpan;
    int priorityPlane;
    int colorTrack;
    int alphaTrack;
    int heightAboveTrack;
    int heightBelowTrack;
    int texSlotTrack;
    int visibilityTrack;
    int pad[1];
};

// GpuM2RibbonFrameProps (C++), 64 bytes
struct GpuM2RibbonFrameProps {
    float color[3];
    float alpha;
    float above;
    float below;
    float texSlotBox[4];
    int dataEnabled;
    int pad[5];
};

// GpuRibbonEdge (C++), 32 bytes
struct GpuRibbonEdge {
    float belowPos[3];
    float age;
    float abovePos[3];
    float pad;
};

// GpuRibbonState (C++), 256 bytes
struct GpuRibbonState {
    int readPos;
    int writePos;
    float startTime;
    int posSet;
    int singletonUpdated;
    int pad0[3];

    float prevPos[4];
    float currPos[4];
    float prevVertical[4];
    float currVertical[4];
    float prevDir[4];
    float currDir[4];
    float prevDirScaled[4];
    float currDirScaled[4];
    float below0[4];
    float below1[4];
    float above0[4];
    float above1[4];

    int staticsIndex;
    int propsIndex;
    int edgesOffset;
    int edgeCount;
    int placementMatIndex;
    int boneOutOffset;
    int iboOffset;
    int iboCount;
};

#endif // RIBBON_GPU_SLANG
