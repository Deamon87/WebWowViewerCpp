#include <iostream>
#include "CRibbonEmitter.h"

//----- (00A19710) --------------------------------------------------------
CRibbonEmitter::CRibbonEmitter()
{
  CRibbonEmitter *result; // eax

  this->m_refCount = 1;
  this->m_prevPos.x = 0.0;
  this->m_prevPos.y = 0.0;
  this->m_prevPos.z = 0.0;
  this->m_pos.x = 0.0;
  this->m_pos.y = 0.0;
  this->m_pos.z = 0.0;
  this->m_texSlotBox.miny = 0.0;
  this->m_texSlotBox.minx = 0.0;
  this->m_texSlotBox.maxy = 0.0;
  this->m_texSlotBox.maxx = 0.0;
  this->m_prevVertical.x = 0.0;
  this->m_prevVertical.y = 0.0;
  this->m_prevVertical.z = 0.0;
  this->m_currVertical.x = 0.0;
  this->m_currVertical.y = 0.0;
  this->m_currVertical.z = 0.0;
  this->m_prevDir.x = 0.0;
  this->m_prevDir.y = 0.0;
  this->m_prevDir.z = 0.0;
  this->m_currDir.x = 0.0;
  this->m_currDir.y = 0.0;
  this->m_currDir.z = 0.0;
  this->m_prevDirScaled.x = 0.0;
  this->m_prevDirScaled.y = 0.0;
  this->m_prevDirScaled.z = 0.0;
  this->m_currDirScaled.x = 0.0;
  this->m_currDirScaled.y = 0.0;
  this->m_currDirScaled.z = 0.0;
  this->m_below0.x = 0.0;
  this->m_below0.y = 0.0;
  this->m_below0.z = 0.0;
  this->m_below1.x = 0.0;
  this->m_below1.y = 0.0;
  this->m_below1.z = 0.0;
  this->m_above0.x = 0.0;
  this->m_above0.y = 0.0;
  this->m_above0.z = 0.0;
  this->m_above1.x = 0.0;
  this->m_above1.y = 0.0;
  this->m_above1.z = 0.0;
  this->m_minWorldBounds.z = 3.4028235e38f;
  this->m_minWorldBounds.y = 3.4028235e38f;
  this->m_minWorldBounds.x = 3.4028235e38f;
  this->m_maxWorldBound.z = -3.4028235e38f;
  this->m_maxWorldBound.y = -3.4028235e38f;
  this->m_maxWorldBound.x = -3.4028235e38f;
  this->m_diffuseClr = {0,0,0,0};
  this->m_texBox.miny = 0.0;
  this->m_texBox.minx = 0.0;
  this->m_texBox.maxy = 0.0;
  this->m_texBox.maxx = 0.0;
  this->m_ribbonEmitterflags.m_singletonUpdated = 0;
  this->m_ribbonEmitterflags.m_initialized = 0;
  this->m_currPos.x = 0.0;
  this->m_currPos.y = 0.0;
  this->m_currPos.z = 0.0;
}

//----- (00A199E0) --------------------------------------------------------
void __cdecl CRibbonEmitter::SetDataEnabled(char a2)
{
  this->m_ribbonEmitterflags.m_dataEnabled = a2;
  if ( !(this->m_ribbonEmitterflags.m_dataEnabled) )
    this->m_ribbonEmitterflags.m_posSet = 0;
}

//----- (00A19A20) --------------------------------------------------------
void CRibbonEmitter::SetUserEnabled(char a2)
{
  
  this->m_ribbonEmitterflags.m_userEnabled = 0;
  if ( !(this->m_ribbonEmitterflags.m_userEnabled) )
    this->m_ribbonEmitterflags.m_posSet = 0;
}

//----- (00A19A60) --------------------------------------------------------
CRibbonEmitter *__cdecl CRibbonEmitter::SetGravity(float a2)
{
  CRibbonEmitter *result; // eax

  result = this;
  this->m_gravity = a2;
  return result;
}

//----- (00A19A80) --------------------------------------------------------
bool CRibbonEmitter::IsDead()
{
  return this->m_readPos == this->m_writePos;
}

//----- (00A19AA0) --------------------------------------------------------
void CRibbonEmitter::SetBelow(float below)
{
  assert(below >= 0.0f);
  this->m_below = below;
}

//----- (00A19B10) --------------------------------------------------------
void CRibbonEmitter::SetAbove(float above)
{
    assert(above >= 0.0f);
    this->m_above = above;
}

//----- (00A19B80) --------------------------------------------------------
void CRibbonEmitter::SetAlpha(float a2)
{
  this->m_diffuseClr.a = fmaxf(255.0f * a2, 0.0f);
}


//----- (00A19C50) --------------------------------------------------------
void __cdecl CRibbonEmitter::InitInterpDeltas()
{
  float diffLen = (this->m_prevPos - this->m_currPos).LengthSquared();
  assert(diffLen >= 0.0);

  float scale = sqrtf(diffLen);

  this->m_below0 = this->m_prevPos - (this->m_below * this->m_prevVertical);
  this->m_below1 = this->m_currPos - (this->m_below * this->m_currVertical);

  this->m_above0 = (this->m_above * this->m_prevVertical) + this->m_prevPos;
  this->m_above1 = (this->m_above * this->m_currVertical) + this->m_currPos;

  this->m_prevDirScaled = scale * this->m_prevDir;
  this->m_currDirScaled = scale * this->m_currDir;
}

//----- (00A19ED0) --------------------------------------------------------
int CRibbonEmitter::GetNumBatches()
{
//  return this->m_materials.size();
  return 0;
}

//----- (00A19EE0) --------------------------------------------------------
void CRibbonEmitter::SetPos(const mathfu::mat4 &modelViewMat, const mathfu::vec3 &a3, const mathfu::mat4 *frameOfReference)
{
  mathfu::mat4 rhs; // [esp+50h] [ebp-58h]

  if (this->m_ribbonEmitterflags.m_userEnabled && this->m_ribbonEmitterflags.m_dataEnabled)
  {
    rhs = modelViewMat;
    mathfu::vec3 position = rhs.GetColumn(3).xyz() + a3;

    rhs.GetColumn(3) = mathfu::vec4(position, 1.0);
    if ( frameOfReference )
    {
        rhs = frameOfReference->Inverse() * rhs;
        position = rhs.GetColumn(3).xyz();
    }
    this->m_pos = a3;
    if ( this->m_ribbonEmitterflags.m_posSet )
    {
      this->m_prevPos = this->m_currPos;
      this->m_prevDir = this->m_currDir;
      this->m_prevVertical = this->m_currVertical;
    }
    else
    {
      this->m_prevPos = position;

      this->m_prevDir = rhs.GetColumn(2).xyz();
      this->m_prevVertical = rhs.GetColumn(1).xyz();;
      this->m_startTime = 0.0;
      this->m_ribbonEmitterflags.m_posSet = 1;
    }
    this->m_currPos = position;

    this->m_currDir = rhs.GetColumn(2).xyz();
    this->m_currVertical = rhs.GetColumn(1).xyz();
  }
}

//----- (00A1A0D0) --------------------------------------------------------
unsigned int __cdecl CRibbonEmitter::GetNumPrimitives()
{
  unsigned int v1; // edx
  unsigned int v2; // eax

  v1 = this->m_readPos;
  v2 = this->m_writePos;
  if ( v1 >= v2 )
    v2 += this->m_edges.size();
  return 2 * (v2 - v1);
}

