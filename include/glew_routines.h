#pragma once

#include <cstdio>
#include <cstdlib>

#include "GL/glew.h"

namespace tiny_graph_plot {

void init_glew(void) {
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW: error: failed to initialize: %s\nAborting.\n",
            glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
#ifndef SUPPRESS_ALL_OUTPUT
    printf("Using GLEW '%s'\n", glewGetString(GLEW_VERSION));
    printf("Version of OpenGL detected in the system: '%s'\n",
        glGetString(GL_VERSION));
#endif
}

} // end of namespace tiny_graph_plot
