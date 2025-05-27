#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
GeometryData geometry;
GLuint vbo;
GLuint uvVBO;
GLuint normVBO;

GLint posLoc;
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint textureLoc;
GLint useClrLoc;
GLint clrLoc;
GLint useCloudsLoc;
GLint timeLoc;
GLuint texture;

GLuint sunTexture;
GLuint earthTexture;
GLuint moonTexture;
GLuint marsTexture;
GLuint jupiterTexture;
GLuint venusTexture;
GLuint cloudTexture;
int colorLoc;
float earthAngle = 0.0f;  // Initial angle of Earth in orbit
float earthDeltaAngle = 0.008f;    // Increment per frame

float moonAngle = 0.0f;  // Initial angle of Moon in orbit
float moonDeltaAngle = 0.1f;    // Increment per frame

float marsAngle = 0.0f;  // Initial angle of Mars in orbit
float marsDeltaAngle = 0.006f;    // Increment per frame

float jupiterAngle = 0.0f;  // Initial angle of Jupiter in orbit
float jupiterDeltaAngle = 0.004f;    // Increment per frame

float venusAngle = 0.0f;  // Initial angle of Venus in orbit
float venusDeltaAngle = 0.005f;    // Increment per frame


bool pause = false; // Make this 0 or 1 to stop the addition of deltaAngle to angle
float deltaTime = 0.0f;

// spherical camera
float camRadius = 3.0f;       // starting distance from origin
float camYaw    =  45.0f;      // degrees around Y
float camPitch  =  20.0f;      // degrees up/down
// speeds
const float yawSpeed   = 120.0f;
const float pitchSpeed = 120.0f;

float camRoll    = 0.0f;      // degrees around forward axis
const float rollSpeed = 120.0f; // deg/sec

float lastTime = 0.0f;

// 2nd light source
const glm::vec3 light2Pos = glm::vec3(0.5f, 0.5f, 0.5f);
float light2Angle      = 0.0f;
const float light2DeltaAngle = 0.005f;  // speed of that orbit
const float light2Radius     = 3.0f;    // how far from the Sun it orbits

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

void OpenGLWindow::loadTexture(const std::string &path, GLuint &outTexID) {
    glGenTextures(1, &outTexID);
    glBindTexture(GL_TEXTURE_2D, outTexID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Get all the data from the texture image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data)
    {
        GLenum internalFmt = (nrChannels == 4 ? GL_RGBA : GL_RGB);
        GLenum srcFmt      = (nrChannels == 4 ? GL_RGBA : GL_RGB);
        // Generates Texture
        glTexImage2D(GL_TEXTURE_2D,
            0,
            internalFmt,
            width, height,
            0,
            srcFmt,
            GL_UNSIGNED_BYTE,
            data);
        // This will automatically generate all the required mipmaps for the currently bound texture under the 2D name
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

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
                              760, 640, SDL_WINDOW_OPENGL);
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

    // Sun Light
    glUniform3f(glGetUniformLocation(shader,"lightPos1"), 0,0,0);
    glUniform3f(glGetUniformLocation(shader,"lightColor1"), 1.0f,0.9f,0.6f);
    // Second “fill” light
    glUniform3fv(glGetUniformLocation(shader, "lightPos2"), 1, glm::value_ptr(light2Pos));
    glUniform3f (glGetUniformLocation(shader, "lightColor2"), 0.7f, 0.7f, 0.7f);

    textureLoc = glGetUniformLocation(shader, "ourTexture");
    glUniform1i(textureLoc, 0);     
    loadTexture("sun_texture.png", sunTexture);
    loadTexture("earth_texture.png", earthTexture);
    loadTexture("moon_texture.png", moonTexture);
    loadTexture("mars_texture.jpg", marsTexture);
    loadTexture("jupiter_texture.jpg", jupiterTexture);
    loadTexture("venus_texture.jpg", venusTexture);
    loadTexture("cloud_texture.jpg", cloudTexture);
    glUniform1i(useCloudsLoc, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);

    // tell shader which unit it lives in
    GLint cloudLoc = glGetUniformLocation(shader, "cloudTexture");
    glUniform1i(cloudLoc, 1);
    
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
                          3 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);


    // UV Data for Textures
    glGenBuffers(1, &uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER,
                    nVerts * 2 * sizeof(float),
                    geometry.textureCoordData(),
                    GL_STATIC_DRAW);

    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(1);

    // Normals For Lighting
    glGenBuffers(1, &normVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 nVerts * 3 * sizeof(float),
                 geometry.normalData(),
                 GL_STATIC_DRAW);
                 
    glVertexAttribPointer(2,3, GL_FLOAT, GL_FALSE,3 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(2);

    modelLoc = glGetUniformLocation(shader, "model");
    viewLoc = glGetUniformLocation(shader, "view");
    projectionLoc = glGetUniformLocation(shader, "projection");
    colorLoc = glGetUniformLocation(shader, "objectColor");
    useClrLoc = glGetUniformLocation(shader, "uUseOverrideColor");
    clrLoc    = glGetUniformLocation(shader, "uOverrideColor");
    useCloudsLoc   = glGetUniformLocation(shader, "uUseClouds");
    timeLoc        = glGetUniformLocation(shader, "uTime");
    GLint cloudStrengthLoc   = glGetUniformLocation(shader, "uCloudStrength");
    GLint cloudBrightnessLoc = glGetUniformLocation(shader, "uCloudBrightness");
    GLint cloudSpeedLoc = glGetUniformLocation(shader, "uCloudSpeed");

    glUniform1f(cloudStrengthLoc,   0.35f);
    glUniform1f(cloudBrightnessLoc, 1.5f);  
    glUniform1f(cloudSpeedLoc, 0.2f);

    // Building Projection Matrix
    float aspect = 760.0f/640.0f;
    float halfHeight = 2.0f;
    float halfWidth = halfHeight * aspect;

    glm::mat4 orthoProjectionTrans = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(orthoProjectionTrans));

    lastTime = SDL_GetTicks() / 1000.0f; 
}



