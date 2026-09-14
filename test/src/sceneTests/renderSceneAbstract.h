//
// Created by Deamon on 10/8/2025.
//

#ifndef WOWSTUDIO_RENDERSCENEABSTRACT_H
#define WOWSTUDIO_RENDERSCENEABSTRACT_H

static const std::string testResultFolder = "testResults";

class RenderSceneAbstract {
public:
    virtual ~RenderSceneAbstract() = default;
    virtual void setup() = 0;
    virtual void tearDown() = 0;
    virtual void openMap(int mapId, int wdtFileId, const mathfu::vec3 &cameraPos, const mathfu::vec3 &lookAt, int timeOverride) = 0;
    virtual void openWMO(int wmoFileId, const mathfu::vec3 &cameraPos, const mathfu::vec3 &lookAt, int timeOverride) = 0;
    virtual void process(testing::Test *test) = 0;
};

typedef std::shared_ptr<RenderSceneAbstract> HRenderSceneInstance;

#endif //WOWSTUDIO_RENDERSCENEABSTRACT_H
