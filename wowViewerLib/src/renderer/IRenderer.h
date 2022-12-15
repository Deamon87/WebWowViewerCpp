//
// Created by Deamon on 06.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IRENDERER_H
#define AWEBWOWVIEWERCPP_IRENDERER_H

#include <memory>

class IRenderer {
public:
    virtual ~IRenderer() = 0;
};

typedef std::shared_ptr<IRenderer> HIRenderer;

#endif //AWEBWOWVIEWERCPP_IRENDERER_H
