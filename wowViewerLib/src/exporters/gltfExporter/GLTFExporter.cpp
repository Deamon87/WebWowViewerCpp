//
// Created by Deamon on 12/22/2020.
//

#include "GLTFExporter.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

//#define TINYGLTF_NO_STB_IMAGE_WRITE

#ifndef TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include "../../engine/algorithms/mathHelper.h"
#include "../../../../../3rdparty/tinygltf/tiny_gltf.h"
#include "../../engine/ApiContainer.h"
#include "../../engine/objects/m2/m2Object.h"

void GLTFExporter::addM2Object(std::shared_ptr<M2Object> &m2Object) {
    tinygltf::TinyGLTF loader;

    //1. Create Buffers
    createVboAndIbo(m2Object);

    //2. Create buffer views
    createVBOAndIBOViews(m2Object);

    //2. Create accesors
    //2.1 Accessors for VBO:
    createAttributesForVBO(m2Object);

    //Primitives
    tinygltf::Mesh mesh;
    {
        createIBOAccessors(m2Object);
        createGlTFBatches(m2Object, mesh);
    }

    model.meshes.push_back(mesh);

    //Add skin (armature of bones) to model
    {
        tinygltf::Skin skin;
        createSkeleton(m2Object, skin);
        model.skins.push_back(skin);
    }

    //Export current animation
    {
        std::vector<char> animationBuffer;

        auto animIndex = m2Object->getCurrentAnimationIndex();
//        for (int i = 0;)
    }


    //Add scene
    int rootSceneNodeIndex = model.nodes.size();
    {
        tinygltf::Node node;
        node.mesh = 0;
        node.skin = 0;
        node.name = "Fox";

        model.nodes.push_back(node);
    }

    {
        tinygltf::Scene scene;
        scene.name = "scene";
        scene.nodes = {rootSceneNodeIndex, skeletonIndex};
        model.scenes.push_back(scene);
    }
    model.defaultScene = 0;

    loader.WriteGltfSceneToFile(&model, "gltf/test.gltf");
}

void GLTFExporter::createSkeleton(std::shared_ptr<M2Object> &m2Object, tinygltf::Skin &skin) {
    auto &bones = * getBoneMasterData(m2Object)->getSkelData()->m_m2CompBones;

    //Create skeleton
    skeletonIndex = model.nodes.size();
    {
        {
            tinygltf::Node node;
            node.mesh = -1;
            node.skin = -1;
            node.name = "Armature";
            auto quat = mathfu::quat::FromMatrix(MathHelper::RotationX(-M_PI_2));
            node.rotation = {quat[1], quat[2], quat[3], quat[0]};

            model.nodes.push_back(node);
        }
        skin.skeleton = skeletonIndex;
    }
    for (int i = 0; i < bones.size; i++) {

        auto bone = bones[i];
        mathfu::vec3 relativePivot = mathfu::vec3(bone->pivot);
        if (bone->parent_bone>-1) {
            relativePivot = mathfu::vec3(bone->pivot) - mathfu::vec3(bones[bone->parent_bone]->pivot);
        }

        int bonePrefixNodeIndex = model.nodes.size();
        {
            tinygltf::Node node;
            node.mesh = -1;
            node.skin = -1;
            node.name = "bone " + std::to_string(i) + " prefix";
            node.translation = {
                relativePivot.x,
                relativePivot.y,
                relativePivot.z,
            };
            model.nodes.push_back(node);
            boneStartIndexes.push_back(bonePrefixNodeIndex);
        }
        int boneNodeIndex = model.nodes.size();
        {
            model.nodes[bonePrefixNodeIndex].children.push_back(boneNodeIndex);

            tinygltf::Node node;
            node.mesh = -1;
            node.skin = -1;
            node.name = "bone " + std::to_string(i);
            model.nodes.push_back(node);

            boneEndIndexes.push_back(boneNodeIndex);
            skin.joints.push_back(boneNodeIndex);
        }

        if (bone->parent_bone>-1) {
            model.nodes[boneEndIndexes[bone->parent_bone]].children.push_back(bonePrefixNodeIndex);
        } else {
            model.nodes[skeletonIndex].children.push_back(bonePrefixNodeIndex);
        }

        inverBindMatrices.push_back(mathfu::mat4::FromTranslationVector(-mathfu::vec3(bone->pivot)));
    }
    {
        //Create inverseMat buffer
        int inverseMatBufferIndex = model.buffers.size();
        {
            tinygltf::Buffer inverseMatBuffer;
            inverseMatBuffer.name = "inverseMatBuffer";

            inverseMatBuffer.data = std::__1::vector<uint8_t>((uint8_t *) inverBindMatrices.data(),
                                                              (uint8_t *) (inverBindMatrices.data() +
                                                                           inverBindMatrices.size()));

            model.buffers.push_back(inverseMatBuffer);
        }
        //Create inverseMat bufferView
        int inverseMatBufferViewIndex = model.bufferViews.size();
        {
            tinygltf::BufferView inverseMatBufferView;
            inverseMatBufferView.buffer = inverseMatBufferIndex;
            inverseMatBufferView.byteOffset = 0;
            inverseMatBufferView.byteLength = model.buffers[inverseMatBufferIndex].data.size();
            inverseMatBufferView.byteStride = 0;
            inverseMatBufferView.target = 0;
            model.bufferViews.push_back(inverseMatBufferView);
        }
        //Create inverseMat accessor
        {
            int accesorInverseMatIndex = model.accessors.size();

            tinygltf::Accessor a_inverseMat;
            a_inverseMat.bufferView = inverseMatBufferViewIndex;
            a_inverseMat.name = "inverseMatBuffer";
            a_inverseMat.byteOffset = 0;
            a_inverseMat.normalized = false;
            a_inverseMat.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            a_inverseMat.count = inverBindMatrices.size();
            a_inverseMat.type = TINYGLTF_TYPE_MAT4;

            model.accessors.push_back(a_inverseMat);
            skin.inverseBindMatrices = accesorInverseMatIndex;
        }
    }
}

