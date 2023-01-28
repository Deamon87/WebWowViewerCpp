//
// Created by deamon on 15.01.20.
//

#ifndef AWEBWOWVIEWERCPP_SCENESCENARIO_H
#define AWEBWOWVIEWERCPP_SCENESCENARIO_H

#include <memory>
#include "../IRenderParameters.h"
#include "../../gapi/interface/IDevice.h"


struct CameraMatrices;

//Holds dependency graph for different scenes
class FrameScenario;

#include <vector>
class SceneComposer;

#include "../IRenderer.h"

class FrameScenarioBuilder {
private:
    template<typename T>
    class RendererSpecificData {
    private:
        FrameScenarioBuilder &m_builder;
    public:
        RendererSpecificData(FrameScenarioBuilder &builder) : m_builder(builder) {
        }
    };

public:
    template <typename T, typename R, typename = std::enable_if_t<std::is_base_of_v<IRendererParameters<T, R>, T>>>
    RendererSpecificData<T> withSceneRenderer(T renderer) {
        return RendererSpecificData<T>(this, renderer);
    }
};

typedef std::function<void(HGDevice &device)> SceneUpdateRenderLambda;
typedef std::function<SceneUpdateRenderLambda()> CullLambda;

struct FrameScenario {
    std::vector<CullLambda> cullFunctions;
    std::vector<SceneUpdateRenderLambda> drawUpdateFunction;
};
typedef std::shared_ptr<FrameScenario> HFrameScenario;


#endif //AWEBWOWVIEWERCPP_SCENESCENARIO_H
