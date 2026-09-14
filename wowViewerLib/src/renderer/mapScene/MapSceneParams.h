//
// Created by Deamon on 08.01.23.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
#define AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H

#include <list>
#include <memory>
#include <functional>
#include <optional>
#include <array>
#include "../../engine/CameraMatrices.h"
#include "../../engine/objects/iScene.h"

class IRenderView;

class IVideoRecordingContext {
public:
    virtual ~IVideoRecordingContext() = default;
    // Interface methods for video recording - no GAPI-specific data
    virtual uint32_t getWidth() const = 0;  // Video output width
    virtual uint32_t getHeight() const = 0; // Video output height
    virtual uint32_t getFramebufferWidth() const = 0;  // Framebuffer width (may differ from output)
    virtual uint32_t getFramebufferHeight() const = 0; // Framebuffer height (may differ from output)
    virtual std::shared_ptr<IRenderView> getRenderView() const = 0;
};

class IRenderView : public std::enable_shared_from_this<IRenderView> {
public:
    virtual ~IRenderView() = default;
    virtual void iterateOverOutputTextures(std::function<void (const std::array<std::shared_ptr<ISamplableTexture>, IDevice::MAX_FRAMES_IN_FLIGHT> &textures, const std::string &name, ITextureFormat textureFormat)> callback) = 0;
    virtual void readRGBAPixels(int frameNumber, int x, int y, int width, int height, void *outputdata) = 0;
    
    // Create a video recording context. Returns nullptr if not supported or if creation fails.
    // framebufferWidth/Height: size of the framebuffer to render to
    // outputWidth/Height: size of the final video output (will resize if different from framebuffer)
    virtual std::shared_ptr<IVideoRecordingContext> createVideoRecordingContext(uint32_t framebufferWidth, uint32_t framebufferHeight, uint32_t outputWidth, uint32_t outputHeight, const std::string &outputFilename) = 0;
    
    // Feed a frame to the video recording context. frameNumber is the current frame number.
    virtual void feedFrameToVideoRecording(std::shared_ptr<IVideoRecordingContext> context, int frameNumber) = 0;

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

    // Object-id pick request for this frame, in the first render target's pixel space. Cleared after consumption.
    std::optional<std::array<int, 2>> pickRequestPixel;
    // If true, this pick request only updates MapRenderPlan::hasHoveredM2/hoveredWMO (not the
    // actual selection / selection AABB highlight) — used for passive hover peeks.
    bool pickIsHoverPeek = false;
};

typedef std::shared_ptr<MapSceneParams> HMapSceneParams;
#endif //AWEBWOWVIEWERCPP_MAPSCENEPARAMS_H
