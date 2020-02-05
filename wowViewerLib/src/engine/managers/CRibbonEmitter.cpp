#include <iostream>
#include <cassert>
#include "CRibbonEmitter.h"
#include "../../gapi/interface/IVertexBufferBindings.h"
#include "../shader/ShaderDefinitions.h"
#include "../../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"

static GBufferBinding staticRibbonBindings[3] = {
    {+ribbonShader::Attribute::aPosition, 3, GBindingType::GFLOAT, false, 24, 0 }, // 0
    {+ribbonShader::Attribute::aColor, 4, GBindingType::GUNSIGNED_BYTE, true, 24, 12}, // 12
    {+ribbonShader::Attribute::aTexcoord0, 2, GBindingType::GFLOAT, false, 24, 16}, // 16
    //24
};

//----- (00A19710) --------------------------------------------------------
CRibbonEmitter::CRibbonEmitter(ApiContainer *api, M2Object *object, std::vector<M2Material> &materials, std::vector<int> &textureIndicies) : m_api(api)
{
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
  this->m_minWorldBounds.z = 3.4028235e10f;
  this->m_minWorldBounds.y = 3.4028235e10f;
  this->m_minWorldBounds.x = 3.4028235e10f;
  this->m_maxWorldBound.z = -3.4028235e10f;
  this->m_maxWorldBound.y = -3.4028235e10f;
  this->m_maxWorldBound.x = -3.4028235e10f;
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

//  std::cout << "sizeof(CRibbonVertex) = " << sizeof(CRibbonVertex) << std::endl;
//  std::cout << "offset(CRibbonVertex.diffuseColor) = " << sizeof(CRibbonVertex) << std::endl;
//  std::cout << "offset(CRibbonVertex.texCoord) = " << sizeof(CRibbonVertex) << std::endl;

  check_offset<offsetof(CRibbonVertex, diffuseColor), 12>();
  check_offset<offsetof(CRibbonVertex, texCoord), 16>();

  createMesh(object, materials, textureIndicies);
}
PACK(
    struct meshParticleWideBlockPS {
        float uAlphaTest;
        float padding[3]; // according to std140
        int uPixelShader;
        float padding2[3];
    });

extern EGxBlendEnum M2BlendingModeToEGxBlendEnum [8];
void CRibbonEmitter::createMesh(M2Object *m2Object, std::vector<M2Material> &materials, std::vector<int> &textureIndicies) {
  auto device = m_api->hDevice;

  //Create Buffers
  for (int k = 0; k < 4; k++) {
    frame[k].m_indexVBO = device->createIndexBuffer();
    frame[k].m_bufferVBO = device->createVertexBuffer();

    frame[k].m_bindings = device->createVertexBufferBindings();
    frame[k].m_bindings->setIndexBuffer(frame[k].m_indexVBO);

    GVertexBufferBinding vertexBinding;
    vertexBinding.vertexBuffer = frame[k].m_bufferVBO;
    vertexBinding.bindings = std::vector<GBufferBinding>(&staticRibbonBindings[0],&staticRibbonBindings[3]);

    frame[k].m_bindings->addVertexBufferBinding(vertexBinding);
    frame[k].m_bindings->save();


    //Get shader
    for(int i = 0; i < materials.size(); i++) {
        auto &material = materials[i];
        auto &textureIndex = textureIndicies[i];

        HGShaderPermutation shaderPermutation = device->getShader("ribbonShader", nullptr);

        //Create mesh
        gMeshTemplate meshTemplate(frame[k].m_bindings, shaderPermutation);

        meshTemplate.depthWrite = !(material.flags & 0x10);
        meshTemplate.depthCulling = !(material.flags & 0x8);
        meshTemplate.backFaceCulling = !(material.flags & 0x4);

        meshTemplate.blendMode = M2BlendingModeToEGxBlendEnum[material.blending_mode];


        meshTemplate.start = 0;
        meshTemplate.end = 0;
        meshTemplate.element = DrawElementMode::TRIANGLE_STRIP;


        meshTemplate.textureCount = 1;
        meshTemplate.texture = std::vector<HGTexture>(1, nullptr);
        HBlpTexture tex0 = m2Object->getBlpTextureData(textureIndicies[i]);
        meshTemplate.texture[0] = device->createBlpTexture(tex0, true, true);

        meshTemplate.ubo[0] = nullptr; //m_api->getSceneWideUniformBuffer();
        meshTemplate.ubo[1] = nullptr;
        meshTemplate.ubo[2] = nullptr;

        meshTemplate.ubo[3] = nullptr;
        meshTemplate.ubo[4] = device->createUniformBufferChunk(sizeof(meshParticleWideBlockPS));

        meshTemplate.ubo[4]->setUpdateHandler([](IUniformBufferChunk *self) {
            meshParticleWideBlockPS& blockPS = self->getObject<meshParticleWideBlockPS>();

            blockPS.uAlphaTest = -1.0f;
            blockPS.uPixelShader = 0;
        });

        
        frame[k].m_meshes.push_back(device->createParticleMesh(meshTemplate));
    }
  }
}

