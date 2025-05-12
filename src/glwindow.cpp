#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
GeometryData geometry;
GLuint vbo;
GLint posLoc;
GLuint transformLoc;
int colorLoc;
float earthAngle = 0.0f;  // Initial angle of Earth in orbit
float earthDeltaAngle = 0.008f;    // Increment per frame

float moonAngle = 0.0f;  // Initial angle of Earth in orbit
float moonDeltaAngle = 0.1f;    // Increment per frame

bool pause = false; // Make this 0 or 1 to stop the addition of deltaAngle to angle

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);


    // 1) Load the OBJ into our GeometryData instance and obtain number of vertices and pointer to vertices array
    geometry.loadFromOBJFile("sphere-fixed.obj");
    int nVerts = geometry.vertexCount();            // how many 3-component vertices
    void* verts = geometry.vertexData();            // pointer to float[3*nVerts]

    // 2) Create & fill the VBO with the sphere’s vertex positions, copy our vertices array in a buffer for OpenGL to use
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 nVerts * 3 * sizeof(float),
                 verts,
                 GL_STATIC_DRAW);

    // 3) Then set the vertex attributes pointers
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)0);
    glEnableVertexAttribArray(0);
    
}

void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    transformLoc = glGetUniformLocation(shader, "transform");
    colorLoc = glGetUniformLocation(shader, "objectColor");

    // Draw the sun
    glm::mat4 sunTrans = glm::mat4(1.0f);
    sunTrans = glm::scale(sunTrans, glm::vec3(0.2f,0.2f,0.2f));

    // This passes the matrix to the shader for the uniform variable transform
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(sunTrans));
    glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    
    // Draw the Earth 
    if (!pause) {
        earthAngle += earthDeltaAngle;
    }
    
    // Starting Identity Matrix
    glm::mat4 earthTrans = glm::mat4(1.0f);

    // Rotate the Earth around the Sun (Z-axis in the x-y plane)
    earthTrans = glm::rotate(sunTrans, earthAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate the Earth away from the Sun (Orbit radius)
    earthTrans = glm::translate(earthTrans, glm::vec3(2.5f, 0.0f, 0.0f));

    // Scale the earth down, this is working off of the suns already existing scale
    earthTrans = glm::scale(earthTrans, glm::vec3(0.5f,0.5f,0.5f));

    // Pass in color and transform uniform variables
    glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(earthTrans));

    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    ////////////////////////////////////////////////////////////////////////////
    // Draw the Moon 
    if (!pause) {
        moonAngle += moonDeltaAngle;
    }
    //moonAngle += moonDeltaAngle * pause; //when pause = 0, moonAngle wont increase
        
    // Wrap around when the angle exceeds 360 degrees, if it goes > 360 then it is viewed as value - 360
    // Scale -> Rotate -> Translate (BEN)
    
    


    // Rotate the Earth around the Sun (Z-axis in the x-y plane)
    earthTrans = glm::rotate(earthTrans, moonAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate the Moon away from the Earth
    earthTrans = glm::translate(earthTrans, glm::vec3(2.0f, 0.0f, 0.0f));

    earthTrans = glm::scale(earthTrans, glm::vec3(0.5f,0.5f,0.5f));



    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(earthTrans));

    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    SDL_GL_SwapWindow(sdlWin);
}
// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
            case SDLK_p:
                pause = !pause;
                break;
            case SDLK_d:  // speed up earth
                earthDeltaAngle += 0.005f;
                break;
            case SDLK_a:  // slow down earth
                earthDeltaAngle -= 0.005;
                if (earthDeltaAngle < 0) {
                    earthDeltaAngle = 0;
                  }
                break;
            case SDLK_s:  // reset earth to default
                earthDeltaAngle = 0.01f;
                break;
            case SDLK_RIGHT: //Speed Up Moon
                moonDeltaAngle += 0.01f;
                break;
            case SDLK_LEFT: // Slow Down Moon
                moonDeltaAngle -= 0.01f;
                if (moonDeltaAngle < 0) {
                    moonDeltaAngle = 0;
                  }
                break;
            case SDLK_DOWN: // Reset Moon to Default
                moonDeltaAngle = 0.1f;
                break;
        }
    }
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
