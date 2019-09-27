#pragma once

#include "Renderer.h"


namespace jcl {


/*/
    Jackal Renderer is the front end rendering engine, whose sole responsibility
    is to run the application as programmed for the game. Graphics Programmers mainly
    work here with freedom of the hardware graphics API, solely to implement lighting
    techniques, animation, particles, physics, gobos, shadows, post-processing, etc; in short,
    figure out what they should be rendering. Different renderers can be implemented for different
    games, but the underlying workhorse will always be the BackendRenderer.
*/
class JackalRenderer
{
public:

    void init() {
    }

    void cleanUp() { }

    void render() {
        beginFrame();
        endFrame();
    }
    
    void update() { }

    void pushCommandsCommands() { }

private:

    void beginFrame() { }
    void endFrame() { }

    gfx::BackendRenderer* m_pBackend;
};
} //