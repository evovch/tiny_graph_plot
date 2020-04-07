#include "canvas_manager.h"

#include "glew_routines.h"
#include "glfw_callback_functions.h"

namespace tiny_graph_plot {

template<typename T>
CanvasManager<T>::CanvasManager(void) {
    glfwSetErrorCallback(tiny_graph_plot::glfw_callback_functions::error_callback_glfw);
    if (!glfwInit()) {
        fprintf(stderr, "GLFW: error: failed to initialize.\n\nAborting.\n");
        exit(EXIT_FAILURE);
    }
}

template<typename T>
CanvasManager<T>::~CanvasManager(void) {
    for (auto* canv : _canvases) {
        delete canv;
    }
    _canvases.clear();
    glfwTerminate();
}

template<typename T>
Canvas<T>& CanvasManager<T>::CreateCanvas(const char* name,
    const unsigned int w, const unsigned int h,
    const unsigned int x, const unsigned int y) {
	GLFWwindow* window = glfwCreateWindow(w, h, name, NULL, NULL);
	if (!window) {
		fprintf(stderr, "GLFW: error: failed to create a window.\n\nAborting.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
    glfwSetWindowPos(window, x, y);
    glfwMakeContextCurrent(window);

    if (!_glew_initialized) {
        tiny_graph_plot::init_glew();
        _glew_initialized = true;
    }

    glfwHideWindow(window);

	Canvas<T>* new_canv = new Canvas<T>(window, w, h);
	_canvases.push_back(new_canv);
	return *new_canv;
}

template<typename T>
void CanvasManager<T>::WaitForTheWindowsToClose(void) {
    if (_canvases.size() < 1) return;
    GLFWwindow* const first_window = _canvases.at(0)->GetWindow();
    while (!glfwWindowShouldClose(first_window)) {
        glfwWaitEvents();
    }
}

template class CanvasManager<float>;
template class CanvasManager<double>;

} // end of namespace tiny_graph_plot
