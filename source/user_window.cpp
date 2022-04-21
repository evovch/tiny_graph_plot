#include "user_window.h"

#include "glfw_callback_functions.h"

//#define DEBUG_WIN_ACTIONS
#define SET_CONTEXT

#ifdef DEBUG_WIN_ACTIONS
#include <cstdio>
#endif

namespace tiny_graph_plot {

UserWindow::UserWindow(GLFWwindow* window, const unsigned int w, const unsigned int h) :
    _window(window), _window_w(w), _window_h(h)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    glfwSetWindowUserPointer(_window, reinterpret_cast<void*>(this));
    this->SetCallbacks();
}

void UserWindow::SetCallbacks(void) const {
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::SetCallbacks\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    glfwSetFramebufferSizeCallback(_window, glfw_callback_functions::framebuffer_size_callback);
    glfwSetWindowRefreshCallback(_window, glfw_callback_functions::window_refresh_callback);
    glfwSetKeyCallback(_window, glfw_callback_functions::key_callback);
    glfwSetMouseButtonCallback(_window, glfw_callback_functions::mouse_button_callback);
    glfwSetCursorPosCallback(_window, glfw_callback_functions::mouse_pos_callback);
    glfwSetScrollCallback(_window, glfw_callback_functions::scroll_callback);
    glfwSetWindowPosCallback(_window, glfw_callback_functions::window_pos_callback);
    glfwSetWindowIconifyCallback(_window, glfw_callback_functions::window_iconify_callback);
    glfwSetWindowMaximizeCallback(_window, glfw_callback_functions::window_maximize_callback);
    glfwSetWindowFocusCallback(_window, glfw_callback_functions::window_focus_callback);
}

void UserWindow::framebuffer_size_event(int width, int height) {
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::framebuffer_size_event: %d, %d\n", width, height);
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    if (width == 0 && height == 0) return; // Window minimized
    _window_w = width;
    _window_h = height;
    this->Reshape(width, height);
}

void UserWindow::window_pos_event(int xpos, int ypos) {
#ifdef DEBUG_WIN_ACTIONS
    printf("GLFW: window move: %d, %d\n", xpos, ypos);
#endif
}

void UserWindow::window_iconify_event(int iconified) {
#ifdef DEBUG_WIN_ACTIONS
    if (iconified) {
        printf("GLFW: window iconified\n");
    } else {
        printf("GLFW: window restored\n");
    }
#endif
}

void UserWindow::window_maximize_event(int maximized) {
#ifdef DEBUG_WIN_ACTIONS
    if (maximized) {
        printf("GLFW: window maximized\n");
    } else {
        printf("GLFW: window restored\n");
    }
#endif
}

void UserWindow::window_focus_event(int focused) {
#ifdef DEBUG_WIN_ACTIONS
    if (focused) {
        printf("GLFW: window gained focus\n");
    } else {
        printf("GLFW: window lost focus\n");
    }
#endif
}

void UserWindow::window_refresh_event() {
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::window_refresh_event\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    this->Clear();
    this->Draw();
    glfwSwapBuffers(_window);
}

void UserWindow::key_event(int key, int scancode, int action, int mods) {
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::key_event\n");
#endif
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_F:
            this->ResetCamera();
            this->window_refresh_event();
            break;
        case GLFW_KEY_Z:
            this->SetPrevViewport();
            this->window_refresh_event();
            break;
        case GLFW_KEY_S:
            this->FixedAspRatCamera();
            this->window_refresh_event();
            break;
        case GLFW_KEY_F1:
            this->ExportSnapshot();
            break;

        case GLFW_KEY_GRAVE_ACCENT: this->ToggleGraphVisibility(0); this->window_refresh_event(); break;
        case GLFW_KEY_1: this->ToggleGraphVisibility(1);  this->window_refresh_event(); break;
        case GLFW_KEY_2: this->ToggleGraphVisibility(2);  this->window_refresh_event(); break;
        case GLFW_KEY_3: this->ToggleGraphVisibility(3);  this->window_refresh_event(); break;
        case GLFW_KEY_4: this->ToggleGraphVisibility(4);  this->window_refresh_event(); break;
        case GLFW_KEY_5: this->ToggleGraphVisibility(5);  this->window_refresh_event(); break;
        case GLFW_KEY_6: this->ToggleGraphVisibility(6);  this->window_refresh_event(); break;
        case GLFW_KEY_7: this->ToggleGraphVisibility(7);  this->window_refresh_event(); break;
        case GLFW_KEY_8: this->ToggleGraphVisibility(8);  this->window_refresh_event(); break;
        case GLFW_KEY_9: this->ToggleGraphVisibility(9);  this->window_refresh_event(); break;
        case GLFW_KEY_0: this->ToggleGraphVisibility(10); this->window_refresh_event(); break;

        default:
            break;
        }
    }
}