//----- (00A199E0) --------------------------------------------------------
void CRibbonEmitter::SetDataEnabled(char a2)
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
CRibbonEmitter * CRibbonEmitter::SetGravity(float a2)
{
  CRibbonEmitter *result; // eax

  result = this;
  this->m_gravity = a2;
  return result;
}

//----- (00A19A80) --------------------------------------------------------
bool CRibbonEmitter::IsDead() const
{
  return this->m_readPos == this->m_writePos;
}

//----- (00A19AA0) --------------------------------------------------------
void CRibbonEmitter::SetBelow(float below)
{
//  assert(below >= 0.0f);
  this->m_below = below;
}

//----- (00A19B10) --------------------------------------------------------
void CRibbonEmitter::SetAbove(float above)
{
//    assert(above >= 0.0f);
    this->m_above = above;
}

//----- (00A19B80) --------------------------------------------------------
void CRibbonEmitter::SetAlpha(float a2)
{
  this->m_diffuseClr.a = fmaxf(255.0f * a2, 0.0f);
}


//----- (00A19C50) --------------------------------------------------------
void  CRibbonEmitter::InitInterpDeltas()
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
unsigned int  CRibbonEmitter::GetNumPrimitives()
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

      result->pos = (*frameOfReference * mathfu::vec4(mathfu::vec3(result->pos), 1.0f)).xyz();
      ++v16;
      ++v17;
    }
    while ( this->m_gxVertices.size() > v16 );
  }
}

