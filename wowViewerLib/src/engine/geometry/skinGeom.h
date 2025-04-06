//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_SKINGEOM_H
#define WOWVIEWERLIB_SKINGEOM_H

#include <vector>
#include "../persistance/header/skinFileHeader.h"
#include "../persistance/header/M2FileHeader.h"

#ifndef WOWLIB_EXCLUDE_RENDERER
#include "../../renderer/mapScene/IMapSceneBufferCreate.h"
#endif

class SkinGeom : public PersistentFile {
public:
    SkinGeom(std::string fileName){};
    SkinGeom(int fileDataId){};

    void process(HFileContent skinFile, const std::string &fileName) override;
    void generateIndexBuffer(std::vector<uint16_t> &buffer);

    M2SkinProfile * getSkinData(){ if (fsStatus == FileStatus::FSLoaded) {return m_skinData;} else {return nullptr;}};

    void fixData(M2Data *m2File);
private:
    HFileContent m2Skin;
    M2SkinProfile *m_skinData = nullptr;

    bool m_fixed = false;
    void fixShaderIdBasedOnLayer(M2Data *m2Data);
    void fixShaderIdBasedOnBlendOverride(M2Data *m2Data);
#ifndef WOWLIB_EXCLUDE_RENDERER
public:
    HGIndexBuffer getIBO(const HMapSceneBufferCreate &renderer);
private:
    //This is being stored here, to prevent from IBO being created for same skin several times
    HGIndexBuffer m_IBO = nullptr;
#endif

};


#endif //WOWVIEWERLIB_SKINGEOM_H