void UserWindow::mouse_button_event(int button, int action, int mods) {
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    double xs; double ys_inv;
    glfwGetCursorPos(_window, &xs, &ys_inv);
    const double ys = (double)_window_h - ys_inv;
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::mouse_button_event: % 0.6f\t% 0.6f\n", xs, ys);
#endif

    if (action == GLFW_PRESS &&
        button == GLFW_MOUSE_BUTTON_LEFT &&
        mods == GLFW_MOD_CONTROL) {
        this->UpdateTexTextRef(xs, ys);
        this->UpdateTexTextCur(xs, ys);
        this->Clear();
        this->Draw();
        this->DrawCursor(xs, ys);
        this->DrawCircles(xs, ys);
        glfwSwapBuffers(_window);
        return;
    }

    if (action == GLFW_PRESS) {
        if (!this->PointerInFrame(xs, ys)) return;
        this->SaveStartState();
        _xs_start = xs; _ys_start = ys;
        _xs_prev = xs; _ys_prev = ys;
        _mouse_moved = false;
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            _cur_action = action_t::ACT_PAN;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (mods == GLFW_MOD_SHIFT) {
                _cur_action = action_t::ACT_ZOOM_Y;
            } else if (mods == GLFW_MOD_CONTROL) {
                _cur_action = action_t::ACT_ZOOM_X;
            } else if (mods == GLFW_MOD_ALT) {
                _cur_action = action_t::ACT_ZOOM;
            } else {
                _cur_action = action_t::ACT_ZOOM_F;
            }
        } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (mods != GLFW_MOD_SHIFT && mods != GLFW_MOD_CONTROL &&
                mods != GLFW_MOD_ALT) {
                _cur_action = action_t::ACT_RECT;
            }
        }
    }
    else if (action == GLFW_RELEASE)
    {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && !_mouse_moved &&
            _cur_action == action_t::ACT_PAN) {
            this->CenterView(xs, ys);
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && _mouse_moved &&
            _cur_action == action_t::ACT_RECT) {
            this->ZoomTo(_xs_start, _ys_start, xs, ys);
        }
        _cur_action = action_t::ACT_NO_ACT;
        this->window_refresh_event();
    }
}

void UserWindow::mouse_pos_event(double xs, double ys_inv) {
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    const double ys = (double)_window_h - ys_inv;
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::mouse_pos_event: % 0.6f\t% 0.6f\n", xs, ys);
#endif

    _mouse_moved = true;

    const int xs_i = (int)xs;
    const int ys_i = (int)ys;

    switch (_cur_action) {
    case action_t::ACT_NO_ACT: {
        double xs_; double ys_;
        this->ClampToFrame(xs, ys, xs_, ys_);
        this->UpdateTexTextCur(xs_, ys_);
        this->Clear();
        this->Draw();
        this->DrawCursor(xs_, ys_);
        this->DrawCircles(xs_, ys_);
        glfwSwapBuffers(_window);
        return;
        break; }
    case action_t::ACT_PAN:    this->Pan(xs, ys);
        break;
    case action_t::ACT_ZOOM:   this->Zoom(xs, ys);
        break;
    case action_t::ACT_ZOOM_F: this->ZoomF(xs, ys);
        break;
    case action_t::ACT_ZOOM_X: this->ZoomX(xs, ys);
        break;
    case action_t::ACT_ZOOM_Y: this->ZoomY(xs, ys);
        break;
    case action_t::ACT_RECT: {
        this->Clear();
        this->Draw();
        this->DrawSelRectangle(_xs_start, _ys_start, xs, ys);
        glfwSwapBuffers(_window);
        return;
        break; }
    default:
        break;
    }

    _xs_prev = xs; _ys_prev = ys;

    this->window_refresh_event();
}

void UserWindow::scroll_event(double xoffset, double yoffset) {
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
#ifdef DEBUG_WIN_ACTIONS
    printf("UserWindow::scroll_event: % 0.6f\t% 0.6f\n", xoffset, yoffset);
#endif

//TODO implement

//    double xs; double ys_inv;
//    glfwGetCursorPos(_window, &xs, &ys_inv);
//    const double ys = (double)_window_h - ys_inv;
//
//    _xs_start = xs; _ys_start = ys;
//    constexpr double s = 1.0;
//    this->ZoomF(_xs_start + s * xoffset, _ys_start + s * yoffset);
//
//    this->window_refresh_event();
}

} // end of namespace tiny_graph_plot
