#include <jni.h>
#include <string>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/input.h>

#include <EGL/egl.h>
#include <imgui.h>
#include <imguiImpl/imgui_impl_android.h>

#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "../../../../src/database/CEmptySqliteDB.h"
#include "../../../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../../../wowViewerLib/src/engine/SceneComposer.h"
#include "../../../../src/ui/FrontendUI.h"
#include "../../../../src/ui/imguiLib/imguiImpl/imgui_impl_android.h"
#include "../../../../src/persistance/HttpRequestProcessor.h"
#include "../../../../wowViewerLib/src/engine/androidLogSupport.h"

SceneComposer *sceneComposer= nullptr;
HApiContainer apiContainer= nullptr;
std::shared_ptr<FrontendUI> frontendUI = nullptr;

static EGLDisplay           g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface           g_EglSurface = EGL_NO_SURFACE;
static EGLContext           g_EglContext = EGL_NO_CONTEXT;
static struct android_app*  g_App = NULL;
static bool                 g_Initialized = false;

static char                 g_LogTag[] = "wowViewer";

bool stopMouse;
bool stopKeyboard;
int canvWidth;
int canvHeight;

AAssetManager * g_assetMgr = nullptr;
static JavaVM *gJavaVM;

#define logExecution {}
//#define logExecution { \
//    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
//}

static int ShowSoftKeyboardInput();
static int PollUnicodeChars();

