#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <curl/curl.h>
#include <string>
#include <iostream>
#include "wowScene.h"
#include <zip.h>

#include "persistance/httpFile/httpFile.h"
static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentContollable();
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

            default:
                break;
        }
    }

}

class RequestProcessor : public IFileRequest {
public:
    RequestProcessor () {
        char *url = "http://deamon87.github.io/WoWFiles/shattrath.zip\0";

        using namespace std::placeholders;
        HttpFile httpFile(new std::string(url));
        httpFile.setCallback(std::bind(&RequestProcessor::loadingFinished, this, _1));
        httpFile.startDownloading();
    }
private:
//    zipper::Unzipper *m_unzipper;
    IFileRequester *m_fileRequester;

public:
    void setFileRequester(IFileRequester *fileRequester) {
        m_fileRequester = fileRequester;
    }
    void loadingFinished(std::vector<unsigned char> * file) {
        zip_source_t *src;
        zip_t *za;
        zip_error_t error;

        zip_error_init(&error);
        /* create source from buffer */
        if ((src = zip_source_buffer_create(&file->at(0), file->size(), 1, &error)) == NULL) {
            fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
//            free(data);
            zip_error_fini(&error);
//            return 1;
        }

        /* open zip archive from source */
        if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
            fprintf(stderr, "can't open zip from source: %s\n", zip_error_strerror(&error));
            zip_source_free(src);
            zip_error_fini(&error);
//            return 1;
        }
        zip_error_fini(&error);

        /* we'll want to read the data back after zip_close */
        zip_source_keep(src);
    }

    void requestFile(const char* fileName) {
        std::string s_fileName(fileName);

        std::vector<unsigned char> unzipped_entry;
//        if (m_unzipper->extractEntryToMemory(s_fileName, unzipped_entry)) {
//            m_fileRequester->provideFile(fileName, &unzipped_entry[0], unzipped_entry.size());
//        } else {
//            m_fileRequester->rejectFile(fileName);
//        }
    };
};


int main(int argc, char** argv) {
    CURL *curl = NULL;
    FILE *fp;
    CURLcode res;


    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

//     Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    window = glfwCreateWindow( 1024, 768, "Test Window", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW

    Config *testConf = new Config();
    RequestProcessor *processor = new RequestProcessor();
    WoWScene *scene = createWoWScene(testConf, processor, 1000, 1000);
    processor->setFileRequester(scene);

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetWindowUserPointer(window, scene);
    glfwSetKeyCallback(window, onKey);

    double currentFrame = glfwGetTime();
    double lastFrame = currentFrame;
    double deltaTime;
    do {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        scene->draw(deltaTime*1000);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

//    } while(true);// Check if the ESC key was pressed or the window was closed
    }    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    return 0;
}