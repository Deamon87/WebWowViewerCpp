#define _X86_ 1


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
#include <errhandlingapi.h>
#endif

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#define NKC_IMPLEMENTATION
#define NKCD NKC_GLFW
#define NKC_USE_OPENGL 3
#define GLFW_INCLUDE_GLCOREARB

//#define __EMSCRIPTEN__
#include "engine/HeadersGL.h"
#include <nuklear_cross.h>



#undef GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cmath>
#include "../wowViewerLib/src/include/wowScene.h"
//#include "persistance/ZipRequestProcessor.h"
#include "persistance/CascRequestProcessor.h"
#include "persistance/HttpZipRequestProcessor.h"
#include "persistance/MpqRequestProcessor.h"
#include "persistance/HttpRequestProcessor.h"




int mleft_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

bool stopInputs = true;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    if (stopInputs) return;
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();

//    if (!pointerIsLocked) {
        if (mleft_pressed == 1) {
            controllable->addHorizontalViewDir((xpos - m_x) / 4.0);
            controllable->addVerticalViewDir((ypos - m_y) / 4.0);

            m_x = xpos;
            m_y = ypos;
        }
//    } else {
//        var delta_x = event.movementX ||
//                      event.mozMovementX          ||
//                      event.webkitMovementX       ||
//                      0;
//        var delta_y = event.movementY ||
//                      event.mozMovementY      ||
//                      event.webkitMovementY   ||
//                      0;
//
//        camera.addHorizontalViewDir((delta_x) / 4.0);
//        camera.addVerticalViewDir((delta_y) / 4.0);
//    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (stopInputs) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mleft_pressed = 1;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        m_x = xpos;
        m_y = ypos;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mleft_pressed = 0;
    }
}

Config *testConf;
static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (stopInputs) return;
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();
    if ( action == GLFW_PRESS) {
        switch (key) {
            case 'W' :
                controllable->startMovingForward();
                break;
            case 'S' :
                controllable->startMovingBackwards();
                break;
            case 'A' :
                controllable->startStrafingLeft();
                break;
            case 'D':
                controllable->startStrafingRight();
                break;
            case 'Q':
                controllable->startMovingUp();
                break;
            case 'E':
                controllable->startMovingDown();
                break;

            default:
                break;
        }
    } else if ( action == GLFW_RELEASE) {
        switch (key) {
            case 'W' :
                controllable->stopMovingForward();
                break;
            case 'S' :
                controllable->stopMovingBackwards();
                break;
            case 'A' :
                controllable->stopStrafingLeft();
                break;
            case 'D':
                controllable->stopStrafingRight();
                break;
            case 'Q':
                controllable->stopMovingUp();
                break;
            case 'E':
                controllable->stopMovingDown();
                break;
            case 'H':
                scene->switchCameras();
                break;
            case 'J':
                testConf->setDoubleCameraDebug(!testConf->getDoubleCameraDebug());
                break;
            case 'K':
                testConf->setRenderPortals(!testConf->getRenderPortals());
                break;
            default:
                break;
        }
    }

}

double calcFPS(GLFWwindow* window, double timeInterval = 1.0, std::string windowTitle = "NONE")
{
    // Static values which only get initialised the first time the function runs
    static double startTime  =  glfwGetTime(); // Set the initial time to now
    static double fps        =  0.0;           // Set the initial FPS value to 0.0

    // Set the initial frame count to -1.0 (it gets set to 0.0 on the next line). Because
    // we don't have a start time we simply cannot get an accurate FPS value on our very
    // first read if the time interval is zero, so we'll settle for an FPS value of zero instead.
    static double frameCount =  -1.0;

    // Here again? Increment the frame count
    frameCount++;

    // Ensure the time interval between FPS checks is sane (low cap = 0.0 i.e. every frame, high cap = 10.0s)
    if (timeInterval < 0.0)
    {
        timeInterval = 0.0;
    }
    else if (timeInterval > 10.0)
    {
        timeInterval = 10.0;
    }

    // Get the duration in seconds since the last FPS reporting interval elapsed
    // as the current time minus the interval start time
    double duration = glfwGetTime() - startTime;

    // If the time interval has elapsed...
    if (duration > timeInterval)
    {
        // Calculate the FPS as the number of frames divided by the duration in seconds
        fps = frameCount / duration;

        // If the user specified a window title to append the FPS value to...
        if (windowTitle != "NONE")
        {
            // Convert the fps value into a string using an output stringstream
            std::string fpsString = std::to_string(fps);

            // Append the FPS value to the window title details
            windowTitle += " | FPS: " + fpsString;

            // Convert the new window title to a c_str and set it
            const char* pszConstString = windowTitle.c_str();
            glfwSetWindowTitle(window, pszConstString);
        }
        else // If the user didn't specify a window to append the FPS to then output the FPS to the console
        {
//            std::cout << "FPS: " << fps << std::endl;
        }

        // Reset the frame count to zero and set the initial time to be now
        frameCount        = 0.0;
        startTime = glfwGetTime();
    }

    // Return the current FPS - doesn't have to be used if you don't want it!
    return fps;
}