//----- (00A1A4E0) --------------------------------------------------------
void CRibbonEmitter::SetColor(float a3, float a4, float a5)
{
    this->m_diffuseClr.r = a5 * 255.0f;
    this->m_diffuseClr.g = a4 * 255.0f;
    this->m_diffuseClr.b = a3 * 255.0f;

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
  int j; // eax
  unsigned int v9; // esi
  CRibbonVertex *v10; // esi
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
 float v39; // ST50_4

  float i; // xmm0_4
  unsigned int v45; // esi
  float v50; // ST48_4
  CRibbonVertex *v51; // eax
  float v52; // ecx
  unsigned int v53; // esi
  CRibbonVertex *v54; // eax
  float v55; // ecx
  unsigned int v56; // esi
  unsigned int v70; // [esp+3Ch] [ebp-ACh]
  int v71; // [esp+40h] [ebp-A8h]
  int v72; // [esp+44h] [ebp-A4h]
  int v73; // [esp+4Ch] [ebp-9Ch]
  float v76; // [esp+70h] [ebp-78h]
  float v77; // [esp+74h] [ebp-74h]
  float ooDenom; // [esp+78h] [ebp-70h]
  int v79; // [esp+7Ch] [ebp-6Ch]
  unsigned int v80; // [esp+84h] [ebp-64h]
  unsigned int v81; // [esp+84h] [ebp-64h]
  CRibbonVertex *v82; // [esp+88h] [ebp-60h]
  unsigned int v84; // [esp+90h] [ebp-58h]
  unsigned int v85; // [esp+94h] [ebp-54h]
  unsigned int v86; // [esp+98h] [ebp-50h]
  unsigned int v87; // [esp+9Ch] [ebp-4Ch]
  unsigned int v88; // [esp+A0h] [ebp-48h]
  unsigned int v89; // [esp+A4h] [ebp-44h]
  unsigned int v91; // [esp+ACh] [ebp-3Ch]
  unsigned int v92; // [esp+B0h] [ebp-38h]
  unsigned int v93; // [esp+B4h] [ebp-34h]
  float v94; // [esp+B8h] [ebp-30h]
  CImVector v95; // [esp+CCh] [ebp-1Ch]

  assert(this->m_ribbonEmitterflags.m_initialized);

  if ( !(this->m_ribbonEmitterflags.m_singletonUpdated) )
  {
    if ( this->m_edgesPerSec > 0.0 )
      deltaTime = (float)(1.0 / this->m_edgesPerSec) + 0.000099999997f;
  }
  if ( deltaTime >= 0.0 )
  {
    if ( this->m_edgeLifeSpan <= deltaTime )
    {
      deltaTime = this->m_edgeLifeSpan;
    }
  }
  else
  {
    deltaTime = 0.0;
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
    }
    else
    {
      v77 = 1.0;
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
  this->m_minWorldBounds.z = 3.4028235e37f;
  this->m_minWorldBounds.y = 3.4028235e37f;
  this->m_minWorldBounds.x = 3.4028235e37f;
  this->m_maxWorldBound.z = -3.4028235e37f;
  this->m_maxWorldBound.y = -3.4028235e37f;
  this->m_maxWorldBound.x = -3.4028235e37f;
  v80 = this->m_readPos;
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
  CRibbonVertex *v20; // edx
  float v26; // xmm1_4
  float v27; // xmm1_4

  float v45; // xmm1_4
  float v46; // xmm0_4

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
  for (auto &m_gxVertice : this->m_gxVertices) {
    v20 = &m_gxVertice;
    v20->pos = mathfu::vec3(0.0, 0.0, 0.0);
    *(int *)&v20->diffuseColor = 0;
    v20->texCoord.x = 0.0;
    v20->texCoord.y = 0.0;
  }

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

void CRibbonEmitter::collectMeshes(std::vector<HGMesh> &meshes, int renderOrder) {

    auto &currFrame = frame[m_api->hDevice->getUpdateFrameNumber()];
    if (currFrame.isDead) return;

    for (int i = 0; i < currFrame.m_meshes.size(); i++) {
        auto mesh = currFrame.m_meshes[i];

        mesh->setRenderOrder(renderOrder);
        meshes.push_back(mesh);
    }
}

void CRibbonEmitter::updateBuffers() {
//    return;

  auto &currentFrame = frame[m_api->hDevice->getUpdateFrameNumber()];
  currentFrame.isDead = this->IsDead();
  if (currentFrame.isDead) return;

  currentFrame.m_indexVBO->uploadData((void *) m_gxIndices.data(), (int) (m_gxIndices.size() * sizeof(uint16_t)));
  currentFrame.m_bufferVBO->uploadData((void *) m_gxVertices.data(), (int) (m_gxVertices.size() * sizeof(CRibbonVertex)));

  int indexCount = m_writePos > m_readPos ?
      2 *(m_writePos - m_readPos) + 2:
      2 *((int)m_edges.size() + m_writePos - m_readPos) + 2;
    for (auto mesh : currentFrame.m_meshes) {
        mesh->setStart(2 * m_readPos * sizeof(uint16_t));
        mesh->setEnd(indexCount);
        mesh->setSortDistance(0);
        mesh->setPriorityPlane(this->m_priority);
    }

}