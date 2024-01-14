#pragma once

#include <vector>
#include <string>
#include "tiny_gl_text_renderer/text_renderer.h"
#include "tiny_gl_text_renderer/data_types.h"
#include "tiny_gl_text_renderer/colors.h"
#include "tiny_gl_text_renderer/mat4.h"
#include "xy_range.h"
#include "grid.h"
#include "user_window.h"

struct GLFWwindow;

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::color_t;
using tiny_gl_text_renderer::Vec4f;
using tiny_gl_text_renderer::Vec4d;
using tiny_gl_text_renderer::Mat4f;
using tiny_gl_text_renderer::Mat4d;

typedef unsigned int GLuint;

template<typename T> class CanvasManager;
template<typename T> class Drawable;
template<typename T> class Graph;
template<typename T, typename VALUETYPE> class Histogram1d;
class SizeInfo;

template<typename T>
class Canvas : public UserWindow
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
    friend class CanvasManager<T>;
private:
    explicit Canvas(GLFWwindow* window, const unsigned int w, const unsigned int h);
    virtual ~Canvas() override;
    Canvas(const Canvas& other) = delete;
    Canvas(Canvas&& other) = delete;
    Canvas& operator=(const Canvas& other) = delete;
    Canvas& operator=(Canvas&& other) = delete;
public:
    void AddGraph(const Graph<T>& p_graph);
    void AddHistogram(const Histogram1d<T, unsigned long>& p_histo);
    void Show();
    virtual void Draw() /*const*/ override;
private:
    void Init();
    virtual void Clear() const override;
    virtual void Reshape(int p_width, int p_height) override;
    void PrintCursorValues(const double xs, const double ys) const;
    __forceinline void SwitchToFullWindow() const;
    __forceinline void SwitchToFrame() const;
    void AllocateBuffersForFixedSizedData() const;
    void SendFixedIndicesToGPU() const;
    int SendGridToGPU();
    void DrawGrid() const;
    void DrawAxes() const;
    void DrawVref() const;
    void SendFrameVerticesToGPU() const;
    void FillInFrame() const;
    void DrawFrame() const;
    void SendDrawableToGPU(const Drawable<T>* const p_graph, const SizeInfo& p_offset) const;
    void DrawDrawable     (const Drawable<T>* const p_graph, const SizeInfo& p_offset) const;
    virtual void DrawCursor(const double xs, const double ys) const override;
    virtual void DrawSelRectangle(const double xs0, const double ys0,
                          const double xs1, const double ys1) const override;
    virtual void DrawCircles(const double xs, const double ys) const override;
    virtual void UpdateTexTextCur(const double xs, const double ys) override;
    virtual void UpdateTexTextRef(const double xs, const double ys) override;
    void UpdateTexTextGridSize();
    void UpdateTexAxesValues();