//----- (00A1A0F0) --------------------------------------------------------
void CRibbonEmitter::ChangeFrameOfReference(const mathfu::mat4 *frameOfReference)
{
  CRibbonEmitter *v2; // esi
  const mathfu::mat4 *l_frameOfReference; // edx
  float v4; // xmm3_4
  float v5; // xmm1_4
  float v6; // xmm5_4
  float v7; // xmm2_4
  float v8; // xmm5_4
  float v9; // xmm2_4
  float v10; // xmm3_4
  float v11; // xmm1_4
  float v12; // xmm5_4
  float v13; // xmm2_4
  float v14; // xmm5_4
  float v15; // xmm2_4
  unsigned int v16; // ebx
  int v17; // ecx
  CRibbonVertex *result; // eax
  float v19; // xmm3_4
  float v20; // xmm1_4

  v2 = this;
//  l_frameOfReference = frameOfReference;
//  v4 = (float)((float)((float)(this->m_prevPos.x * frameOfReference->m_elements[0][2])
//                     + (float)(this->m_prevPos.y * frameOfReference->m_elements[1][2]))
//             + (float)(this->m_prevPos.z * frameOfReference->m_elements[2][2]))
//     + frameOfReference->m_elements[3][2];
//  v5 = (float)((float)((float)(this->m_prevPos.x * frameOfReference->m_elements[0][1])
//                     + (float)(this->m_prevPos.y * frameOfReference->m_elements[1][1]))
//             + (float)(this->m_prevPos.z * frameOfReference->m_elements[2][1]))
//     + frameOfReference->m_elements[3][1];
//  this->m_prevPos.x = (float)((float)((float)(this->m_prevPos.x * frameOfReference->m_elements[0][0])
//                                    + (float)(this->m_prevPos.y * frameOfReference->m_elements[1][0]))
//                            + (float)(this->m_prevPos.z * frameOfReference->m_elements[2][0]))
//                    + frameOfReference->m_elements[3][0];
//  this->m_prevPos.y = v5;
//  this->m_prevPos.z = v4;
//  v6 = (float)((float)(v2->m_prevDir.x * l_frameOfReference->m_elements[0][1])
//             + (float)(v2->m_prevDir.y * l_frameOfReference->m_elements[1][1]))
//     + (float)(v2->m_prevDir.z * l_frameOfReference->m_elements[2][1]);
//  v7 = (float)((float)(v2->m_prevDir.x * l_frameOfReference->m_elements[0][0])
//             + (float)(v2->m_prevDir.y * l_frameOfReference->m_elements[1][0]))
//     + (float)(v2->m_prevDir.z * l_frameOfReference->m_elements[2][0]);
//  v2->m_prevDir.z = (float)((float)(v2->m_prevDir.x * l_frameOfReference->m_elements[0][2])
//                          + (float)(v2->m_prevDir.y * l_frameOfReference->m_elements[1][2]))
//                  + (float)(v2->m_prevDir.z * l_frameOfReference->m_elements[2][2]);
//  this->m_prevDir.y = v6;
//  this->m_prevDir.x = v7;
//  v8 = (float)((float)(v2->m_prevVertical.x * l_frameOfReference->m_elements[0][1])
//             + (float)(v2->m_prevVertical.y * l_frameOfReference->m_elements[1][1]))
//     + (float)(v2->m_prevVertical.z * l_frameOfReference->m_elements[2][1]);
//  v9 = (float)((float)(v2->m_prevVertical.x * l_frameOfReference->m_elements[0][0])
//             + (float)(v2->m_prevVertical.y * l_frameOfReference->m_elements[1][0]))
//     + (float)(v2->m_prevVertical.z * l_frameOfReference->m_elements[2][0]);
//  v2->m_prevVertical.z = (float)((float)(v2->m_prevVertical.x * l_frameOfReference->m_elements[0][2])
//                               + (float)(v2->m_prevVertical.y * l_frameOfReference->m_elements[1][2]))
//                       + (float)(v2->m_prevVertical.z * l_frameOfReference->m_elements[2][2]);
//  this->m_prevVertical.y = v8;
//  this->m_prevVertical.x = v9;
//  v10 = (float)((float)((float)(v2->m_currPos.x * l_frameOfReference->m_elements[0][2])
//                      + (float)(v2->m_currPos.y * l_frameOfReference->m_elements[1][2]))
//              + (float)(v2->m_currPos.z * l_frameOfReference->m_elements[2][2]))
//      + l_frameOfReference->m_elements[3][2];
//  v11 = (float)((float)((float)(v2->m_currPos.x * l_frameOfReference->m_elements[0][1])
//                      + (float)(v2->m_currPos.y * l_frameOfReference->m_elements[1][1]))
//              + (float)(v2->m_currPos.z * l_frameOfReference->m_elements[2][1]))
//      + l_frameOfReference->m_elements[3][1];
//  v2->m_currPos.x = (float)((float)((float)(v2->m_currPos.x * l_frameOfReference->m_elements[0][0])
//                                  + (float)(v2->m_currPos.y * l_frameOfReference->m_elements[1][0]))
//                          + (float)(v2->m_currPos.z * l_frameOfReference->m_elements[2][0]))
//                  + l_frameOfReference->m_elements[3][0];
//  this->m_currPos.y = v11;
//  this->m_currPos.z = v10;
//  v12 = (float)((float)(v2->m_currDir.x * l_frameOfReference->m_elements[0][1])
//              + (float)(v2->m_currDir.y * l_frameOfReference->m_elements[1][1]))
//      + (float)(v2->m_currDir.z * l_frameOfReference->m_elements[2][1]);
//  v13 = (float)((float)(v2->m_currDir.x * l_frameOfReference->m_elements[0][0])
//              + (float)(v2->m_currDir.y * l_frameOfReference->m_elements[1][0]))
//      + (float)(v2->m_currDir.z * l_frameOfReference->m_elements[2][0]);
//  v2->m_currDir.z = (float)((float)(v2->m_currDir.x * l_frameOfReference->m_elements[0][2])
//                          + (float)(v2->m_currDir.y * l_frameOfReference->m_elements[1][2]))
//                  + (float)(v2->m_currDir.z * l_frameOfReference->m_elements[2][2]);
//  this->m_currDir.y = v12;
//  this->m_currDir.x = v13;
//  v14 = (float)((float)(v2->m_currVertical.x * l_frameOfReference->m_elements[0][1])
//              + (float)(v2->m_currVertical.y * l_frameOfReference->m_elements[1][1]))
//      + (float)(v2->m_currVertical.z * l_frameOfReference->m_elements[2][1]);
//  v15 = (float)((float)(v2->m_currVertical.x * l_frameOfReference->m_elements[0][0])
//              + (float)(v2->m_currVertical.y * l_frameOfReference->m_elements[1][0]))
//      + (float)(this->m_currVertical.z * frameOfReference->m_elements[2][0]);
//  v2->m_currVertical.z = (float)((float)(v2->m_currVertical.x * l_frameOfReference->m_elements[0][2])
//                               + (float)(v2->m_currVertical.y * l_frameOfReference->m_elements[1][2]))
//                       + (float)(v2->m_currVertical.z * l_frameOfReference->m_elements[2][2]);
//  this->m_currVertical.y = v14;
//  this->m_currVertical.x = v15;
  if ( this->m_gxVertices.size() )
  {
    v16 = 0;
    v17 = 0;
    do
    {
      result = &this->m_gxVertices[v17];

      result->pos = (*frameOfReference * mathfu::vec4(result->pos, 1.0f)).xyz();
      ++v16;
      ++v17;
    }
    while ( this->m_gxVertices.size() > v16 );
  }
}