void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    GLint overrideLoc = glGetUniformLocation(shader, "uUseOverrideColor");
    GLint colorLoc    = glGetUniformLocation(shader, "uOverrideColor");
    glUniform1i(overrideLoc, 0); 

    float now    = SDL_GetTicks() / 1000.0f;
    deltaTime    = now - lastTime;
    lastTime     = now;

    glUniform1f(timeLoc, now);

    // Update 2nd light orbiting around solar system
    if (!pause) {
        light2Angle += light2DeltaAngle;
    }
    glm::vec3 light2Pos = {
        light2Radius * cos(light2Angle),
        0.5f,
        light2Radius * sin(light2Angle)
    };

    glUseProgram(shader);
    glUniform3fv(
        glGetUniformLocation(shader, "lightPos2"),
        1,
        glm::value_ptr(light2Pos)
    );
 
    float yawR   = glm::radians(camYaw);
    float pitchR = glm::radians(camPitch);

    glm::vec3 up = (cos(pitchR) >= 0.0f)
              ? glm::vec3(0,1,0)
              : glm::vec3(0,-1,0);
 
    glm::vec3 cameraPos;
    cameraPos.x = camRadius * cos(pitchR) * sin(yawR);
    cameraPos.y = camRadius * sin(pitchR);
    cameraPos.z = camRadius * cos(pitchR) * cos(yawR);

    // Base up, flipping at the poles
    glm::vec3 baseUp = (cos(pitchR) >= 0.0f)
    ? glm::vec3(0,1,0)
    : glm::vec3(0,-1,0);

    // Forward axis
    glm::vec3 forward = glm::normalize(glm::vec3(0.0f) - cameraPos);

    // Right axis
    glm::vec3 right   = glm::normalize(glm::cross(forward, baseUp));

    // Roll in radians
    float rollR = glm::radians(camRoll);

    // Rotate baseUp around forward by rollR
    glm::vec3 upRolled = glm::normalize(
    baseUp * cos(rollR) +
    right  * sin(rollR)
    );
 
    // View matrix that looks at origin
    glm::mat4 view = glm::lookAt(
       cameraPos,
       glm::vec3(0.0f,0.0f,0.0f),
       upRolled
     );
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Draw the sun
    glm::mat4 sunModelTrans = glm::mat4(1.0f);
    sunModelTrans = glm::scale(sunModelTrans, glm::vec3(0.4f,0.4f,0.4f));

    // This passes the matrix to the shader for the uniform variable transform
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sunModelTrans));

    //Dont draw clouds
    glUniform1i(useCloudsLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture);

    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());
    glUniform1f(glGetUniformLocation(shader,"Ka"), 0.2f);

    
    // Draw the Earth 
    if (!pause) {
        earthAngle += earthDeltaAngle;
    }
    
    // Starting Identity Matrix
    glm::mat4 earthModelTrans = sunModelTrans;

    // Rotate the Earth around the Sun
    earthModelTrans = glm::rotate(earthModelTrans, earthAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate the Earth away from the Sun (Orbit radius)
    earthModelTrans = glm::translate(earthModelTrans, glm::vec3(2.5f, 0.0f, 0.0f));

    // Scale the earth down, this is working off of the suns already existing scale
    earthModelTrans = glm::scale(earthModelTrans, glm::vec3(0.8f,0.8f,0.8f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(earthModelTrans));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, earthTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);

    // Turn clouds on:
    glUniform1i(useCloudsLoc, 1);
    glUniform1f(timeLoc, now);

    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    // Turning off clouds so other objects dont have cloud texture drawn on them
    glUniform1i(useCloudsLoc, 0);

    
    ////////////////////////////////////////////////////////////////////////////
    // Draw the Moon 
    if (!pause) {
        moonAngle += moonDeltaAngle;
    }

    // Rotate the Moon around the Earth
    earthModelTrans = glm::rotate(earthModelTrans, moonAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate the Moon away from the Earth
    earthModelTrans = glm::translate(earthModelTrans, glm::vec3(2.0f, 0.0f, 0.0f));
    earthModelTrans = glm::scale(earthModelTrans, glm::vec3(0.5f,0.5f,0.5f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(earthModelTrans));

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(useCloudsLoc, 0);
    glBindTexture(GL_TEXTURE_2D, moonTexture);
    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    ////////////////////////////////////////////////////////////////////////////
    // Draw Mars 
    if (!pause) {
        marsAngle += marsDeltaAngle;
    }

    // Starting Identity Matrix
    glm::mat4 marsModelTrans = sunModelTrans;

    // Rotate Mars around the Sun
    marsModelTrans = glm::rotate(marsModelTrans, marsAngle, glm::vec3(0.0f, 0.0f, 1.0f));
  
    // Translate Mars away from the Sun
    marsModelTrans = glm::translate(marsModelTrans, glm::vec3(3.5f, 0.5f, 0.0f));
  
    // Scale mars down
    marsModelTrans = glm::scale(marsModelTrans, glm::vec3(0.3f,0.3f,0.3f));
  
  
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(marsModelTrans));
  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, marsTexture);
    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    ////////////////////////////////////////////////////////////////////////////
    // Draw jupiter 
    if (!pause) {
        jupiterAngle += jupiterDeltaAngle;
    }

    // Starting Identity Matrix
    glm::mat4 jupiterModelTrans = sunModelTrans;

    // Rotate Mars around the Sun
    jupiterModelTrans = glm::rotate(jupiterModelTrans, jupiterAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate Mars away from the Sun (Orbit radius)
    jupiterModelTrans = glm::translate(jupiterModelTrans, glm::vec3(4.5f, 1.0f, 0.0f));

    // Scale mars down
    jupiterModelTrans = glm::scale(jupiterModelTrans, glm::vec3(0.6f,0.6f,0.6f));


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(jupiterModelTrans));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, jupiterTexture);
    glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount());

    ////////////////////////////////////////////////////////////////////////////
    // Draw Venus
    if (!pause) {
        venusAngle += venusDeltaAngle;
    }

    // Starting Identity Matrix
    glm::mat4 venusModelTrans = sunModelTrans;

    // Rotate Venus around the Sun
    venusModelTrans = glm::rotate(venusModelTrans, venusAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate Venus away from the Sun
    venusModelTrans = glm::translate(venusModelTrans, glm::vec3(4.0f, 1.0f, 0.0f));

    // Scale Venus down
    venusModelTrans = glm::scale(venusModelTrans, glm::vec3(0.35f,0.35f,0.35f));


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(venusModelTrans));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, venusTexture);
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
            case SDLK_c:  // speed up earth
                earthDeltaAngle += 0.005f;
                break;
            case SDLK_z:  // slow down earth
                earthDeltaAngle -= 0.005;
                if (earthDeltaAngle < 0) {
                    earthDeltaAngle = 0;
                  }
                break;
            case SDLK_x:  // reset earth to default
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
            case SDLK_a:  
                camYaw -= yawSpeed   * deltaTime;  
                break;
            case SDLK_d: 
                camYaw += yawSpeed   * deltaTime;  
                break;
            case SDLK_w:    
                camPitch += pitchSpeed * deltaTime; 
                break;
            case SDLK_s:  
                camPitch -= pitchSpeed * deltaTime; 
                break;
            case SDLK_q:
                camRoll += rollSpeed * deltaTime;
                break;
            case SDLK_e:
                camRoll -= rollSpeed * deltaTime;
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