void GLTFExporter::createGlTFBatches(std::shared_ptr<M2Object> &m2Object, tinygltf::Mesh mesh) {
    auto skinProfile = getSkinGeom(m2Object)->getSkinData();
    M2Array<M2Batch> &batches = skinProfile->batches;
    for (int i = 0; i < batches.size; i++) {
        auto m2Batch = batches[i];

        tinygltf::Primitive p;
        p.material = -1;
        p.attributes["POSITION"] = accesorPosIndex;
        p.attributes["NORMAL"] = accesorNormalIndex;
        p.attributes["TEXCOORD_0"] = accesorTexCoordIndex;
        p.attributes["JOINTS_0"] = accesorJointsIndex;
        p.attributes["WEIGHTS_0"] = accesorWeightsIndex;
        p.indices = m2Batch->skinSectionIndex + IBOAccessorsStart;
        p.mode = TINYGLTF_MODE_TRIANGLES;

        mesh.primitives.push_back(p);
    }
}

void GLTFExporter::createIBOAccessors(std::shared_ptr<M2Object> &m2Object) {
    auto skinProfile = getSkinGeom(m2Object)->getSkinData();
    auto sections = skinProfile->skinSections;

    IBOAccessorsStart =  model.accessors.size();

    for (int i = 0; i < sections.size; i++) {
        auto skinSection = skinProfile->skinSections[i];

        //Create accessor for IBO
        tinygltf::Accessor a_primitive;
        a_primitive.bufferView = IBOBufferViewIndex;
        a_primitive.name = "";
        a_primitive.byteOffset = (skinSection->indexStart + (skinSection->Level << 16)) * 2 ;
        a_primitive.normalized = false;
        a_primitive.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        a_primitive.count = skinSection->indexCount;
        a_primitive.type = TINYGLTF_TYPE_SCALAR;

        assert(a_primitive.byteOffset <= iboSize);
        assert((a_primitive.byteOffset+a_primitive.count) <= iboSize);

        model.accessors.push_back(a_primitive);
    }
}