//----- (00A1A4E0) --------------------------------------------------------
void CRibbonEmitter::SetColor(float a3, float a4, float a5)
{
    this->m_diffuseClr.r = a3;
    this->m_diffuseClr.g = a4;
    this->m_diffuseClr.b = a5;

}
// FCBEB0: using guessed type int dword_FCBEB0;

//----- (00A1A820) --------------------------------------------------------
void CRibbonEmitter::SetTexSlot(unsigned int texSlot)
{
  assert(texSlot < m_rows * m_cols);
  
  if ( this->m_texSlot != texSlot )
  {
    this->m_texSlot = texSlot;
    int texSlotMod = texSlot % this->m_cols;
    float texSlotModf = texSlotMod;
    if ( (texSlotMod & 0x80000000) != 0 ) {
        texSlotModf = (float) (((texSlotMod) & 1) | (texSlotMod >> 1)) +
             (float) ((texSlotMod & 1) | (texSlotMod >> 1));
    }
    
    float minx = (texSlotModf * this->m_tmpDU) + this->m_texBox.minx;
    this->m_texSlotBox.minx = minx;

    int texSlotDiv = texSlot / this->m_cols;
    float texSlotDivf = texSlotDiv;
    if ( ((texSlotDiv) & 0x80000000) != 0 )
    {
      texSlotDiv = ((texSlotDiv) & 1) | ((texSlotDiv) >> 1);
      texSlotDivf = texSlotDiv + texSlotDiv;
      minx = this->m_texSlotBox.minx;
    }
    float miny = (texSlotDivf * this->m_tmpDV) + this->m_texBox.miny;
    this->m_texSlotBox.miny = miny;
    this->m_texSlotBox.maxx = minx + this->m_tmpDU;
    this->m_texSlotBox.maxy = miny + this->m_tmpDV;
  }
  
}

//----- (00A1A960) --------------------------------------------------------
void  CRibbonEmitter::InterpEdge(float age, float t, unsigned int advance)
{
  unsigned int l_amount; // eax

  assert(2 * this->m_writePos < this->m_gxVertices.size());

  CRibbonVertex * firstVertex = &this->m_gxVertices[2 * this->m_writePos];
  assert(2 * this->m_writePos + 1 < this->m_gxVertices.size());


  auto secondVertex = &this->m_gxVertices[2 * this->m_writePos + 1];

  firstVertex->pos = ((this->m_below1 - ((1.0f - t) * this->m_currDirScaled)) * t)
             + (((t * this->m_prevDirScaled) + this->m_below0) * (1.0f - t));

  secondVertex->pos = ((this->m_above1 - ((1.0f - t) * this->m_currDirScaled)) * t)
            + ((1.0f - t) * ((t * this->m_prevDirScaled) + this->m_above0));

  assert(this->m_writePos < this->m_edges.size());

  this->m_edges[this->m_writePos] = age;
  l_amount = advance;
  assert(advance <= this->m_edges.size());

  this->m_writePos = this->m_writePos + l_amount;
  if ( this->m_writePos >= this->m_edges.size() )
  {
    this->m_writePos -= this->m_edges.size();
  }
  assert((this->m_writePos != this->m_readPos) || (advance == 0));

}

void CRibbonEmitter::Advance(int &pos, unsigned int amount) {
    CRibbonEmitter *v3; // esi
    unsigned int v4; // eax
    unsigned int v5; // edx

    v3 = this;
    assert(amount < this->m_edges.size());

    v4 = amount + pos;
    pos = v4;
    v5 = v3->m_edges.size();
    if (v4 >= v5)
        pos = v4 - v5;
}

