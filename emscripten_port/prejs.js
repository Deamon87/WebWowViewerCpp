// Module['locateFile'] = function (path, prefix) {
//     console.log("path = "+path, path);
//     if ((""+path).indexOf("?") <= 0) {
//         return prefix+path+"?t="+new Date().getTime();
//     }
//     return prefix+path;
// };
Module['hammerJsAssignControl'] = function () {
    var element = Module['canvas'];

    var mleft_pressed = 0;
    var m_x = 0, m_y = 0;

    var isPitchGoingOn = false;
    var isCameraMoving = false;

    var lastTouchStart = new Date(); 
    function touchStart(event) {
        event.preventDefault();
        if (isPitchGoingOn) return;
        mleft_pressed = 1;

        m_x = event.touches[0].pageX;
        m_y = event.touches[0].pageY;

        var newTime = new Date();
        if (newTime.getTime() - lastTouchStart.getTime() < 700 && event.touches.length == 1) {
            isCameraMoving = true;
            Module['_startMovingForward']();
        }
        lastTouchStart = newTime;
    }

    function touchMove(event) {
        event.preventDefault();
        if (isPitchGoingOn) return;
        var x = event.touches[0].pageX;
        var y = event.touches[0].pageY;

        if (mleft_pressed === 1) {
            Module['_addHorizontalViewDir']((x - m_x) / 4.0);
            Module['_addVerticalViewDir']((y - m_y) / 4.0);

            m_x = x;
            m_y = y;
        }
    }
    function touchEnd(event) {
        event.preventDefault();
        if (event.touches.length == 0) {
            mleft_pressed = 0;
            Module['_stopMovingForward']();
            Module['_stopMovingBackwards']();
            isCameraMoving = false;
        }
        event.preventDefault();
        // console.log("(end) event.touches.length" + event.touches.length)
    }

    element.addEventListener('touchstart', touchStart, false);
    element.addEventListener('touchmove', touchMove, false);
    element.addEventListener('touchend', touchEnd, false);

    var mc = new Hammer(element);
    mc.get('pinch').set({ enable: true });
    var pinchScale = 0;

    var previous_pinch_delta = {
        x: 0,
        y: 0
    };

    mc.on("pinchstart pinchin pinchout pinchend pinch", function(ev) {
        if (ev.type == 'pinchstart') {
            pinchScale = ev.scale;
            isPitchGoingOn = true;
            // console.log("pinchstart")
            if (isCameraMoving) {
                Module['_stopMovingForward']();
                Module['_startMovingBackwards']();
            }
        } else if (ev.type == 'pinchend') {
            pinchScale = 0;
            isPitchGoingOn = false;
            console.log("pinchend");
            if (isCameraMoving) {
                Module['_stopMovingBackwards']();
                Module['_startMovingForward']();
            }
        } else if (ev.type == 'pinchin') {
            Module["_zoomInFromMouseScroll"]((pinchScale - ev.scale) * 5);
            pinchScale = ev.scale;
        } else if (ev.type == 'pinchout') {

            Module["_zoomInFromMouseScroll"]((pinchScale - ev.scale) * 5);
            pinchScale = ev.scale;
        }


        if (pinchScale < 0.01) pinchScale = 0.01;

        ev.preventDefault();
    });
}