void init(struct android_app* app)
{
    if (g_Initialized)
        return;

    g_App = app;
    ANativeWindow_acquire(g_App->window);

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    {
        g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (g_EglDisplay == EGL_NO_DISPLAY)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");

        if (eglInitialize(g_EglDisplay, 0, 0) != EGL_TRUE)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglInitialize() returned with an error");

        const EGLint egl_attributes[] = { EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
        EGLint num_configs = 0;
        if (eglChooseConfig(g_EglDisplay, egl_attributes, nullptr, 0, &num_configs) != EGL_TRUE)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglChooseConfig() returned with an error");
        if (num_configs == 0)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglChooseConfig() returned 0 matching config");

        // Get the first matching config
        EGLConfig egl_config;
        eglChooseConfig(g_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
        EGLint egl_format;
        eglGetConfigAttrib(g_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
        ANativeWindow_setBuffersGeometry(g_App->window, 0, 0, egl_format);

        const EGLint egl_context_attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
        g_EglContext = eglCreateContext(g_EglDisplay, egl_config, EGL_NO_CONTEXT, egl_context_attributes);

        if (g_EglContext == EGL_NO_CONTEXT)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglCreateContext() returned EGL_NO_CONTEXT");

        g_EglSurface = eglCreateWindowSurface(g_EglDisplay, egl_config, g_App->window, NULL);
        eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);
    }

    int32_t window_width = ANativeWindow_getWidth(g_App->window);
    int32_t window_height = ANativeWindow_getHeight(g_App->window);

    canvHeight = window_height;
    canvWidth = window_width;

    g_assetMgr = g_App->activity->assetManager;

    void *callback = nullptr;

    //Open Sql storage
    apiContainer = std::make_shared<ApiContainer>();

    //Create device
    std::string rendererName = "ogl3";
    logExecution
    auto hdevice = IDeviceFactory::createDevice(rendererName, &callback);
    logExecution
    apiContainer->databaseHandler = std::make_shared<CEmptySqliteDB>() ;
    logExecution
    apiContainer->hDevice = hdevice;
    logExecution
    apiContainer->camera = std::make_shared<FirstPersonCamera>();
    logExecution

    sceneComposer = new SceneComposer(apiContainer);
    logExecution


    logExecution
    frontendUI = std::make_shared<FrontendUI>(apiContainer, nullptr);
    logExecution
//    auto window = ANativeWindow_fromSurface(env, surface);
    auto window = g_App->window;
    logExecution
    frontendUI->initImgui(window);
    logExecution

    g_Initialized = true;
}


double currentFrame = 0.0f;
double lastFrame = 0.0f;
void tick()
{
    ImGuiIO& io = ImGui::GetIO();
    if (g_EglDisplay == EGL_NO_DISPLAY)
        return;

    // Our state
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Poll Unicode characters via JNI
    // FIXME: do not call this every frame because of JNI overhead
    PollUnicodeChars();

    //Render
    frontendUI->newFrame();
    frontendUI->composeUI();

    stopMouse = frontendUI->getStopMouse();
    stopKeyboard = frontendUI->getStopKeyboard();

    logExecution
    {
        auto processor = frontendUI->getProcessor();
        if (frontendUI->getProcessor()) {

            if (!processor->getThreaded()) {
                processor->processRequests(false);
            }
        }
    }

    int32_t window_width = ANativeWindow_getWidth(g_App->window);
    int32_t window_height = ANativeWindow_getHeight(g_App->window);
    canvHeight = window_height;
    canvWidth = window_width;

    double deltaTime = currentFrame - lastFrame;

    apiContainer->camera->tick(deltaTime*(1000.0f));
    //DrawStage for screenshot
//        needToMakeScreenshot = true;
    if (apiContainer->getConfig()->pauseAnimation) {
        deltaTime = 0.0;
    }
    auto sceneScenario = frontendUI->createFrameScenario(canvWidth, canvHeight, deltaTime);
    sceneComposer->draw(sceneScenario);


    // Open on-screen (soft) input if requested by Dear ImGui
    static bool WantTextInputLast = false;
    if (io.WantTextInput && !WantTextInputLast)
        ShowSoftKeyboardInput();
    WantTextInputLast = io.WantTextInput;

    eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

void shutdown()
{
    if (!g_Initialized)
        return;

    // Cleanup
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (g_EglDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_EglContext != EGL_NO_CONTEXT)
            eglDestroyContext(g_EglDisplay, g_EglContext);

        if (g_EglSurface != EGL_NO_SURFACE)
            eglDestroySurface(g_EglDisplay, g_EglSurface);

        eglTerminate(g_EglDisplay);
    }

    g_EglDisplay = EGL_NO_DISPLAY;
    g_EglContext = EGL_NO_CONTEXT;
    g_EglSurface = EGL_NO_SURFACE;
    ANativeWindow_release(g_App->window);

    g_Initialized = false;
}

static void handleAppCmd(struct android_app* app, int32_t appCmd)
{
    switch (appCmd)
    {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            init(app);
            break;
        case APP_CMD_TERM_WINDOW:
            shutdown();
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            break;
    }
}

static int32_t handleInputEvent(struct android_app* app, AInputEvent* inputEvent)
{
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}


int cdToExtStorage(void) {

    // Make JNI calls to get the external storage directory, and cd to it.

    // To begin, get a reference to the env and attach to it.
    JNIEnv *env;
    int isAttached = 0;
    int ret = 0;
    jthrowable exception;
    if ((gJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6)) < 0) {
        // Couldn't get JNI environment, so this thread is native.
        if ((gJavaVM->AttachCurrentThread(&env, NULL)) < 0) {
            fprintf(stderr, "Error: Couldn't attach to Java VM.\n");
            return (-1);
        }
        isAttached = 1;
    }

    // Get File object for the external storage directory.
    jclass classEnvironment = env->FindClass("android/os/Environment");
    if (!classEnvironment) {
        std::cout << "Error: JNI call failure.\n" << std::endl;
    };
    jmethodID methodIDgetExternalStorageDirectory = env->GetStaticMethodID(classEnvironment, "getExternalStorageDirectory", "()Ljava/io/File;"); // public static File getExternalStorageDirectory ()
    if (!methodIDgetExternalStorageDirectory)  {
        std::cout << "Error: JNI call failure.\n" << std::endl;
    };
    jobject objectFile = env->CallStaticObjectMethod(classEnvironment, methodIDgetExternalStorageDirectory);
    exception = env->ExceptionOccurred();
    if (exception) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    // Call method on File object to retrieve String object.
    jclass classFile = env->GetObjectClass(objectFile);
    if (!classFile) {
        std::cout << "Error: JNI call failure.\n" << std::endl;
    }
    jmethodID methodIDgetAbsolutePath = env->GetMethodID(classFile, "getAbsolutePath", "()Ljava/lang/String;");
    if (!methodIDgetAbsolutePath) {
        std::cout << "Error: JNI call failure.\n" << std::endl;
    }

    jstring stringPath = static_cast<jstring>(env->CallObjectMethod(objectFile,
                                                                    methodIDgetAbsolutePath));
    exception = env->ExceptionOccurred();
    if (exception) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    // Extract a C string from the String object, and chdir() to it.
    const char *wpath3 = env->GetStringUTFChars(stringPath, NULL);
    if (chdir(wpath3) != 0) {
        fprintf(stderr, "Error: Unable to change working directory to %s.\n", wpath3);
        perror(NULL);
    }

    env->ReleaseStringUTFChars(stringPath, wpath3);

    if (isAttached) gJavaVM->DetachCurrentThread(); // Clean up.
    return (ret);
}

void android_main(struct android_app* app) {
    gJavaVM = app->activity->vm;
    std::cout.rdbuf(new androidbuf);

    cdToExtStorage();

    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    while (true)
    {
        int out_events;
        struct android_poll_source* out_data;

        // Poll all events. If the app is not visible, this loop blocks until g_Initialized == true.
        while (ALooper_pollAll(g_Initialized ? 0 : -1, NULL, &out_events, (void**)&out_data) >= 0)
        {
            // Process one event
            if (out_data != NULL)
                out_data->process(app, out_data);

            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0)
            {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (!g_Initialized)
                    shutdown();

                return;
            }
        }

        // Initiate a new frame
        tick();
    }
}

// Unfortunately, the native KeyEvent implementation has no getUnicodeChar() function.
// Therefore, we implement the processing of KeyEvents in MainActivity.kt and poll
// the resulting Unicode characters here via JNI and send them to Dear ImGui.
static int PollUnicodeChars()
{
    return 0;
    JavaVM* java_vm = g_App->activity->vm;
    JNIEnv* java_env = NULL;

    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    if (native_activity_clazz == NULL)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "pollUnicodeChar", "()I");
    if (method_id == NULL)
        return -4;

    // Send the actual characters to Dear ImGui
    ImGuiIO& io = ImGui::GetIO();
    jint unicode_character;
    while ((unicode_character = java_env->CallIntMethod(g_App->activity->clazz, method_id)) != 0)
        io.AddInputCharacter(unicode_character);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}

// Unfortunately, there is no way to show the on-screen input from native code.
// Therefore, we call ShowSoftKeyboardInput() of the main activity implemented in MainActivity.kt via JNI.
static int ShowSoftKeyboardInput()
{
    return 0;
    JavaVM* java_vm = g_App->activity->vm;
    JNIEnv* java_env = NULL;

    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    if (native_activity_clazz == NULL)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "showSoftInput", "()V");
    if (method_id == NULL)
        return -4;

    java_env->CallVoidMethod(g_App->activity->clazz, method_id);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}