//----- (00A1AD60) --------------------------------------------------------
void CRibbonEmitter::Update(float deltaTime, int suppressNewEdges)
{
  float v4; // xmm0_4
  unsigned int v5; // esi
  unsigned int v6; // eax
  int j; // eax
  unsigned int v9; // esi
  CRibbonVertex *v10; // esi
  float v11; // xmm0_4
  float v12; // xmm0_4
  float v13; // xmm2_4
  float v14; // xmm4_4
  float v15; // xmm0_4
  float v16; // xmm3_4
  float v17; // xmm1_4
  float v18; // xmm5_4
  float v19; // xmm2_4
  float v20; // xmm1_4
  float v21; // xmm0_4
  float v22; // xmm0_4
  float v23; // xmm0_4
  unsigned int v24; // edx
  int v25; // esi
  int v26; // ebx
  int v27; // eax
  int v28; // eax
  int v29; // ebx
  int v30; // eax
  int v31; // ebx
  int v32; // eax
  int v33; // ebx
  int v34; // eax
  int v35; // ebx
  int v36; // eax
  __int32 result; // eax
  CImVector v38; // eax
  float v39; // ST50_4

  float i; // xmm0_4
  unsigned int v45; // esi
  int v46; // ebx
  int v47; // eax
  int v48; // ebx
  int v49; // eax
  float v50; // ST48_4
  CRibbonVertex *v51; // eax
  float v52; // ecx
  unsigned int v53; // esi
  CRibbonVertex *v54; // eax
  float v55; // ecx
  unsigned int v56; // esi
  int v57; // ebx
  int v58; // eax
  int v59; // ebx
  int v60; // eax
  int v61; // ebx
  int v62; // eax
  int v63; // ebx
  int v64; // eax
  CImVector v65; // ebx
  char *v66; // [esp+2Ch] [ebp-BCh]
  char *v67; // [esp+30h] [ebp-B8h]
  char *v68; // [esp+34h] [ebp-B4h]
  char *v69; // [esp+38h] [ebp-B0h]
  unsigned int v70; // [esp+3Ch] [ebp-ACh]
  int v71; // [esp+40h] [ebp-A8h]
  int v72; // [esp+44h] [ebp-A4h]
  int v73; // [esp+4Ch] [ebp-9Ch]
  float v74; // [esp+68h] [ebp-80h]
  float v75; // [esp+6Ch] [ebp-7Ch]
  float v76; // [esp+70h] [ebp-78h]
  float v77; // [esp+74h] [ebp-74h]
  float ooDenom; // [esp+78h] [ebp-70h]
  int v79; // [esp+7Ch] [ebp-6Ch]
  unsigned int v80; // [esp+84h] [ebp-64h]
  unsigned int v81; // [esp+84h] [ebp-64h]
  CRibbonVertex *v82; // [esp+88h] [ebp-60h]
  unsigned int v83; // [esp+8Ch] [ebp-5Ch]
  unsigned int v84; // [esp+90h] [ebp-58h]
  unsigned int v85; // [esp+94h] [ebp-54h]
  unsigned int v86; // [esp+98h] [ebp-50h]
  unsigned int v87; // [esp+9Ch] [ebp-4Ch]
  unsigned int v88; // [esp+A0h] [ebp-48h]
  unsigned int v89; // [esp+A4h] [ebp-44h]
  unsigned int v90; // [esp+A8h] [ebp-40h]
  unsigned int v91; // [esp+ACh] [ebp-3Ch]
  unsigned int v92; // [esp+B0h] [ebp-38h]
  unsigned int v93; // [esp+B4h] [ebp-34h]
  float v94; // [esp+B8h] [ebp-30h]
  CImVector v95; // [esp+CCh] [ebp-1Ch]

  assert(this->m_ribbonEmitterflags.m_initialized);

  if ( !(this->m_ribbonEmitterflags.m_singletonUpdated) )
  {
    v4 = this->m_edgesPerSec;
    if ( this->m_edgesPerSec > 0.0 )
      deltaTime = (float)(1.0 / this->m_edgesPerSec) + 0.000099999997f;
  }
  if ( deltaTime >= 0.0 )
  {
    if ( this->m_edgeLifeSpan <= deltaTime )
    {
      deltaTime = this->m_edgeLifeSpan;
      v5 = this->m_readPos;
    }
  }
  else
  {
    deltaTime = 0.0;
    v5 = this->m_readPos;
  }

  for ( i = this->m_readPos; i != this->m_writePos; i = this->m_readPos )
  {
      assert(i < this->m_edges.size() );

     if ( deltaTime + this->m_edges[i] <= this->m_edgeLifeSpan )
        break;
     this->Advance(this->m_readPos, 1u);
  }
  if ( !suppressNewEdges
    && (this->m_ribbonEmitterflags.m_userEnabled & this->m_ribbonEmitterflags.m_dataEnabled &  this->m_ribbonEmitterflags.m_posSet))
  {
    v76 = (float)(deltaTime * this->m_edgesPerSec) + this->m_startTime;

    v95 = this->m_diffuseClr;
    this->InitInterpDeltas();
    bool bool_a = false;
    v79 = 0;
    if ( !(v76 < 1.0) ) {

        v94 = this->m_startTime;
        ooDenom = 1.0f / (float) (v76 - v94);
        v39 = floorf(v76 - 1.0f);
        v79 = ceil(fmaxf(v39, 0.0));
    } else {
        bool_a = true;
    }
    if ( v79 == -1 || bool_a )
    {
      v69 = (char *)&this->m_gxVertices;
    }
    else
    {
      v77 = 1.0;
      v69 = (char *)&this->m_gxVertices;
      for ( i = 1.0; ; i = v77 )
      {
        v73 = this->m_writePos;
        v84 = this->m_gxVertices.size();
        assert( 2 * v73 < v84 );
        this->m_gxVertices[2 * v73].diffuseColor = v95;
        v45 = 2 * this->m_writePos + 1;
        v85 = this->m_gxVertices.size();
        assert( v45 < v85 );

        this->m_gxVertices[v45].diffuseColor = v95;
//    std::cout << __FUNCTION__ << " "<< __LINE__ << " this->m_readPos = " << this->m_readPos << " this->m_writePos = " << this->m_writePos << std::endl;
        this->InterpEdge(
          (float)((float)(i - v94) * ooDenom) * (-deltaTime) ,
          (float)(i - v94) * ooDenom,
          1u);
        if ( --v79 == -1 )
          break;
        v77 = v77 + 1.0f;
        v94 = this->m_startTime;
      }
    }
    v50 = floorf(v76);
    this->m_startTime = v76 - v50;
    this->InterpEdge(0.0, 1.0, 0);
    v72 = this->m_writePos;
    v86 = this->m_gxVertices.size();
    assert( 2 * v72 < v86 );

    v51 = &this->m_gxVertices[2 * v72];
    v52 = this->m_texSlotBox.minx;
    v51->texCoord.y = this->m_texSlotBox.miny;
    v51->texCoord.x = v52;
    v53 = 2 * this->m_writePos + 1;
    v87 = this->m_gxVertices.size();
    assert( v53 < v87 );

    v54 = &this->m_gxVertices[v53];
    v55 = this->m_texSlotBox.minx;
    v54->texCoord.y = this->m_texSlotBox.maxy;
    v54->texCoord.x = v55;
    v71 = this->m_writePos;
    v88 = this->m_gxVertices.size();
    assert(2 * v71 < v88 );

    this->m_gxVertices[2 * v71].diffuseColor = v95;
    v56 = 2 * this->m_writePos + 1;
    v89 = this->m_gxVertices.size();
    assert(v56 < v89 );

    this->m_gxVertices[v56].diffuseColor = v95;
  }
  this->m_minWorldBounds.z = 3.4028235e38f;
  this->m_minWorldBounds.y = 3.4028235e38f;
  this->m_minWorldBounds.x = 3.4028235e38f;
  this->m_maxWorldBound.z = -3.4028235e38f;
  this->m_maxWorldBound.y = -3.4028235e38f;
  this->m_maxWorldBound.x = -3.4028235e38f;
  v80 = this->m_readPos;
  v68 = (char *)&this->m_gxVertices;
  v67 = (char *)&this->m_edges;
  for ( j = this->m_readPos; j != this->m_writePos; v80 = j )
  {
    v25 = 2 * j;
    v93 = this->m_gxVertices.size();
    assert( 2 * j < v93 );

    v70 = v80;
    v82 = &this->m_gxVertices[2 * j];
    v9 = v25 + 1;
    assert( v9 < v93 );

    v10 = &this->m_gxVertices[2 * j + 1];
    v11 = this->m_gravity;
    v74 = this->m_gravity;
    v90 = this->m_edges.size();
    assert(v80 < this->m_edges.size());

    v12 = (float)((float)((float)(this->m_gravity + this->m_gravity) * this->m_edges[v70]) * deltaTime)
        + (float)((float)(deltaTime * this->m_gravity) * deltaTime);
    v82->pos.z = v82->pos.z + v12;
    v10->pos.z = v12 + v10->pos.z;
    v13 = v82->pos.x;
    v14 = this->m_minWorldBounds.x;
    if ( v14 > v82->pos.x )
    {
      v14 = v82->pos.x;
      this->m_minWorldBounds.x = v13;
      v13 = v82->pos.x;
    }
    v15 = v82->pos.y;
    v16 = this->m_minWorldBounds.y;
    if ( v16 > v15 )
    {
      v16 = v82->pos.y;
      this->m_minWorldBounds.y = v15;
      v15 = v82->pos.y;
    }
    v17 = v82->pos.z;
    v18 = this->m_minWorldBounds.z;
    if ( v18 > v17 )
    {
      v18 = v82->pos.z;
      this->m_minWorldBounds.z = v17;
      v17 = v82->pos.z;
    }
    if ( v13 > this->m_maxWorldBound.x )
      this->m_maxWorldBound.x = v13;
    if ( v15 > this->m_maxWorldBound.y )
      this->m_maxWorldBound.y = v15;
    if ( v17 > this->m_maxWorldBound.z )
      this->m_maxWorldBound.z = v17;
    v19 = v10->pos.x;
    if ( v14 > v10->pos.x )
    {
      this->m_minWorldBounds.x = v19;
      v19 = v10->pos.x;
    }
    v20 = v10->pos.y;
    if ( v16 > v20 )
    {
      this->m_minWorldBounds.y = v20;
      v20 = v10->pos.y;
    }
    v21 = v10->pos.z;
    if ( v18 > v21 )
    {
      this->m_minWorldBounds.z = v21;
      v21 = v10->pos.z;
    }
    if ( v19 > this->m_maxWorldBound.x )
      this->m_maxWorldBound.x = v19;
    if ( v20 > this->m_maxWorldBound.y )
      this->m_maxWorldBound.y = v20;
    if ( v21 > this->m_maxWorldBound.z )
      this->m_maxWorldBound.z = v21;
    v91 = this->m_edges.size();
    assert(v80 < v91 );

    this->m_edges[v70] = deltaTime + this->m_edges[v70];
    v22 = this->m_tmpDU;
    v75 = this->m_tmpDU;
    v92 = this->m_edges.size();
    assert( v80 < v92 );

    v23 = (float)((float)(v22 * this->m_edges[v70]) * this->m_ooLifeSpan) + this->m_texSlotBox.minx;
    v82->texCoord.y = this->m_texSlotBox.miny;
    v82->texCoord.x = v23;
    v10->texCoord.y = this->m_texSlotBox.maxy;
    v10->texCoord.x = v23;
    v24 = this->m_edges.size();
//    if ( !v24 )
//    {
//      SErrDisplayError(
//        -2062548992,
//        (int)"/Users/patchman/buildserver/wow-b/work/WoW-code/branches/wow-patch-4_1_0-branch/WoW/Source/Mac/../../../Eng"
//             "ine/Source/Services/RibbonEmitter.cpp",
//        156,
//        "amount <= m_edges.Count()",
//        0,
//        1,
//        0);
//      v24 = this->m_edges.m_currentLength;
//    }
    v81 = v80 + 1;
    j = v81 - v24;
    if ( v24 > v81 )
      j = v81;
  }

  this->m_ribbonEmitterflags.m_singletonUpdated = 1;
}
// 10131A0: using guessed type __int128 xmmword_10131A0;

