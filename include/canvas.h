#pragma once

#include <vector>
#include <string>

#include "tiny_gl_text_renderer/colors.h"
#include "tiny_gl_text_renderer/data_types.h"
#include "tiny_gl_text_renderer/mat4.h"
#include "tiny_gl_text_renderer/text_renderer.h"
#include "buffer_set.h"
#include "grid.h"
#include "shader_program.h"
#include "user_window.h"
#include "xy_range.h"

struct GLFWwindow;

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::color_t;
using tiny_gl_text_renderer::Vec4f;
using tiny_gl_text_renderer::Vec4d;
using tiny_gl_text_renderer::Mat4f;
using tiny_gl_text_renderer::Mat4d;

typedef unsigned int GLuint;
typedef int GLint;

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
    void SwitchToFullWindow() const; //inline? //__forceinline?
    void SwitchToFrame() const; //inline? //__forceinline?
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
    virtual void DrawCursor      (const double xs,  const double ys) const override;
    virtual void DrawSelRectangle(const double xs0, const double ys0,
                                  const double xs1, const double ys1) const override;
    virtual void DrawCircles     (const double xs,  const double ys) const override;
    virtual void UpdateTexTextCur(const double xs,  const double ys) override;
    virtual void UpdateTexTextRef(const double xs,  const double ys) override;
    void UpdateTexTextGridSize();
    void UpdateTexAxesValues();
private:
    // Buffers
    GLuint _vaoID_grid;         //!< 1. Grid
    GLuint _vboID_grid;
    GLuint _iboID_grid_w;
    GLuint _iboID_grid_coarse_w;
    BufferSet buf_set_axes_;    //!< 2. Axes
    BufferSet buf_set_vref_;    //!< 3. Vref
    GLuint _vaoID_frame;        //!< 4. Frame
    GLuint _vboID_frame;
    GLuint _iboID_frame_onscr_w;
    GLuint _iboID_frame_onscr_q;
    GLuint _vaoID_graphs;       //!< 5. Graphs
    GLuint _vboID_graphs;
    GLuint _iboID_graphs_w;
    GLuint _iboID_graphs_m;
    BufferSet buf_set_cursor_;  //!< 6. Cursor
    GLuint _vaoID_sel;          //!< 7. Select rectangle
    GLuint _vboID_sel;
    GLuint _iboID_sel_q;
    GLuint _iboID_sel_w;
    BufferSet buf_set_circles_; //!< 8. Circles
    // Programs with camera uniforms
    ShaderProgram prog_sel_q_;
    ShaderProgram prog_onscr_q_;
    ShaderProgram prog_w_;
    ShaderProgram prog_onscr_w_;
    ShaderProgram prog_m_;
    ShaderProgram prog_c_;
    // Other uniforms
    GLint _fr_bg_unif_onscr_q; //!< In frame background color
    GLint _circle_r_unif_c;
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
    Vec4f TransformToVisrange(const double xs, const double ys,
        Vec4f* const o_in_clip_space = nullptr,
        Vec4f* const o_in_viewport_space = nullptr) const;
    Vec4f TransformToVisrange(const Vec4f& in_screen_space,
        Vec4f* const o_in_clip_space = nullptr,
        Vec4f* const o_in_viewport_space = nullptr) const;
    //Vec4d TransformToVisrangeHP(const double xs, const double ys,
    //    Vec4d* const o_in_clip_space = nullptr) const;
    //Vec4d TransformToVisrangeHP(const Vec4d& in_screen_space,
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
private:
    T ref_x_;
    // ===========================================================================
public:
    void UpdateSizeLimits();
    // ===========================================================================
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    tiny_gl_text_renderer::TextRenderer& GetTextRenderer() noexcept {
        return text_rend_;
    }
    void FinalizeTextRenderer();