private:
    // VAOs, VBOs, IBOs
    GLuint _vaoID_grid;     //!< 1. Grid
    GLuint _vboID_grid;
    GLuint _iboID_grid_w;
    GLuint _iboID_grid_coarse_w;
    GLuint _vaoID_axes;     //!< 2. Axes
    GLuint _vboID_axes;
    GLuint _iboID_axes_w;
    GLuint _vaoID_vref;     //!< 3. Vref
    GLuint _vboID_vref;
    GLuint _iboID_vref_w;
    GLuint _vaoID_frame;    //!< 4. Frame
    GLuint _vboID_frame;
    GLuint _iboID_frame_onscr_w;
    GLuint _iboID_frame_onscr_q;
    GLuint _vaoID_graphs;   //!< 5. Graphs
    GLuint _vboID_graphs;
    GLuint _iboID_graphs_w;
    GLuint _iboID_graphs_m;
    GLuint _vaoID_cursor;   //!< 6. Cursor
    GLuint _vboID_cursor;
    GLuint _iboID_cursor_onscr_w;
    GLuint _vaoID_sel;      //!< 7. Select rectangle
    GLuint _vboID_sel;
    GLuint _iboID_sel_q;
    GLuint _iboID_sel_w;
    GLuint _vaoID_c;        //!< 8. Circles
    GLuint _vboID_c;
    GLuint _iboID_c;
    // Programs
    GLuint _progID_sel_q;
    GLuint _progID_onscr_q;
    //GLuint _progID_tr;
    GLuint _progID_w;
    GLuint _progID_onscr_w;
    GLuint _progID_m;
    GLuint _progID_c;
    // Uniforms
    // --------------------
    GLuint _fr_bg_unif_onscr_q; //!< In frame background color
    // --------------------
    GLuint _s2v_unif_sel_q; //!< screen-to-viewport matrix
    GLuint _v2c_unif_sel_q; //!< viewport-to-clip matrix
    GLuint _s2c_unif_sel_q; //!< screen-to-clip matrix
    GLuint _r2c_unif_sel_q; //!< visrange-to-clip matrix
    // --------------------------
    ////GLuint _s2v_unif_onscr_q; //!< screen-to-viewport matrix
    ////GLuint _v2c_unif_onscr_q; //!< viewport-to-clip matrix
    GLuint _s2c_unif_onscr_q;     //!< screen-to-clip matrix
    ////GLuint _v2c_unif_onscr_q; //!< visrange-to-clip matrix
    // -------------------
    //GLuint _s2v_unif_tr; //!< screen-to-viewport matrix
    //GLuint _v2c_unif_tr; //!< viewport-to-clip matrix
    //GLuint _s2c_unif_tr; //!< screen-to-clip matrix
    //GLuint _r2c_unif_tr; //!< visrange-to-clip matrix
    // --------------------------
    ////GLuint _s2v_unif_onscr_w; //!< screen-to-viewport matrix
    ////GLuint _v2c_unif_onscr_w; //!< viewport-to-clip matrix
    GLuint _s2c_unif_onscr_w;     //!< screen-to-clip matrix
    ////GLuint _r2c_unif_onscr_w; //!< visrange-to-clip matrix
    // ----------------
    GLuint _s2v_unif_w;  //!< screen-to-viewport matrix
    GLuint _v2c_unif_w;  //!< viewport-to-clip matrix
    GLuint _s2c_unif_w;  //!< screen-to-clip matrix
    GLuint _r2c_unif_w;  //!< visrange-to-clip matrix
    // ----------------
    GLuint _s2v_unif_m; //!< screen-to-viewport matrix
    GLuint _v2c_unif_m; //!< viewport-to-clip matrix
    GLuint _s2c_unif_m; //!< screen-to-clip matrix
    GLuint _r2c_unif_m; //!< visrange-to-clip matrix
    // ----------------
    //GLuint _s2v_unif_c; //!< screen-to-viewport matrix
    GLuint _v2c_unif_c; //!< viewport-to-clip matrix
    //GLuint _s2c_unif_c; //!< screen-to-clip matrix
    GLuint _r2c_unif_c; //!< visrange-to-clip matrix
    GLuint _circle_r_unif_c;
    // ----------------
private:
    std::vector<const Graph<T>*> _graphs;
    std::vector<const Histogram1d<T, unsigned long>*> _histograms;
    XYrange<float> _total_xy_range;
    XYrange<float> _visible_range;
    XYrange<float> _visible_range_start; //!< At mouse press
private:
    Grid<float> _grid;
