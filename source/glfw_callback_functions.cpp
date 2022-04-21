#include "glfw_callback_functions.h"

#include <cstdio>

#include "user_window.h"

namespace tiny_graph_plot {
namespace glfw_callback_functions {

void error_callback_glfw(int error, const char* description) {
    printf("GLFW: error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->framebuffer_size_event(width, height);
}

void window_refresh_callback(GLFWwindow* window) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->window_refresh_event();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->key_event(key, scancode, action, mods);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->mouse_button_event(button, action, mods);
}

void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->mouse_pos_event(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->scroll_event(xoffset, yoffset);
}

void window_pos_callback(GLFWwindow* window, int xpos, int ypos) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->window_pos_event(xpos, ypos);
}

void window_iconify_callback(GLFWwindow* window, int iconified) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->window_iconify_event(iconified);
}

void window_maximize_callback(GLFWwindow* window, int maximized) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->window_maximize_event(maximized);
}

void window_focus_callback(GLFWwindow* window, int focused) {
    UserWindow* userwin = reinterpret_cast<UserWindow*>(glfwGetWindowUserPointer(window));
    if (userwin) userwin->window_focus_event(focused);
}

} // end of namespace glfw_callback_functions
} // end of namespace tiny_graph_plot
