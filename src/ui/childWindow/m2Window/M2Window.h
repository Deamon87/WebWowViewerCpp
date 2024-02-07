//
// Created by Deamon on 2/7/2024.
//

#ifndef AWEBWOWVIEWERCPP_M2WINDOW_H
#define AWEBWOWVIEWERCPP_M2WINDOW_H


#include "../sceneWindow/SceneWindow.h"

class M2Window : public SceneWindow {
public:
    bool draw();

private:
    bool m_showWindow;
};


#endif //AWEBWOWVIEWERCPP_M2WINDOW_H