int canvWidth = 640;
int canvHeight = 480;
bool windowSizeChanged = false;

void window_size_callback(GLFWwindow* window, int width, int height)
{
    canvWidth = width;
    canvHeight = height;
    windowSizeChanged = true;
}

void beforeCrash(void);

#ifdef _WIN32
static const bool SET_TERMINATE = std::set_terminate(beforeCrash);

void beforeCrash() {
    //__asm("int3");
}

static LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stderr);
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
            break;
        case EXCEPTION_BREAKPOINT:
            fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
            break;
    }
	return 0;
}
#endif


enum radioOptions {
    EASY,
    HARD
};

struct my_nkc_app {
    struct nkc* nkcHandle;

    /* some user data */
    float movementSpeed = 1.0;
    bool drawM2AABB = false;
    bool drawWMOAABB = false;

    int wmoMinBatch = 0;
    int wmoMaxBatch = 9999;

    int m2MinBatch = 0;
    int m2MaxBatch = 9999;

    int minParticle = 0;
    int maxParticle = 9999;

    WoWScene *scene;
    RequestProcessor *processor;

    double currentFrame;
    double lastFrame;

    struct nk_color ambientColor;
    float ambientIntensity = 1.0;
    struct nk_color sunColor;
};

inline void CopyAndNullTerminate( const std::string& source,
                                  char* dest,
                                  size_t dest_size )
{
    dest[source.copy(dest, dest_size-1)] = 0;
}