private:
    tiny_gl_text_renderer::TextRenderer text_rend_;
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
    void SetXaxisTitle(const char* title) { x_axis_title_ = std::string(title); }
    void SetYaxisTitle(const char* title, const bool rotated = false) {
        y_axis_title_ = std::string(title); y_axis_title_rotated_ = rotated; }
    void EnableHgrid()    noexcept { enable_hgrid_   = true; }
    void EnableVgrid()    noexcept { enable_vgrid_   = true; }
    void EnableAxes()     noexcept { enable_axes_    = true; }
    void EnableVref()     noexcept { enable_vref_    = true; }
    void EnableFrame()    noexcept { enable_frame_   = true; }
    void EnableCursor()   noexcept { enable_cursor_  = true; }
    void EnableCircles()  noexcept { enable_circles_ = true; }
    void DisableHgrid()   noexcept { enable_hgrid_   = false; }
    void DisableVgrid()   noexcept { enable_vgrid_   = false; }
    void DisableAxes()    noexcept { enable_axes_    = false; }
    void DisableVref()    noexcept { enable_vref_    = false; }
    void DisableFrame()   noexcept { enable_frame_   = false; }
    void DisableCursor()  noexcept { enable_cursor_  = false; }
    void DisableCircles() noexcept { enable_circles_ = false; }
    // Color settings ------------------------------------------------------------
    void SetDarkColorScheme();
    void SetBrightColorScheme();
    void SetBackgroundColor(const color_t& color);
    void SetInFrameBackgroundColor(const color_t& color);
    void SetHGridFineColor  (const color_t& color) noexcept { _grid.SetHGridFineColor(color); }
    void SetVGridFineColor  (const color_t& color) noexcept { _grid.SetVGridFineColor(color); }
    void SetHGridCoarseColor(const color_t& color) noexcept { _grid.SetHGridCoarseColor(color); }
    void SetVGridCoarseColor(const color_t& color) noexcept { _grid.SetVGridCoarseColor(color); }
    void SetAxesColor       (const color_t& color) noexcept { axes_line_color_  = color; }
    void SetVrefColor       (const color_t& color) noexcept { vref_line_color_  = color; }
    void SetFrameColor      (const color_t& color) noexcept { frame_line_color_ = color; }
    void SetCursorColor     (const color_t& color) noexcept { cursor_color_     = color; }
    void SetTextColor       (const color_t& color) noexcept { gen_text_color_   = color; }
    // ---------------------------------------------------------------------------
    void SetHGridFineLineWidth  (const float width) noexcept { _grid.SetHGridFineLineWidth(width); }
    void SetVGridFineLineWidth  (const float width) noexcept { _grid.SetVGridFineLineWidth(width); }
    void SetHGridCoarseLineWidth(const float width) noexcept { _grid.SetHGridCoarseLineWidth(width); }
    void SetVGridCoarseLineWidth(const float width) noexcept { _grid.SetVGridCoarseLineWidth(width); }
    void SetAxesLineWidth       (const float width) noexcept { axes_line_width_   = width; }
    void SetVrefLineWidth       (const float width) noexcept { vref_line_width_   = width; }
    void SetFrameLineWidth      (const float width) noexcept { frame_line_width_  = width; }
    void SetCursorLineWidth     (const float width) noexcept { cursor_line_width_ = width; }
    void SetFontSize(const float size) noexcept { font_size_ = size; }
    void SetCircleRadius(const unsigned int r) noexcept { circle_r_ = r; }
    void SetMarginXleft(const unsigned int w_in_pix) {
        margin_xl_pix_ = w_in_pix; this->UpdateSizeLimits(); }
    void SetMarginXright(const unsigned int w_in_pix) {
        margin_xr_pix_ = w_in_pix; this->UpdateSizeLimits(); }
    void SetMarginYbottom(const unsigned int h_in_pix) {
        margin_yb_pix_ = h_in_pix; this->UpdateSizeLimits(); }
    void SetMarginYtop(const unsigned int h_in_pix) {
        margin_yt_pix_ = h_in_pix; this->UpdateSizeLimits(); }
    void SetAllMargins(const unsigned int xl_in_pix, const unsigned int xr_in_pix,
        const unsigned int yb_in_pix, const unsigned int yt_in_pix) {
        margin_xl_pix_ = xl_in_pix; margin_xr_pix_ = xr_in_pix;
        margin_yb_pix_ = yb_in_pix; margin_yt_pix_ = yt_in_pix;
        this->UpdateSizeLimits();
    }
private: // visual parameters
    std::string x_axis_title_ = "x axis";
    std::string y_axis_title_ = "y axis";
    bool y_axis_title_rotated_ = true;
    bool enable_hgrid_   = true;
    bool enable_vgrid_   = true;
    bool enable_axes_    = true;
    bool enable_vref_    = true;
    bool enable_frame_   = true;
    bool enable_cursor_  = true;
    bool enable_circles_ = true;
    color_t background_color_ = tiny_gl_text_renderer::colors::gray1;
    color_t in_frame_bg_color_ = tiny_gl_text_renderer::colors::gray05;
    color_t axes_line_color_  = tiny_gl_text_renderer::colors::gray75;
    color_t vref_line_color_  = tiny_gl_text_renderer::colors::olive;
    color_t frame_line_color_ = tiny_gl_text_renderer::colors::gray5;
    color_t cursor_color_     = tiny_gl_text_renderer::colors::white;
    color_t gen_text_color_   = tiny_gl_text_renderer::colors::white;
    float axes_line_width_   = 3.0f;
    float vref_line_width_   = 3.0f;
    float frame_line_width_  = 3.0f;
    float cursor_line_width_ = 1.0f;
    float font_size_ = 1.0f;
    unsigned int circle_r_ = 10u;
    unsigned int margin_xl_pix_ = 280u;
    unsigned int margin_xr_pix_ = 22u;
    unsigned int margin_yb_pix_ = 34u;
    unsigned int margin_yt_pix_ = 22u;
    static constexpr int _h_offset = 2;
    static constexpr int _v_offset = 2;
    // ===========================================================================
};

} // end of namespace tiny_graph_plot

#include "canvas_inline.h"
