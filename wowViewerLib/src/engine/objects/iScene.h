//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_IINNERSCENEAPI_H
#define WEBWOWVIEWERCPP_IINNERSCENEAPI_H

#include <memory>

class IScene {
public:
    virtual ~IScene() = 0;
};
typedef std::shared_ptr<IScene> HScene;

#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