void mainLoop(void* loopArg){
    struct my_nkc_app* myapp = (struct my_nkc_app*)loopArg;
    struct nk_context *ctx = nkc_get_ctx(myapp->nkcHandle);

    union nkc_event e = nkc_poll_events(myapp->nkcHandle);
    if( (e.type == NKC_EWINDOW) && (e.window.param == NKC_EQUIT) ){
        nkc_stop_main_loop(myapp->nkcHandle);
    }

    // Render scene
    myapp->currentFrame = glfwGetTime();
    double deltaTime = myapp->currentFrame - myapp->lastFrame;
    myapp->lastFrame = myapp->currentFrame;

    double fps = calcFPS(nullptr, 2.0);

    myapp->processor->processRequests(false);
    myapp->processor->processResults(10);

    if (windowSizeChanged) {
        myapp->scene->setScreenSize(canvWidth, canvHeight);
        windowSizeChanged = false;
    }

    myapp->scene->draw((deltaTime*1000));

    stopInputs = false;
    /* Nuklear GUI code */
    if (nk_begin(ctx, "Show", nk_rect(50, 50, 300, 300),
                 NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        /* fixed widget pixel width */
//        nk_layout_row_static(ctx, 30, 80, 1);
//        if (nk_button_label(ctx, "button")) {
//            /* event handling */
//            printf("Button pressed\n");
//        }

        //        //MENU
//        {
//            /* menubar */
//            enum menu_states {MENU_DEFAULT, MENU_WINDOWS};
//            static nk_size mprog = 60;
//            static int mslider = 10;
//            static int mcheck = nk_true;
//            nk_menubar_begin(ctx);
//
//            /* menu #1 */
//            nk_layout_row_begin(ctx, NK_STATIC, 25, 5);
//            nk_layout_row_push(ctx, 45);
//            if (nk_menu_begin_label(ctx, "MENU", NK_TEXT_LEFT, nk_vec2(120, 200)))
//            {
//                static size_t prog = 40;
//                static int slider = 10;
//                static int check = nk_true;
//                nk_layout_row_dynamic(ctx, 25, 1);
////                if (nk_menu_item_label(ctx, "Hide", NK_TEXT_LEFT))
////                    show_menu = nk_false;
////                if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT))
////                    show_app_about = nk_true;
//                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
//                nk_slider_int(ctx, 0, &slider, 16, 1);
//                nk_checkbox_label(ctx, "check", &check);
//                nk_menu_end(ctx);
//            }
//            /* menu #2 */
//            nk_layout_row_push(ctx, 60);
//            if (nk_menu_begin_label(ctx, "ADVANCED", NK_TEXT_LEFT, nk_vec2(200, 600)))
//            {
//                enum menu_state {MENU_NONE,MENU_FILE, MENU_EDIT,MENU_VIEW,MENU_CHART};
//                static enum menu_state menu_state = MENU_NONE;
//                enum nk_collapse_states state;
//
//                state = (menu_state == MENU_FILE) ? NK_MAXIMIZED: NK_MINIMIZED;
//                if (nk_tree_state_push(ctx, NK_TREE_TAB, "FILE", &state)) {
//                    menu_state = MENU_FILE;
//                    nk_menu_item_label(ctx, "New", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Close", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT);
//                    nk_tree_pop(ctx);
//                } else menu_state = (menu_state == MENU_FILE) ? MENU_NONE: menu_state;
//
//                state = (menu_state == MENU_EDIT) ? NK_MAXIMIZED: NK_MINIMIZED;
//                if (nk_tree_state_push(ctx, NK_TREE_TAB, "EDIT", &state)) {
//                    menu_state = MENU_EDIT;
//                    nk_menu_item_label(ctx, "Copy", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Delete", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Cut", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Paste", NK_TEXT_LEFT);
//                    nk_tree_pop(ctx);
//                } else menu_state = (menu_state == MENU_EDIT) ? MENU_NONE: menu_state;
//
//                state = (menu_state == MENU_VIEW) ? NK_MAXIMIZED: NK_MINIMIZED;
//                if (nk_tree_state_push(ctx, NK_TREE_TAB, "VIEW", &state)) {
//                    menu_state = MENU_VIEW;
//                    nk_menu_item_label(ctx, "About", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Options", NK_TEXT_LEFT);
//                    nk_menu_item_label(ctx, "Customize", NK_TEXT_LEFT);
//                    nk_tree_pop(ctx);
//                } else menu_state = (menu_state == MENU_VIEW) ? MENU_NONE: menu_state;
//
//                state = (menu_state == MENU_CHART) ? NK_MAXIMIZED: NK_MINIMIZED;
//                if (nk_tree_state_push(ctx, NK_TREE_TAB, "CHART", &state)) {
//                    size_t i = 0;
//                    const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f,12.0f,8.0f,22.0f,28.0f};
//                    menu_state = MENU_CHART;
//                    nk_layout_row_dynamic(ctx, 150, 1);
//                    nk_chart_begin(ctx, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
//                    for (i = 0; i < NK_LEN(values); ++i)
//                    nk_chart_push(ctx, values[i]);
//                    nk_chart_end(ctx);
//                    nk_tree_pop(ctx);
//                } else menu_state = (menu_state == MENU_CHART) ? MENU_NONE: menu_state;
//                nk_menu_end(ctx);
//            }
//            /* menu widgets */
//            nk_layout_row_push(ctx, 70);
//            nk_progress(ctx, &mprog, 100, NK_MODIFIABLE);
//            nk_slider_int(ctx, 0, &mslider, 16, 1);
//            nk_checkbox_label(ctx, "check", &mcheck);
//            nk_menubar_end(ctx);
//        }


        nk_layout_row_begin(ctx, NK_STATIC, 30, 1);
        {
            nk_layout_row_push(ctx, 250);
            std::string areaName = "AreaName: " + testConf->getAreaName();
            nk_label(ctx, areaName.c_str(), NK_TEXT_LEFT);
        }
        nk_layout_row_end(ctx);
        /* fixed widget window ratio width */
        nk_layout_row_begin(ctx, NK_STATIC, 30, 1);
        {
            nk_layout_row_push(ctx, 50);
            nk_label(ctx, "Draw M2 AABB:", NK_TEXT_LEFT);
        }
        nk_layout_row_end(ctx);
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 50);
            if (nk_option_label(ctx, "on", myapp->drawM2AABB)) myapp->drawM2AABB = true;
            if (nk_option_label(ctx, "off", !myapp->drawM2AABB)) myapp->drawM2AABB = false;
        }
        nk_layout_row_end(ctx);

        nk_layout_row_push(ctx, 50);
        nk_label(ctx, "Draw WMO AABB:", NK_TEXT_LEFT);
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 50);
            if (nk_option_label(ctx, "on", myapp->drawWMOAABB)) myapp->drawWMOAABB = true;
            if (nk_option_label(ctx, "off", !myapp->drawWMOAABB)) myapp->drawWMOAABB = false;
        }
        nk_layout_row_end(ctx);


        testConf->setDrawM2BB(myapp->drawM2AABB);
        testConf->setDrawWmoBB(myapp->drawWMOAABB);


        nk_layout_row_push(ctx, 50);
        nk_label(ctx, "AmbientColor:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, myapp->ambientColor, nk_vec2(nk_widget_width(ctx),400))) {

            nk_layout_row_dynamic(ctx, 120, 1);
            myapp->ambientColor = nk_color_picker(ctx, myapp->ambientColor, NK_RGBA);
            nk_layout_row_dynamic(ctx, 25, 1);
            myapp->ambientColor.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, myapp->ambientColor.r, 255, 1,1);
            myapp->ambientColor.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, myapp->ambientColor.g, 255, 1,1);
            myapp->ambientColor.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, myapp->ambientColor.b, 255, 1,1);
            myapp->ambientColor.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, myapp->ambientColor.a, 255, 1,1);
            nk_combo_end(ctx);

            testConf->setAmbientColor(
                myapp->ambientColor.r / 255.0f,
                myapp->ambientColor.g / 255.0f,
                myapp->ambientColor.b / 255.0f,
                myapp->ambientColor.a / 255.0f
            );
            stopInputs = true;
        }

        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 50);
            nk_label(ctx, "Ambient intensity:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 110);
            if (nk_slider_float(ctx, 0, &(myapp->ambientIntensity), 1.0f, 0.1f)) {
                stopInputs = true;
            }
        }
        nk_layout_row_end(ctx);

        nk_layout_row_push(ctx, 50);
        nk_label(ctx, "SunColor:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, myapp->sunColor, nk_vec2(nk_widget_width(ctx),400))) {

            nk_layout_row_dynamic(ctx, 120, 1);
            myapp->sunColor = nk_color_picker(ctx, myapp->sunColor, NK_RGBA);
            nk_layout_row_dynamic(ctx, 25, 1);
            myapp->sunColor.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, myapp->sunColor.r, 255, 1,1);
            myapp->sunColor.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, myapp->sunColor.g, 255, 1,1);
            myapp->sunColor.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, myapp->sunColor.b, 255, 1,1);
            myapp->sunColor.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, myapp->sunColor.a, 255, 1,1);
            nk_combo_end(ctx);

            testConf->setSunColor(
                myapp->sunColor.r / 255.0f,
                myapp->sunColor.g / 255.0f,
                myapp->sunColor.b / 255.0f,
                myapp->sunColor.a / 255.0f
            );
            stopInputs = true;
        }

        //Debug batch stuff
        nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
        {
            nk_layout_row_push(ctx, 120);
            nk_label(ctx, "Wmo batch limiter:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);

            char buffer[10];
            CopyAndNullTerminate(std::to_string(myapp->wmoMinBatch), buffer, 10);
            nk_layout_row_push(ctx, 50);
            nk_flags event = nk_edit_string_zero_terminated(ctx,
                                                            NK_EDIT_BOX |
                                                            NK_EDIT_AUTO_SELECT, //fcous will auto select all text (NK_EDIT_BOX not sure)
                                                            buffer, sizeof(buffer),
                                                            nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.

            try {
                myapp->wmoMinBatch = std::stoi(buffer);
            } catch(...) {
                myapp->wmoMinBatch = 0;
            }
            nk_layout_row_push(ctx, 50);
            CopyAndNullTerminate(std::to_string(myapp->wmoMaxBatch), buffer, 10);
            nk_edit_string_zero_terminated(ctx,
                                           NK_EDIT_BOX |
                                           NK_EDIT_AUTO_SELECT, //fcous will auto select all text (NK_EDIT_BOX not sure)
                                           buffer, sizeof(buffer),
                                           nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.

            try {
                myapp->wmoMaxBatch = std::stoi(buffer);
            } catch(...) {
                myapp->wmoMaxBatch = 9999;
            }

            testConf->setWmoMinBatch(myapp->wmoMinBatch);
            testConf->setWmoMaxBatch(myapp->wmoMaxBatch);
        }
        nk_layout_row_end(ctx);

        nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
        {
            nk_layout_row_push(ctx, 120);
            nk_label(ctx, "M2 batch limiter:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);

            char buffer[10];
            CopyAndNullTerminate(std::to_string(myapp->m2MinBatch), buffer, 10);
            nk_layout_row_push(ctx, 50);
            nk_flags event = nk_edit_string_zero_terminated(ctx,
                                                            NK_EDIT_BOX |
                                                            NK_EDIT_AUTO_SELECT, //fcous will auto select all text (NK_EDIT_BOX not sure)
                                                            buffer, sizeof(buffer),
                                                            nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.

            try {
                myapp->m2MinBatch = std::stoi(buffer);
            } catch(...) {
                myapp->m2MinBatch = 0;
            }
            nk_layout_row_push(ctx, 50);
            CopyAndNullTerminate(std::to_string(myapp->m2MaxBatch), buffer, 10);
            nk_edit_string_zero_terminated(ctx,
                                           NK_EDIT_BOX |
                                           NK_EDIT_AUTO_SELECT, //fcous will auto select all text (NK_EDIT_BOX not sure)
                                           buffer, sizeof(buffer),
                                           nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.

            try {
                myapp->m2MaxBatch = std::stoi(buffer);
            } catch(...) {
                myapp->m2MaxBatch = 9999;
            }

            testConf->setM2MinBatch(myapp->m2MinBatch);
            testConf->setM2MaxBatch(myapp->m2MaxBatch);
        }
        nk_layout_row_end(ctx);

        nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
        {
            nk_layout_row_push(ctx, 120);
            nk_label(ctx, "Particle limiter:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);

            char buffer[10];
            CopyAndNullTerminate(std::to_string(myapp->minParticle), buffer, 10);
            nk_flags event = nk_edit_string_zero_terminated(ctx,
                                                            NK_EDIT_BOX |
                                                            NK_EDIT_AUTO_SELECT, //fcous will auto select all text (NK_EDIT_BOX not sure)
                                                            buffer, sizeof(buffer),
                                                            nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.

            try {
                myapp->minParticle = std::stoi(buffer);
            } catch(...) {
                myapp->minParticle = 0;
            }
            CopyAndNullTerminate(std::to_string(myapp->maxParticle), buffer, 10);
            nk_edit_string_zero_terminated(ctx,
                                           NK_EDIT_BOX |
                                           NK_EDIT_AUTO_SELECT, //fcous will auto select all text (NK_EDIT_BOX not sure)
                                           buffer, sizeof(buffer),
                                           nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.

            try {
                myapp->maxParticle = std::stoi(buffer);
            } catch(...) {
                myapp->maxParticle = 9999;
            }

            testConf->setMinParticle(myapp->minParticle);
            testConf->setMaxParticle(myapp->maxParticle);
        }
        nk_layout_row_end(ctx);
//
        /* custom widget pixel width */
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        {
            nk_layout_row_push(ctx, 120);
            nk_label(ctx, "Movement Speed:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 110);
            nk_slider_float(ctx, 0, &(myapp->movementSpeed), 20.0f, 0.1f);
            testConf->setMovementSpeed(myapp->movementSpeed);
        }
        nk_layout_row_end(ctx);

        nk_layout_row_push(ctx, 80);
        nk_label(ctx, "Coordinates :", NK_TEXT_LEFT);

        nk_layout_row_begin(ctx, NK_STATIC, 15, 3);
        {


            float cameraPos[4];
            myapp->scene->getCurrentCamera()->getCameraPosition(cameraPos);

            nk_layout_row_push(ctx, 80);
            nk_label(ctx, std::to_string(cameraPos[0]).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 80);
            nk_label(ctx, std::to_string(cameraPos[1]).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 80);
            nk_label(ctx, std::to_string(cameraPos[2]).c_str(), NK_TEXT_LEFT);
        }
        nk_layout_row_end(ctx);
        nk_layout_row_begin(ctx, NK_STATIC, 15, 2);
        {
            nk_layout_row_push(ctx, 50);
            nk_label(ctx, "FPS :", NK_TEXT_LEFT);
            nk_label(ctx, std::to_string(fps).c_str(), NK_TEXT_LEFT);
        }

    }
    nk_end(ctx);
    /* End Nuklear GUI */

    nkc_render_gui(myapp->nkcHandle);
}


int main(){
    struct my_nkc_app myapp;
    struct nkc nkcx; /* Allocate memory for Nuklear+ handle */
    myapp.nkcHandle = &nkcx;


#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif


//    const char *url = "http://deamon87.github.io/WoWFiles/shattrath.zip\0";
//    const char *url = "http://deamon87.github.io/WoWFiles/ironforge.zip\0";
//    const char *filePath = "D:\\shattrath (1).zip\0";
//    const char *filePath = "D:\\ironforge.zip\0";
    const char * url = "http://178.165.92.24:40001/get/";
    const char * urlFileId = "http://178.165.92.24:40001/get_file_id/";
//    const char *filePath = "d:\\Games\\WoW_3.3.5._uwow.biz_EU\\Data\\\0";
    const char *filePath = "d:\\Games\\WoWLimitedUS\\World of Warcraft\\\0";
//     const char *url = "http://localhost:8084/get/";

    testConf = new Config();



    if( nkc_init( myapp.nkcHandle, "WowViewer", 640, 480, NKC_WIN_NORMAL) ){
        printf("Successfull init. Starting 'infinite' main loop...\n");

        //    HttpZipRequestProcessor *processor = new HttpZipRequestProcessor(url);
        //    ZipRequestProcessor *processor = new ZipRequestProcessor(filePath);
        //    MpqRequestProcessor *processor = new MpqRequestProcessor(filePath);
        HttpRequestProcessor *processor = new HttpRequestProcessor(url, urlFileId);
//        CascRequestProcessor *processor = new CascRequestProcessor(filePath);
        processor->setThreaded(true);

        WoWScene *scene = createWoWScene(testConf, processor, canvWidth, canvHeight);
        processor->setFileRequester(scene);
        testConf->setDrawM2BB(false);
        //testConf->setUsePortalCulling(false);

        myapp.scene = scene;
        myapp.processor = processor;
        myapp.currentFrame = glfwGetTime();
        myapp.lastFrame = myapp.currentFrame;

        glfwSetWindowUserPointer(myapp.nkcHandle->window, scene);
        glfwSetKeyCallback(myapp.nkcHandle->window, onKey);
        glfwSetCursorPosCallback( myapp.nkcHandle->window, cursor_position_callback);
        glfwSetWindowSizeCallback( myapp.nkcHandle->window, window_size_callback);
        glfwSetWindowSizeLimits( myapp.nkcHandle->window, canvWidth, canvHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetMouseButtonCallback( myapp.nkcHandle->window, mouse_button_callback);
//        glfwSwapInterval(0);

        nkc_set_main_loop(myapp.nkcHandle, mainLoop, (void*)&myapp );
    } else {
        printf("Can't init NKC\n");
    }
    nkc_shutdown( myapp.nkcHandle );
    return 0;
}