private:
    virtual void CenterView(const double xs,  const double ys) override;
    virtual void Pan       (const double xs,  const double ys) override;
    virtual void Zoom      (const double xs,  const double ys) override;
    virtual void ZoomF     (const double xs,  const double ys) override; //!< Fixed aspect ratio
    virtual void ZoomX     (const double xs,  const double ys) override;
    virtual void ZoomY     (const double xs,  const double ys) override;
    virtual void ZoomTo    (const double xs0, const double ys0,
                            const double xs1, const double ys1) override;
    virtual void ResetCamera() override;
    virtual void SetPrevViewport() override;
    virtual void FixedAspRatCamera() override;
    virtual void ExportSnapshot() override;
    virtual bool PointerInFrame(const double xs, const double ys) const override;
    virtual void ClampToFrame(const double xs, const double ys,
                              double& o_xs, double& o_ys) const override;
    virtual void SaveStartState() override;
    virtual void ToggleGraphVisibility(const int iGraph) const override;
    void ExportPNG(const char* const dir, const char* const filename) const;
    void UpdateMatricesReshape();
    void UpdateMatricesPanZoom();
    // Transformation matrices are stored using single precision floats,
    // thus performing transformations in double precision make no sense.
    __forceinline Vec4f TransformToVisrange(const double xs, const double ys,
        Vec4f* const o_in_clip_space = nullptr,
        Vec4f* const o_in_viewport_space = nullptr) const;
    __forceinline Vec4f TransformToVisrange(const Vec4f& in_screen_space,
        Vec4f* const o_in_clip_space = nullptr,
        Vec4f* const o_in_viewport_space = nullptr) const;
    //__forceinline Vec4d TransformToVisrangeHP(const double xs, const double ys,
    //    Vec4d* const o_in_clip_space = nullptr) const;
    //__forceinline Vec4d TransformToVisrangeHP(const Vec4d& in_screen_space,
    //    Vec4d* const o_in_clip_space = nullptr) const;
private:
    // Direct matrices
    Mat4f _screen_to_viewport;
    Mat4f _viewport_to_clip;
    Mat4f _screen_to_clip;
    Mat4f _clip_to_visrange;
    // Inverse matrices
    Mat4f _viewport_to_screen;
    Mat4f _clip_to_viewport;
    Mat4f _clip_to_screen;
    Mat4f _visrange_to_clip;
    // For intermediate high-precision calculations
    //Mat4d _screen_to_viewport_hp;
    //Mat4d _viewport_to_clip_hp;
    //Mat4d _screen_to_clip_hp;
    //Mat4d _clip_to_visrange_hp;
    //Mat4d _screen_to_visrange_hp;
    // ===========================================================================
public:
    T _ref_x;
    // ===========================================================================
public:
    void UpdateSizeLimits();
    // ===========================================================================
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    tiny_gl_text_renderer::TextRenderer& GetTextRenderer() {
        return _text_rend;
    }
    void FinalizeTextRenderer();
private:
    tiny_gl_text_renderer::TextRenderer _text_rend;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    static constexpr unsigned int _n_x_axis_value_labels_max = 20u;
    static constexpr unsigned int _n_y_axis_value_labels_max = 20u;
    size_t _x_axis_title_label_idx = 0u;
    size_t _y_axis_title_label_idx = 1u;
    size_t _grid_params_label_idx = 2u;
    //size_t _hint_label_idx = 3u;
    size_t _x_axis_values_lables_start_idx = 4u;
    size_t _y_axis_values_lables_start_idx = 24u;
    size_t _labels_start_idx = 44u;
    // ===========================================================================