//----- (00A1C1A0) --------------------------------------------------------
void CRibbonEmitter::Initialize(float edgesPerSec, float edgeLifeSpanInSec, CImVector diffuseColor, CRect *a8, unsigned int rows, unsigned int cols)
{
  float v11; // ST2C_4

  unsigned int newEdgesCount; // esi
  unsigned int v17; // edi
  unsigned int v18; // ecx
  unsigned int v19; // eax
  CRibbonVertex *v20; // edx
  unsigned int v21; // esi
  unsigned int v22; // esi
  unsigned int v23; // edi
  int v24; // ebx
  int v25; // eax
  float v26; // xmm1_4
  float v27; // xmm1_4
  CRibbonEmitter *v28; // edi
  char *v29; // esi
  int v30; // edi
  int v31; // ebx
  int v32; // eax
  int v34; // esi
  int v36; // edx
  char *v37; // edi
  int v38; // esi
  int v39; // ebx
  int v40; // eax
  int v41; // edx
  unsigned int *v42; // ecx
  CRibbonEmitter *v43; // ecx
  float v45; // xmm1_4
  float v46; // xmm0_4
  signed int result; // eax
  unsigned int j; // ebx
  unsigned int v49; // ecx
  unsigned int v50; // edi
  int v51; // ebx
  int v52; // eax
  char *v53; // eax
  unsigned int k; // ebx
  int v55; // ecx
  unsigned int i; // ebx
  int v57; // ecx
  unsigned int v58; // edi
  int v59; // ebx
  int v60; // eax
  float *v61; // eax
  int v62; // ebx
  int v63; // eax
  unsigned int v64; // ebx
  int v65; // edx
  float *v66; // ecx
  char *v67; // edi
  int v68; // ebx
  int v69; // eax
  char *v70; // eax
  int v71; // ebx
  int v72; // eax
  unsigned int v73; // ebx
  int v74; // edx
  int v76; // ebx
  int v77; // eax
  unsigned int v78; // eax
  int v79; // ebx
  int v80; // ecx
  CRibbonVertex *v81; // edx
  int v82; // [esp+4h] [ebp-84h]
  int v83; // [esp+8h] [ebp-80h]
  int v84; // [esp+Ch] [ebp-7Ch]
  char *v85; // [esp+38h] [ebp-50h]
  char *v86; // [esp+40h] [ebp-48h]
  float *v87; // [esp+44h] [ebp-44h]
  char *v88; // [esp+4Ch] [ebp-3Ch]
  CRibbonVertex *v89; // [esp+50h] [ebp-38h]
  char *v90; // [esp+54h] [ebp-34h]
  unsigned int *v92; // [esp+60h] [ebp-28h]
  unsigned int v93; // [esp+64h] [ebp-24h]
  unsigned int v94; // [esp+68h] [ebp-20h]
  float edgesPerSeca; // [esp+6Ch] [ebp-1Ch]
  float edgeLifetimea; // [esp+98h] [ebp+10h]

  assert(this->m_ribbonEmitterflags.m_initialized == 0);
  assert(edgesPerSec >= 1.0f );
  assert(edgeLifeSpanInSec > 0.0 );

  edgesPerSeca = ceilf(edgesPerSec);
  edgeLifetimea = fmaxf(0.25, edgeLifeSpanInSec);
  v11 = ceilf(edgeLifetimea * edgesPerSeca);

  newEdgesCount = ceil(fmaxf((float)(v11 + 1.0) + 1.0, 0.0));
  this->m_edges = std::vector<float>(newEdgesCount, 0.0f);
  this->m_readPos = 0;
  this->m_writePos = 0;
  this->m_startTime = 0.0;
  this->m_ribbonEmitterflags.m_posSet = 0;
  this->m_gxVertices = std::vector<CRibbonVertex>(2 * newEdgesCount);
  for (int i = 0; i < this->m_gxVertices.size(); i++) {
    v20 = &this->m_gxVertices[i];
    v20->pos.x = 0.0;
    v20->pos.y = 0.0;
    v20->pos.z = 0.0;
    *(int *)&v20->diffuseColor = 0;
    v20->texCoord.x = 0.0;
    v20->texCoord.y = 0.0;
  }

  v21 = 4 * newEdgesCount;
  this->m_gxIndices = std::vector<uint16_t>(4 * newEdgesCount);
  for(int i = 0; i < this->m_gxIndices.size(); i++) {
    this->m_gxIndices[i] = static_cast<unsigned short>(i % (2 * newEdgesCount));
  }
  this->m_ooLifeSpan = 1.0f / edgeLifetimea;
  v26 = (float)(signed int)cols;
  if ( (cols & 0x80000000) != 0 )
    v26 = (float)((cols & 1) | (cols >> 1)) + (float)((cols & 1) | (cols >> 1));
  this->m_tmpDU = (float)(a8->maxx - a8->minx) / v26;
  v27 = (float)(signed int)rows;
  if ( (rows & 0x80000000) != 0 )
    v27 = (float)((rows & 1) | (rows >> 1)) + (float)((rows & 1) | (rows >> 1));

  this->m_tmpDV = (float)(a8->maxy - a8->miny) / v27;
  this->m_ooTmpDU = 1.0f / this->m_tmpDU;
  this->m_ooTmpDV = 1.0f / this->m_tmpDV;
  this->m_edgesPerSec = edgesPerSeca;
  this->m_edgeLifeSpan = edgeLifetimea;
  this->m_diffuseClr = diffuseColor;

  this->m_texBox = *a8;
  this->m_rows = rows;
  this->m_cols = cols;

  this->m_texSlot = 0;
  v45 = (float)(0.0 * this->m_tmpDU) + this->m_texBox.minx;
  this->m_texSlotBox.minx = v45;
  v46 = (float)(0.0 * this->m_tmpDV) + this->m_texBox.miny;
  this->m_texSlotBox.miny = v46;
  this->m_texSlotBox.maxx = v45 + this->m_tmpDU;
  this->m_texSlotBox.maxy = v46 + this->m_tmpDV;
  this->m_above = 10.0;
  this->m_below = 10.0;
  this->m_gravity = 0.0;
  this->m_ribbonEmitterflags.m_userEnabled = 1;
  this->m_ribbonEmitterflags.m_dataEnabled = 1;
  this->m_ribbonEmitterflags.m_initialized = 1;
}

