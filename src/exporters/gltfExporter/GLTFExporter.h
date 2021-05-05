//
// Created by Deamon on 12/22/2020.
//

#ifndef AWEBWOWVIEWERCPP_GLTFEXPORTER_H
#define AWEBWOWVIEWERCPP_GLTFEXPORTER_H


#include <mathfu/glsl_mappings.h>
#include "../../../wowViewerLib/src/exporters/IExporter.h"
#include "../../../3rdparty/tinygltf/tiny_gltf.h"

class GLTFExporter : public IExporter {
private:
    tinygltf::Model model;

    std::vector<std::shared_ptr<M2Object>> m2sToExport;

    template<typename N>
    void addTrack(tinygltf::Animation &animation, M2Track<N> &track, int animIndex, float animationLength,
                  std::string trackName, int nodeIndex, int animationBufferIndex, M2Array<M2Loop> &globalSequences);

    struct M2ModelData {
        int VBOBufferIndex = -1;
        int IBOBufferIndex = -1;

        int iboSize;

        int VBOBufferViewIndex = -1;
        int IBOBufferViewIndex = -1;

        int accesorPosIndex = -1;
        int accesorNormalIndex = -1;
        int accesorTexCoordIndex = -1;
        int accesorTexCoordIndex2 = -1;
        int accesorJointsIndex = -1;
        int accesorWeightsIndex = -1;

        int IBOAccessorsStart = -1;

        int textureIndexStart = -1;

        //Bones
        int skeletonIndex;

        std::vector<int> boneStartIndexes;
        std::vector<int> boneEndIndexes;
        std::vector<mathfu::mat4> inverBindMatrices;
    };
    std::string m_folderPath;

public:
    GLTFExporter(std::string folderPath);

    void addM2Object(std::shared_ptr<M2Object> &m2Object) override;
    void exportM2Object(std::shared_ptr<M2Object> &m2Object);
    void saveToFile(std::string filePath) override;

    void createVBOAndIBOViews(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object) ;

    void createVboAndIbo(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object) ;

    void createAttributesForVBO(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object);

    void createIBOAccessors(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object);

    void createGlTFBatches(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object, tinygltf::Mesh &mesh) ;

    void createSkeleton(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object, tinygltf::Skin &skin);

    void decodeTextureAndSaveToFile(HBlpTexture textureData, const std::string &texturePath);

    void createTextures(M2ModelData &m2ModelData, std::shared_ptr<M2Object> &m2Object);
};


#endif //AWEBWOWVIEWERCPP_GLTFEXPORTER_H