public: // visual parameters
    void SetXaxisTitle(const char* title) { _x_axis_title = std::string(title); }
    void SetYaxisTitle(const char* title, const bool rotated = false) {
        _y_axis_title = std::string(title); _y_axis_title_rotated = rotated; }
    void EnableHgrid()    { _enable_hgrid   = true; }
    void EnableVgrid()    { _enable_vgrid   = true; }
    void EnableAxes()     { _enable_axes    = true; }
    void EnableVref()     { _enable_vref    = true; }
    void EnableFrame()    { _enable_frame   = true; }
    void EnableCursor()   { _enable_cursor  = true; }
    void EnableCircles()  { _enable_circles = true; }
    void DisableHgrid()   { _enable_hgrid   = false; }
    void DisableVgrid()   { _enable_vgrid   = false; }
    void DisableAxes()    { _enable_axes    = false; }
    void DisableVref()    { _enable_vref    = false; }
    void DisableFrame()   { _enable_frame   = false; }
    void DisableCursor()  { _enable_cursor  = false; }
    void DisableCircles() { _enable_circles = false; }
    // Color settings ------------------------------------------------------------
    void SetDarkColorScheme();
    void SetBrightColorScheme();
    void SetBackgroundColor(const color_t& color);
    void SetInFrameBackgroundColor(const color_t& color);
    void SetHGridFineColor  (const color_t& color) { _grid.SetHGridFineColor(color); }
    void SetVGridFineColor  (const color_t& color) { _grid.SetVGridFineColor(color); }
    void SetHGridCoarseColor(const color_t& color) { _grid.SetHGridCoarseColor(color); }
    void SetVGridCoarseColor(const color_t& color) { _grid.SetVGridCoarseColor(color); }
    void SetAxesColor       (const color_t& color) { _axes_line_color  = color; }
    void SetVrefColor       (const color_t& color) { _vref_line_color  = color; }
    void SetFrameColor      (const color_t& color) { _frame_line_color = color; }
    void SetCursorColor     (const color_t& color) { _cursor_color     = color; }
    void SetTextColor       (const color_t& color) { _gen_text_color   = color; }
    // ---------------------------------------------------------------------------
    void SetHGridFineLineWidth  (const float width) { _grid.SetHGridFineLineWidth(width); }
    void SetVGridFineLineWidth  (const float width) { _grid.SetVGridFineLineWidth(width); }
    void SetHGridCoarseLineWidth(const float width) { _grid.SetHGridCoarseLineWidth(width); }
    void SetVGridCoarseLineWidth(const float width) { _grid.SetVGridCoarseLineWidth(width); }
    void SetAxesLineWidth       (const float width) { _axes_line_width   = width; }
    void SetVrefLineWidth       (const float width) { _vref_line_width   = width; }
    void SetFrameLineWidth      (const float width) { _frame_line_width  = width; }
    void SetCursorLineWidth     (const float width) { _cursor_line_width = width; }
    void SetFontSize(const float size) { _font_size = size; }
    void SetCircleRadius(const unsigned int r) { _circle_r = r; }
    void SetMarginXleft(const unsigned int w_in_pix) {
        _margin_xl_pix = w_in_pix; this->UpdateSizeLimits(); }
    void SetMarginXright(const unsigned int w_in_pix) {
        _margin_xr_pix = w_in_pix; this->UpdateSizeLimits(); }
    void SetMarginYbottom(const unsigned int h_in_pix) {
        _margin_yb_pix = h_in_pix; this->UpdateSizeLimits(); }
    void SetMarginYtop(const unsigned int h_in_pix) {
        _margin_yt_pix = h_in_pix; this->UpdateSizeLimits(); }
    void SetAllMargins(const unsigned int xl_in_pix, const unsigned int xr_in_pix,
        const unsigned int yb_in_pix, const unsigned int yt_in_pix) {
        _margin_xl_pix = xl_in_pix; _margin_xr_pix = xr_in_pix;
        _margin_yb_pix = yb_in_pix; _margin_yt_pix = yt_in_pix;
        this->UpdateSizeLimits();
    }
private: // visual parameters
    std::string _x_axis_title = "x axis";
    std::string _y_axis_title = "y axis";
    bool _y_axis_title_rotated = true;
    bool _enable_hgrid   = true;
    bool _enable_vgrid   = true;
    bool _enable_axes    = true;
    bool _enable_vref    = true;
    bool _enable_frame   = true;
    bool _enable_cursor  = true;
    bool _enable_circles = true;
    color_t _background_color = tiny_gl_text_renderer::colors::gray1;
    color_t _in_frame_bg_color = tiny_gl_text_renderer::colors::gray05;
    color_t _axes_line_color  = tiny_gl_text_renderer::colors::gray75;
    color_t _vref_line_color  = tiny_gl_text_renderer::colors::olive;
    color_t _frame_line_color = tiny_gl_text_renderer::colors::gray5;
    color_t _cursor_color     = tiny_gl_text_renderer::colors::white;
    color_t _gen_text_color   = tiny_gl_text_renderer::colors::white;
    float _axes_line_width   = 3.0f;
    float _vref_line_width   = 3.0f;
    float _frame_line_width  = 3.0f;
    float _cursor_line_width = 1.0f;
    float _font_size = 1.0f;
    unsigned int _circle_r = 10u;
    unsigned int _margin_xl_pix = 280u;
    unsigned int _margin_xr_pix = 22u;
    unsigned int _margin_yb_pix = 34u;
    unsigned int _margin_yt_pix = 22u;
    static constexpr int _h_offset = 2;
    static constexpr int _v_offset = 2;
    // ===========================================================================
};

} // end of namespace tiny_graph_plot
