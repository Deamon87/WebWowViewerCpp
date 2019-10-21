//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONTROLLABLE_H
#define WOWVIEWERLIB_CONTROLLABLE_H
class IControllable {
public:
    virtual void startMovingForward() = 0;
    virtual void stopMovingForward() = 0;
    virtual void startMovingBackwards() = 0;
    virtual void stopMovingBackwards() = 0;
    virtual void startStrafingLeft() = 0;
    virtual void stopStrafingLeft() = 0;
    virtual void startStrafingRight() = 0;
    virtual void stopStrafingRight() = 0;
    virtual void startMovingUp() = 0;
    virtual void stopMovingUp() = 0;
    virtual void startMovingDown() = 0;
    virtual void stopMovingDown() = 0;

    virtual void addForwardDiff(float val) = 0;

    virtual void addHorizontalViewDir(float val) = 0;
    virtual void addVerticalViewDir(float val) = 0;

    virtual void zoomInFromMouseScroll(float val) = 0;
    virtual void zoomInFromTouch(float val) = 0;
    virtual void addCameraViewOffset(float x, float y) = 0;


    virtual void getCameraPosition(float *position) = 0;
};
#endif //WOWVIEWERLIB_CONTROLLABLE_H
