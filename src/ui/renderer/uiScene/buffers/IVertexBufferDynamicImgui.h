//
// Created by Deamon on 14.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICIMGUI_H
#define AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICIMGUI_H


#include "imgui.h"
#include "../../../../../wowViewerLib/src/renderer/buffers/IVertexBufferDynamicTemplate.h"

class IVertexBufferDynamicImgui : public IVertexBufferDynamicTemplate<ImDrawVert>{
public:
    ~IVertexBufferDynamicImgui() override = default ;
};
typedef std::shared_ptr<IVertexBufferDynamicImgui> HIVertexBufferDynamicImgui;

#endif //AWEBWOWVIEWERCPP_IVERTEXBUFFERDYNAMICIMGUI_H
