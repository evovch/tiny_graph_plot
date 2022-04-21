#pragma once

struct GLFWwindow;

namespace tiny_graph_plot {

enum class action_t {
    ACT_NO_ACT,
    ACT_PAN,
    ACT_ZOOM,
    ACT_ZOOM_F, //!< Fixed aspect ratio
    ACT_ZOOM_X,
    ACT_ZOOM_Y,
    ACT_RECT
};

class UserWindow
{
protected:
    UserWindow(GLFWwindow* window, const unsigned int w, const unsigned int h);
    virtual ~UserWindow(void) = 0 {};
private:
    void SetCallbacks(void) const;
public:
    GLFWwindow* const GetWindow(void) const { return _window; }
    void framebuffer_size_event(int width, int height);
    void window_pos_event(int xpos, int ypos);
    void window_iconify_event(int iconified);
    void window_maximize_event(int maximized);
    void window_focus_event(int focused);
    void window_refresh_event();
    void key_event(int key, int scancode, int action, int mods);
    void mouse_button_event(int button, int action, int mods);
    void mouse_pos_event(double xs, double ys_inv);
    void scroll_event(double xoffset, double yoffset);
protected:
    virtual void CenterView(const double xs,  const double ys) = 0;
    virtual void Pan       (const double xs,  const double ys) = 0;
    virtual void Zoom      (const double xs,  const double ys) = 0;
    virtual void ZoomF     (const double xs,  const double ys) = 0;
    virtual void ZoomX     (const double xs,  const double ys) = 0;
    virtual void ZoomY     (const double xs,  const double ys) = 0;
    virtual void ZoomTo    (const double xs0, const double ys0,
                            const double xs1, const double ys1) = 0;
    virtual void ResetCamera(void) = 0;
    virtual void SetPrevViewport(void) = 0;
    virtual void FixedAspRatCamera(void) = 0;
    virtual bool PointerInFrame(const double xs, const double ys) const = 0;
    virtual void ClampToFrame(const double xs, const double ys,
                              double& o_xs, double& o_ys) const = 0;
    virtual void SaveStartState(void) = 0;
    virtual void ToggleGraphVisibility(const int iGraph) const = 0;
    virtual void Clear(void) const = 0;
    virtual void Reshape(int p_width, int p_height) = 0;
    virtual void Draw(void) /*const*/ = 0;
    virtual void DrawCursor(const double xs, const double ys) const = 0;
    virtual void DrawSelRectangle(const double xs0, const double ys0,
                                  const double xs1, const double ys1) const = 0;
    virtual void DrawCircles(const double xs, const double ys) const = 0;
    virtual void UpdateTexTextCur(const double xs, const double ys) = 0;
    virtual void UpdateTexTextRef(const double xs, const double ys) = 0;
protected:
    GLFWwindow* _window = nullptr;
    int _window_w;
    int _window_h;
    bool _mouse_moved = false;
    action_t _cur_action = action_t::ACT_NO_ACT;
    double _xs_prev; //!< At the previous position
    double _ys_prev; //!< At the previous position
    double _xs_start; //!< At mouse press
    double _ys_start; //!< At mouse press
};

} // end of namespace tiny_graph_plot
