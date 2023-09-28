//
// Created by Deamon on 08.01.23.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
#define AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H

#include <list>
#include "../../engine/CameraMatrices.h"
#include "../../engine/objects/iScene.h"

class IRenderView {
public:
    virtual ~IRenderView() = default;
    virtual void iterateOverOutputTextures(std::function<void (const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures, const std::string &name, ITextureFormat textureFormat)> callback) = 0;


    std::unique_ptr<std::list<std::function<void()>>::const_iterator> addOnUpdate(std::function<void ()> callback) {
        m_onHandleChangeList.push_back(callback);
        return std::make_unique<std::list<std::function<void()>>::const_iterator>(
                std::prev(m_onHandleChangeList.end())
        );
    };
    void eraseOnUpdate(std::unique_ptr<std::list<std::function<void()>>::const_iterator> &iterator) {
        m_onHandleChangeList.erase(*iterator);
        iterator = nullptr;
    };
protected:
    void executeOnChange() {
        for (auto &callBack : m_onHandleChangeList) {
            callBack();
        }
    }
private:
    std::list<std::function<void()>> m_onHandleChangeList;
};

struct MapSceneParams {
    std::shared_ptr<IScene> scene;
    HCameraMatrices matricesForCulling;

    struct RenderTuple {
        HCameraMatrices cameraMatricesForRendering = nullptr;
        std::shared_ptr<IRenderView> target = nullptr;
        ViewPortDimensions viewPortDimensions = {{0,0}, {64, 64}};
        bool clearTarget = false;
    };
    std::vector<RenderTuple> renderTargets;

    mathfu::vec4 clearColor;
};

typedef std::shared_ptr<MapSceneParams> HMapSceneParams;
#endif //AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
