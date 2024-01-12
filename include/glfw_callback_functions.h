#pragma once

#include "GLFW/glfw3.h"

namespace tiny_graph_plot
{
namespace glfw_callback_functions
{

void error_callback_glfw(int error, const char* description);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_refresh_callback(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void window_pos_callback(GLFWwindow* window, int xpos, int ypos);
void window_iconify_callback(GLFWwindow* window, int iconified);
void window_maximize_callback(GLFWwindow* window, int maximized);
void window_focus_callback(GLFWwindow* window, int focused);

} // end of namespace glfw_callback_functions
} // end of namespace tiny_graph_plot