//----- (00A1CD90) --------------------------------------------------------
//signed int CRibbonEmitter::Render(const mathfu::mat4 *a2)
//{
//  CGxDevice *v2; // ebx
//  int v3; // eax
//  int v4; // eax
//  _DWORD *v5; // edx
//  _DWORD *v6; // eax
//  int v7; // eax
//  _QWORD *v8; // eax
//  int v9; // ebx
//  int v10; // esi
//  unsigned int v11; // edx
//  unsigned int v12; // eax
//  int v13; // ebx
//  int v14; // eax
//  CGxDevice *v15; // esi
//  unsigned int v16; // edi
//  unsigned int i; // ebx
//  int v18; // ecx
//  int v19; // ebx
//  int v20; // eax
//  char *v21; // eax
//  int v22; // eax
//  int *v23; // edx
//  unsigned int v24; // esi
//  unsigned int v25; // esi
//  unsigned int v26; // esi
//  unsigned int v27; // esi
//  CGxDevice *v28; // edi
//  _DWORD *v29; // ebx
//  unsigned int v30; // esi
//  CGxDevice *v31; // edi
//  _DWORD *v32; // ebx
//  unsigned int v33; // esi
//  CGxDevice *v34; // edi
//  _DWORD *v35; // ebx
//  unsigned int v36; // esi
//  int v37; // eax
//  int v38; // ebx
//  CGxDevice *v39; // edx
//  int v40; // eax
//  int v42; // ebx
//  int v43; // eax
//  int v44; // ebx
//  int v45; // eax
//  int v46; // ebx
//  int v47; // eax
//  int v48; // ebx
//  int v49; // eax
//  int v50; // ebx
//  int v51; // eax
//  int v52; // ebx
//  int v53; // eax
//  int v54; // ebx
//  int v55; // eax
//  int v56; // esi
//  int v57; // eax
//  int v58; // esi
//  int v59; // eax
//  int v60; // esi
//  int v61; // eax
//  int v62; // ebx
//  int v63; // eax
//  unsigned int v64; // ebx
//  int v65; // edx
//  _DWORD *v66; // ecx
//  int v67; // [esp+4h] [ebp-E4h]
//  int v68; // [esp+8h] [ebp-E0h]
//  int v69; // [esp+Ch] [ebp-DCh]
//  char *v70; // [esp+2Ch] [ebp-BCh]
//  char *v71; // [esp+30h] [ebp-B8h]
//  unsigned __int8 v72; // [esp+36h] [ebp-B2h]
//  unsigned __int8 v73; // [esp+37h] [ebp-B1h]
//  int v74; // [esp+38h] [ebp-B0h]
//  int v75; // [esp+3Ch] [ebp-ACh]
//  unsigned int v76; // [esp+40h] [ebp-A8h]
//  int v77; // [esp+44h] [ebp-A4h]
//  char *v78; // [esp+48h] [ebp-A0h]
//  int v79; // [esp+50h] [ebp-98h]
//  unsigned int v80; // [esp+54h] [ebp-94h]
//  unsigned int v81; // [esp+58h] [ebp-90h]
//  unsigned int v82; // [esp+5Ch] [ebp-8Ch]
//  mathfu::mat4 v83; // [esp+60h] [ebp-88h]
//  int v84; // [esp+A0h] [ebp-48h]
//  int v85; // [esp+A4h] [ebp-44h]
//  int v86; // [esp+A8h] [ebp-40h]
//  int v87; // [esp+ACh] [ebp-3Ch]
//  int v88; // [esp+B0h] [ebp-38h]
//  int v89; // [esp+B4h] [ebp-34h]
//  int v90; // [esp+B8h] [ebp-30h]
//  int v91; // [esp+BCh] [ebp-2Ch]
//  int v92; // [esp+C0h] [ebp-28h]
//  int v93; // [esp+C4h] [ebp-24h]
//  int v94; // [esp+C8h] [ebp-20h]
//  __int16 v95; // [esp+CCh] [ebp-1Ch]
//  __int16 v96; // [esp+CEh] [ebp-1Ah]
//
//  if ( !(this->m_ribbonEmitterflags & m_initialized) )
//    SErrDisplayError(
//      -2062548992,
//      (int)"/Users/patchman/buildserver/wow-b/work/WoW-code/branches/wow-patch-4_1_0-branch/WoW/Source/Mac/../../../Engin"
//           "e/Source/Services/RibbonEmitter.cpp",
//      576,
//      "m_initialized",
//      0,
//      1,
//      0);
//  if ( !CRibbonEmitter::s_enableRibbonRendering || this->m_readPos == this->m_writePos )
//    return 0;
//  *(_QWORD *)&v83.m_elements[0][0] = 0x3F800000LL;
//  *(_QWORD *)&v83.m_elements[0][2] = 0LL;
//  *(_QWORD *)&v83.m_elements[1][0] = 0x3F80000000000000LL;
//  *(_QWORD *)&v83.m_elements[1][2] = 0LL;
//  *(_QWORD *)&v83.m_elements[2][0] = 0LL;
//  *(_QWORD *)&v83.m_elements[2][2] = 0x3F800000LL;
//  *(_QWORD *)&v83.m_elements[3][0] = 0LL;
//  *(_QWORD *)&v83.m_elements[3][2] = 0x3F80000000000000LL;
//  if ( a2 )
//  {
//    *(_QWORD *)&v83.m_elements[0][0] = *(_QWORD *)&a2->m_elements[0][0];
//    *(_QWORD *)&v83.m_elements[0][2] = *(_QWORD *)&a2->m_elements[0][2];
//    *(_QWORD *)&v83.m_elements[1][0] = *(_QWORD *)&a2->m_elements[1][0];
//    *(_QWORD *)&v83.m_elements[1][2] = *(_QWORD *)&a2->m_elements[1][2];
//    *(_QWORD *)&v83.m_elements[2][0] = *(_QWORD *)&a2->m_elements[2][0];
//    *(_QWORD *)&v83.m_elements[2][2] = *(_QWORD *)&a2->m_elements[2][2];
//    *(_QWORD *)&v83.m_elements[3][0] = *(_QWORD *)&a2->m_elements[3][0];
//    *(_QWORD *)&v83.m_elements[3][2] = *(_QWORD *)&a2->m_elements[3][2];
//  }
//  v83.m_elements[3][0] = v83.m_elements[3][0] - this->m_pos.x;
//  v83.m_elements[3][1] = v83.m_elements[3][1] - this->m_pos.y;
//  v83.m_elements[3][2] = v83.m_elements[3][2] - this->m_pos.z;
//  v2 = g_theGxDevicePtr;
//  v3 = 3;
//  if ( (unsigned int)(*(_DWORD *)&g_theGxDevicePtr->gap106C[2236] + 1) <= 3 )
//    v3 = *(_DWORD *)&g_theGxDevicePtr->gap106C[2236] + 1;
//  *(_DWORD *)&g_theGxDevicePtr->gap106C[2236] = v3;
//  v4 = v3 << 6;
//  v5 = (CGxDeviceVTable **)((char *)&v2->m_vTable + v4);
//  v6 = (_DWORD *)((char *)v2 + v4 - 64);
//  v5[1612] = v6[1612];
//  v5[1613] = v6[1613];
//  v5[1614] = v6[1614];
//  v5[1615] = v6[1615];
//  v5[1616] = v6[1616];
//  v5[1617] = v6[1617];
//  v5[1618] = v6[1618];
//  v5[1619] = v6[1619];
//  v5[1620] = v6[1620];
//  v5[1621] = v6[1621];
//  v5[1622] = v6[1622];
//  v5[1623] = v6[1623];
//  v5[1624] = v6[1624];
//  v5[1625] = v6[1625];
//  v5[1626] = v6[1626];
//  v5[1627] = v6[1627];
//  v7 = *(_DWORD *)&v2->gap106C[2236];
//  *(_DWORD *)&v2->gap106C[4 * v7 + 2500] = *(_DWORD *)&v2->gap106C[4 * v7 + 2496];
//  v2->gap106C[2240] = 1;
//  *(_DWORD *)&v2->gap106C[4 * v7 + 2500] &= 0xFFFFFFFE;
//  v8 = (_QWORD *)((char *)v2 + 64 * v7);
//  v8[806] = *(_QWORD *)&v83.m_elements[0][0];
//  v8[807] = *(_QWORD *)&v83.m_elements[0][2];
//  v8[808] = *(_QWORD *)&v83.m_elements[1][0];
//  v8[809] = *(_QWORD *)&v83.m_elements[1][2];
//  v8[810] = *(_QWORD *)&v83.m_elements[2][0];
//  v8[811] = *(_QWORD *)&v83.m_elements[2][2];
//  v8[812] = *(_QWORD *)&v83.m_elements[3][0];
//  v8[813] = *(_QWORD *)&v83.m_elements[3][2];
//  v9 = CGxDevice::BufStream((int)g_theGxDevicePtr, 0, 24, this->m_gxVertices.m_currentLength);
//  v10 = CGxDevice::BufStream((int)g_theGxDevicePtr, 1, 2, this->m_gxIndices.m_currentLength);
//  GxBufData(v9, (int)this->m_gxVertices.m_elementPointer, 24 * this->m_gxVertices.m_currentLength, 0);
//  ((void (__cdecl *)(CGxDevice *, int, signed int))g_theGxDevicePtr->m_vTable->PrimVertexPtr)(g_theGxDevicePtr, v9, 9);
//  v11 = this->m_readPos;
//  v12 = this->m_writePos;
//  if ( v11 >= v12 )
//    v12 += this->m_edges.m_currentLength;
//  v13 = 2 * (v12 - v11) + 2;
//  GxBufData(v10, this->m_gxIndices.m_elementPointer + 4 * v11, 2 * v13, 0);
//  CGxDevice::PrimIndexPtr((int)g_theGxDevicePtr, v10);
//  v14 = this->m_gxVertices.m_currentLength - 1;
//  v92 = 4;
//  v93 = 0;
//  v94 = v13;
//  v95 = 0;
//  v96 = v14;
//  v75 = this->m_materials.m_currentLength;
//  if ( v75 )
//  {
//    v76 = 0;
//    v71 = (char *)&this->m_materials;
//    v70 = (char *)&this->m_textures;
//    while ( 1 )
//    {
//      v15 = g_theGxDevicePtr;
//      v16 = g_theGxDevicePtr->field_32 + 1;
//      if ( v16 > g_theGxDevicePtr->field_28 )
//        break;
//LABEL_24:
//      v22 = v15->field_32;
//      v23 = (int *)(v15->field_36 + 4 * v22);
//      v15->field_32 = v22 + 1;
//      *v23 = v15->field_12;
//      v24 = this->m_materials.m_currentLength;
//      if ( v24 <= v76 )
//      {
//        v52 = (*(int (__cdecl **)(char *, int, int, int))(this->m_materials.vTable + 4))(v71, v67, v68, v69);
//        v53 = (*(int (__cdecl **)(char *))this->m_materials.vTable)(v71);
//        SErrDisplayErrorFmt(-2062548864, v53, v52, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v24);
//      }
//      v79 = 8 * v76;
//      if ( *(_BYTE *)(this->m_materials.m_elementPointer + 8 * v76) & 1 )
//      {
//        v84 = 0;
//        v85 = 0;
//        v86 = 0;
//        v87 = 0;
//        CShaderEffect::SetEmissive((int)&v84);
//      }
//      else
//      {
//        v88 = 1065353216;
//        v89 = 1065353216;
//        v90 = 1065353216;
//        v91 = 0;
//        CShaderEffect::SetEmissive((int)&v88);
//      }
//      if ( CRibbonEmitter::s_enableFog )
//      {
//        v25 = this->m_materials.m_currentLength;
//        if ( v25 <= v76 )
//        {
//          v54 = (*(int (__cdecl **)(char *, int, int, int))(this->m_materials.vTable + 4))(v71, v67, v68, v69);
//          v55 = (*(int (__cdecl **)(char *))this->m_materials.vTable)(v71);
//          SErrDisplayErrorFmt(-2062548864, v55, v54, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v25);
//        }
//        CShaderEffect::SetFogEnabled((CShaderEffect *)((*(_DWORD *)(this->m_materials.m_elementPointer + v79) >> 1) & 1));
//      }
//      v26 = this->m_materials.m_currentLength;
//      if ( v26 <= v76 )
//      {
//        v44 = (*(int (__cdecl **)(char *, int, int, int))(this->m_materials.vTable + 4))(v71, v67, v68, v69);
//        v45 = (*(int (__cdecl **)(char *))this->m_materials.vTable)(v71);
//        SErrDisplayErrorFmt(-2062548864, v45, v44, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v26);
//      }
//      CShaderEffect::SetLightingEnabled((CShaderEffect *)(*(_BYTE *)(this->m_materials.m_elementPointer + v79) & 1));
//      v27 = this->m_materials.m_currentLength;
//      if ( v27 <= v76 )
//      {
//        v42 = (*(int (__cdecl **)(char *, int, int, int))(this->m_materials.vTable + 4))(v71, v67, v68, v69);
//        v43 = (*(int (__cdecl **)(char *))this->m_materials.vTable)(v71);
//        SErrDisplayErrorFmt(-2062548864, v43, v42, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v27);
//      }
//      v72 = (*(_DWORD *)(this->m_materials.m_elementPointer + v79) >> 2) & 1;
//      v28 = g_theGxDevicePtr;
//      if ( g_theGxDevicePtr->field_4024 )
//      {
//        v80 = g_theGxDevicePtr->field_10040;
//        if ( v80 <= 0xF )
//        {
//          v58 = (*(int (__cdecl **)(int *, int, int, int))(g_theGxDevicePtr->field_10032 + 4))(
//                  &g_theGxDevicePtr->field_10032,
//                  v67,
//                  v68,
//                  v69);
//          v59 = (*(int (__cdecl **)(int *))v28->field_10032)(&v28->field_10032);
//          SErrDisplayErrorFmt(-2062548864, v59, v58, 1, 1, "index (0x%08X), array size (0x%08X)", 15, v80);
//        }
//        v29 = (_DWORD *)(v28->field_10044 + 360);
//        if ( v72 != *v29 )
//        {
//          CGxDevice::IRsDirty(v28, GxRs_DepthWriteMask);
//          *v29 = v72;
//        }
//      }
//      v30 = this->m_materials.m_currentLength;
//      if ( v30 <= v76 )
//      {
//        v48 = (*(int (__cdecl **)(char *, int, int, int))(this->m_materials.vTable + 4))(v71, v67, v68, v69);
//        v49 = (*(int (__cdecl **)(char *))this->m_materials.vTable)(v71);
//        SErrDisplayErrorFmt(-2062548864, v49, v48, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v30);
//      }
//      v73 = (*(_DWORD *)(this->m_materials.m_elementPointer + v79) >> 3) & 1;
//      v31 = g_theGxDevicePtr;
//      if ( g_theGxDevicePtr->field_4024 )
//      {
//        v81 = g_theGxDevicePtr->field_10040;
//        if ( v81 <= 0x11 )
//        {
//          v56 = (*(int (__cdecl **)(int *, int, int, int))(g_theGxDevicePtr->field_10032 + 4))(
//                  &g_theGxDevicePtr->field_10032,
//                  v67,
//                  v68,
//                  v69);
//          v57 = (*(int (__cdecl **)(int *))v31->field_10032)(&v31->field_10032);
//          SErrDisplayErrorFmt(-2062548864, v57, v56, 1, 1, "index (0x%08X), array size (0x%08X)", 17, v81);
//        }
//        v32 = (_DWORD *)(v31->field_10044 + 408);
//        if ( v73 != *v32 )
//        {
//          CGxDevice::IRsDirty(v31, GxRs_BackFaceCullMode);
//          *v32 = v73;
//        }
//      }
//      v33 = this->m_materials.m_currentLength;
//      if ( v33 <= v76 )
//      {
//        v46 = (*(int (__cdecl **)(char *, int, int, int))(this->m_materials.vTable + 4))(v71, v67, v68, v69);
//        v47 = (*(int (__cdecl **)(char *))this->m_materials.vTable)(v71);
//        SErrDisplayErrorFmt(-2062548864, v47, v46, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v33);
//      }
//      v74 = *(_DWORD *)(this->m_materials.m_elementPointer + v79 + 4);
//      v34 = g_theGxDevicePtr;
//      if ( g_theGxDevicePtr->field_4024 )
//      {
//        v82 = g_theGxDevicePtr->field_10040;
//        if ( v82 <= 6 )
//        {
//          v60 = (*(int (__cdecl **)(int *, int, int, int))(g_theGxDevicePtr->field_10032 + 4))(
//                  &g_theGxDevicePtr->field_10032,
//                  v67,
//                  v68,
//                  v69);
//          v61 = (*(int (__cdecl **)(int *))v34->field_10032)(&v34->field_10032);
//          SErrDisplayErrorFmt(-2062548864, v61, v60, 1, 1, "index (0x%08X), array size (0x%08X)", 6, v82);
//        }
//        v35 = (_DWORD *)(v34->field_10044 + 144);
//        if ( v74 != *v35 )
//        {
//          CGxDevice::IRsDirty(v34, GxRs_BlendMode);
//          *v35 = v74;
//        }
//      }
//      CShaderEffect::SetAlphaRefDefault();
//      v36 = this->m_textures.m_currentLength;
//      if ( v36 <= v76 )
//      {
//        v50 = (*(int (__cdecl **)(char *, int, int, int))(this->m_textures.vTable + 4))(v70, v67, v68, v69);
//        v51 = (*(int (__cdecl **)(char *))this->m_textures.vTable)(v70);
//        SErrDisplayErrorFmt(-2062548864, v51, v50, 1, 1, "index (0x%08X), array size (0x%08X)", v76, v36);
//      }
//      v37 = TextureGetGxTex(this->m_textures.m_elementPointer[v76]);
//      v38 = v37;
//      if ( v37 )
//      {
//        CGxDevice::RsSet(g_theGxDevicePtr, GxRs_BackFaceCullMode|GxRs_MatSpecularExp, v37);
//        GxTexSetWrap(v38, 0, 0);
//        CShaderEffect::SetDefaultShaders(0, 0);
//        CShaderEffect::UpdateWorldViewMatrix();
//        if ( !v94 )
//          SErrDisplayError(-2062548992, (int)"../../../Engine/Source/Gx/Gx.h", 2073, "batch.m_count > 0", 0, 1, 0);
//        ((void (__cdecl *)(CGxDevice *, int *))g_theGxDevicePtr->m_vTable->Draw)(g_theGxDevicePtr, &v92);
//      }
//      CGxDevice::RsPop(g_theGxDevicePtr);
//      if ( ++v76 == v75 )
//        goto LABEL_62;
//    }
//    i = g_theGxDevicePtr->field_40;
//    if ( !i )
//    {
//      if ( v16 > 0x3F )
//      {
//        g_theGxDevicePtr->field_40 = 64;
//        v18 = v16 & 0x3F;
//        i = 64;
//LABEL_21:
//        if ( v18 )
//          v16 = i + v16 - v18;
//        goto LABEL_23;
//      }
//      for ( i = g_theGxDevicePtr->field_32 + 1; i & (i - 1); i &= i - 1 )
//        ;
//      if ( !i )
//      {
//LABEL_23:
//        v77 = (int)&v15->field_24;
//        v78 = (char *)v15->field_36;
//        v15->field_28 = v16;
//        v19 = (*(int (__cdecl **)(int *, int, int, int))(v15->field_24 + 4))(&v15->field_24, v67, v68, v69);
//        v20 = (*(int (__cdecl **)(int *))v15->field_24)(&v15->field_24);
//        v21 = SMemReAlloc(v78, 4 * v16, v20, v19, 16);
//        v15->field_36 = (int)v21;
//        if ( !v21 )
//        {
//          v62 = (*(int (__cdecl **)(int))(v15->field_24 + 4))(v77);
//          v63 = (*(int (__cdecl **)(int))v15->field_24)(v77);
//          v15->field_36 = (int)SMemAlloc(4 * v16, v63, v62, 0);
//          if ( v78 )
//          {
//            v64 = v15->field_32;
//            if ( v16 <= v15->field_32 )
//              v64 = v16;
//            if ( v64 )
//            {
//              v65 = 0;
//              do
//              {
//                v66 = (_DWORD *)(v15->field_36 + 4 * v65);
//                if ( v66 )
//                  *v66 = *(_DWORD *)&v78[4 * v65];
//                ++v65;
//              }
//              while ( v65 != v64 );
//            }
//            v69 = 0;
//            v68 = (*(int (__cdecl **)(int))(v15->field_24 + 4))(v77);
//            v67 = (*(int (__cdecl **)(int))v15->field_24)(v77);
//            SMemFree((int)v78);
//          }
//        }
//        goto LABEL_24;
//      }
//    }
//    v18 = v16 % i;
//    goto LABEL_21;
//  }
//LABEL_62:
//  v39 = g_theGxDevicePtr;
//  v40 = *(_DWORD *)&g_theGxDevicePtr->gap106C[2236];
//  if ( v40 )
//    *(_DWORD *)&g_theGxDevicePtr->gap106C[2236] = v40 - 1;
//  v39->gap106C[2240] = 1;
//  return 1;
//}


