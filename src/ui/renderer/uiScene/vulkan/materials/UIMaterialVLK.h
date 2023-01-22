//
// Created by Deamon on 22.01.23.
//

#ifndef AWEBWOWVIEWERCPP_UIMATERIALVLK_H
#define AWEBWOWVIEWERCPP_UIMATERIALVLK_H

#include "../../materials/UIMaterial.h"
#include "../../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"

class UIMaterialVLK : public ISimpleMaterialVLK {
public:
    UIMaterialVLK(const HGDeviceVLK &device,
                  std::shared_ptr<IBufferChunk<ImgUI::modelWideBlockVS>> UIWideChunk,
                  std::shared_ptr<GTextureVLK> &texture
                  ) :
        ISimpleMaterialVLK(device, "imguiShader", "imguiShader",
                           {std::dynamic_pointer_cast<IBufferVLK>(UIWideChunk)},
                           {std::dynamic_pointer_cast<GTextureVLK>(texture)})
    {

    }
};


#endif //AWEBWOWVIEWERCPP_UIMATERIALVLK_H