void GLTFExporter::createAttributesForVBO(std::shared_ptr<M2Object> &m2Object) {
    auto m2Data = getM2Geom(m2Object)->getM2Data();
    //Position
    {
        accesorPosIndex = model.accessors.size();

        tinygltf::Accessor a_position;
        a_position.bufferView = VBOBufferViewIndex;
        a_position.name = "POSITION";
        a_position.byteOffset = 0;
        a_position.normalized = false;
        a_position.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        a_position.count = m2Data->vertices.size;
        a_position.type = TINYGLTF_TYPE_VEC3;

        model.accessors.push_back(a_position);
    }
    //Weights
    {
        accesorWeightsIndex = model.accessors.size();

        tinygltf::Accessor a_weights;
        a_weights.bufferView = VBOBufferViewIndex;
        a_weights.name = "WEIGHTS_0";
        a_weights.byteOffset = 12;
        a_weights.normalized = true;
        a_weights.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        a_weights.count = m2Data->vertices.size;
        a_weights.type = TINYGLTF_TYPE_VEC4;

        model.accessors.push_back(a_weights);
    }
    //Joints (bone indices)
    {
        accesorJointsIndex = model.accessors.size();

        tinygltf::Accessor a_weights;
        a_weights.bufferView = VBOBufferViewIndex;
        a_weights.name = "JOINTS_0";
        a_weights.byteOffset = 16;
        a_weights.normalized = false;
        a_weights.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        a_weights.count = m2Data->vertices.size;
        a_weights.type = TINYGLTF_TYPE_VEC4;

        model.accessors.push_back(a_weights);
    }
    //Normal
    {
        accesorNormalIndex = model.accessors.size();

        tinygltf::Accessor a_normal;
        a_normal.bufferView = VBOBufferViewIndex;
        a_normal.name = "NORMAL";
        a_normal.byteOffset = 20;
        a_normal.normalized = false;
        a_normal.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        a_normal.count = m2Data->vertices.size;
        a_normal.type = TINYGLTF_TYPE_VEC3;

        model.accessors.push_back(a_normal);
    }
    //aTexCoord
    {
        accesorTexCoordIndex = model.accessors.size();

        tinygltf::Accessor a_texturecoord;
        a_texturecoord.bufferView = VBOBufferViewIndex;
        a_texturecoord.name = "TEXCOORD_0";
        a_texturecoord.byteOffset = 32;
        a_texturecoord.normalized = false;
        a_texturecoord.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        a_texturecoord.count = m2Data->vertices.size;
        a_texturecoord.type = TINYGLTF_TYPE_VEC2;

        model.accessors.push_back(a_texturecoord);
    }
}

GLTFExporter::GLTFExporter() {
    model.asset.version = "2.0";
}

void GLTFExporter::createVboAndIbo(std::shared_ptr<M2Object> &m2Object) {
    auto m2Data = getM2Geom(m2Object)->getM2Data();
    {
        //1. Create VBO
        VBOBufferIndex = model.buffers.size();
        {
            tinygltf::Buffer modelVBO;
            modelVBO.name = "vbo";

            auto firstVertex = m2Data->vertices.getElement(0);
            modelVBO.data = std::__1::vector<uint8_t>((uint8_t *) firstVertex,
                                                      (uint8_t *) (firstVertex + m2Data->vertices.size));

            model.buffers.push_back(modelVBO);
        }

        //1.1 Create IBO
        IBOBufferIndex = model.buffers.size();
        {
            tinygltf::Buffer modelIBO;
            modelIBO.name = "ibo";

            auto indicies = getSkinGeom(m2Object)->generateIndexBuffer();

            modelIBO.data = std::__1::vector<uint8_t>((uint8_t *) indicies.data(),
                                                      (uint8_t *) (indicies.data() + indicies.size()));

            iboSize = modelIBO.data.size();

            model.buffers.push_back(modelIBO);
        }
    }
}

void GLTFExporter::createVBOAndIBOViews(std::shared_ptr<M2Object> &m2Object) {
    {
        //2.1 Create VBO buffer view
        VBOBufferViewIndex = model.bufferViews.size();
        {
            tinygltf::BufferView modelVBOView;
            modelVBOView.buffer = VBOBufferIndex;
            modelVBOView.byteOffset = 0;
            modelVBOView.byteLength = getM2Geom(m2Object)->getM2Data()->vertices.size * sizeof(M2Vertex);
            modelVBOView.byteStride = 48;
            modelVBOView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
            model.bufferViews.push_back(modelVBOView);
        }
        //2.1 Create IBO buffer view
        IBOBufferViewIndex = model.bufferViews.size();
        {
            tinygltf::BufferView modelIBOView;
            modelIBOView.buffer = IBOBufferIndex;
            modelIBOView.byteOffset = 0;
            modelIBOView.byteLength = model.buffers[IBOBufferIndex].data.size();
            modelIBOView.byteStride = 1;
            modelIBOView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
            model.bufferViews.push_back(modelIBOView);
        }
    }
}
