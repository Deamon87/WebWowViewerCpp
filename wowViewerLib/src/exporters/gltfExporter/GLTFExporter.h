//
// Created by Deamon on 12/22/2020.
//

#ifndef AWEBWOWVIEWERCPP_GLTFEXPORTER_H
#define AWEBWOWVIEWERCPP_GLTFEXPORTER_H


#include <mathfu/glsl_mappings.h>
#include "../IExporter.h"
#include "../../../3rdparty/tinygltf/tiny_gltf.h"

class GLTFExporter : public IExporter {
private:
    tinygltf::Model model;

    int VBOBufferIndex = -1;
    int IBOBufferIndex = -1;

    int iboSize;

    int VBOBufferViewIndex = -1;
    int IBOBufferViewIndex = -1;

    int accesorPosIndex = -1;
    int accesorNormalIndex = -1;
    int accesorTexCoordIndex = -1;
    int accesorJointsIndex = -1;
    int accesorWeightsIndex = -1;

    int IBOAccessorsStart = -1;

    //Bones
    int skeletonIndex;

    std::vector<int> boneStartIndexes;
    std::vector<int> boneEndIndexes;
    std::vector<mathfu::mat4> inverBindMatrices;

    template<typename N>
    void addTrack(tinygltf::Animation &animation, M2Track<N> &track, int animIndex, float animationLength,
                  std::string trackName, int nodeIndex, int animationBufferIndex);

public:
    GLTFExporter();

    void addM2Object(std::shared_ptr<M2Object> &m2Object) override;

    void createVBOAndIBOViews(std::shared_ptr<M2Object> &m2Object) ;

    void createVboAndIbo(std::shared_ptr<M2Object> &m2Object) ;

    void createAttributesForVBO(std::shared_ptr<M2Object> &m2Object);

    void createIBOAccessors(std::shared_ptr<M2Object> &m2Object);

    void createGlTFBatches(std::shared_ptr<M2Object> &m2Object, tinygltf::Mesh &mesh) ;

    void createSkeleton(std::shared_ptr<M2Object> &m2Object, tinygltf::Skin &skin);
};


#endif //AWEBWOWVIEWERCPP_GLTFEXPORTER_H
