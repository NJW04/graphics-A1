#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>

#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();
    GeometryData geometry;


    void initGL();
    void render();
    bool handleEvent(SDL_Event e);
    void cleanup();
    void loadTexture(const std::string &path, GLuint &outTexID);
    

private:
    SDL_Window* sdlWin;

    GLuint vao;
    GLuint shader;
    GLuint vertexBuffer;
};

#endif
