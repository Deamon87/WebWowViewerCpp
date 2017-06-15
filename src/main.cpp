#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <curl/curl.h>
#include <string>
#include <iostream>
#include <zipper/unzipper.h>
#include "wowScene.h"

#include "persistance/httpFile/httpFile.h"

int main(int argc, char** argv) {
    CURL *curl = NULL;
    FILE *fp;
    CURLcode res;

    char *url = "http://deamon87.github.io/WoWFiles/shattrath.zip\0";

    HttpFile httpFile(new std::string(url));
    httpFile.setCallback([](HttpFile* httpFileLocal) {

        std::vector<unsigned char> unzipped_entry;
        zipper::Unzipper unzipper(*httpFileLocal->getFileBuffer());
        if (unzipper.extractEntryToMemory("world\\generic\\passivedoodads\\fruits\\fruitbowl.blp", unzipped_entry)) {
            std::cout << "file extracted"<<std::flush;
        }
        unzipper.close();
    });
    httpFile.startDownloading();

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
    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    WoWScene *scene = createWoWScene();

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    do {
        // Draw nothing, see you in tutorial 2 !
        scene->draw(1);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

//    } while(true);// Check if the ESC key was pressed or the window was closed
    }    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    return 0;
}