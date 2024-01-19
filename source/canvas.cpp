#include "canvas.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "stb_image_write.h"

#include "canvas_shader_sources.h"
#include "graph.h"
#include "histogram1d.h"

namespace tiny_graph_plot
{

//#define SET_CONTEXT

using tiny_gl_text_renderer::marker_t;
using tiny_gl_text_renderer::wire_t;
//using tiny_gl_text_renderer::triangle_t;
using tiny_gl_text_renderer::quad_t;

#define MINWINWIDTH 200
#define MINWINHEIGHT 100
#define MINFRAMEWIDTH 50
#define MINFRAMEHEIGHT 50

template<typename T>
Canvas<T>::Canvas(GLFWwindow* window, const unsigned int w, const unsigned int h)
:   UserWindow(window, w, h),
    buf_set_axes_("axes"),
    buf_set_vref_("vref"),
    buf_set_cursor_("cursor"),
    buf_set_circles_("circles"),
    prog_sel_q_("prog_sel_quads"),
    prog_onscr_q_("prog_onscr_quads"),
    prog_w_("prog_wires"),
    prog_onscr_w_("prog_onscr_wires"),
    prog_m_("prog_markers"),
    prog_c_("prog_circles")
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    //this->UpdateSizeLimits();
    this->Init();
}

template<typename T>
Canvas<T>::~Canvas(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // VAOs, VBOs, IBOs ----------------------------------------------------------
    {
        glDeleteVertexArrays(1, &_vaoID_grid);
        glDeleteBuffers(1, &_vboID_grid);
        glDeleteBuffers(1, &_iboID_grid_w);
        glDeleteBuffers(1, &_iboID_grid_coarse_w);

        glDeleteVertexArrays(1, &_vaoID_frame);
        glDeleteBuffers(1, &_vboID_frame);
        glDeleteBuffers(1, &_iboID_frame_onscr_w);
        glDeleteBuffers(1, &_iboID_frame_onscr_q);

        glDeleteVertexArrays(1, &_vaoID_graphs);
        glDeleteBuffers(1, &_vboID_graphs);
        glDeleteBuffers(1, &_iboID_graphs_w);
        glDeleteBuffers(1, &_iboID_graphs_m);

        glDeleteVertexArrays(1, &_vaoID_sel);
        glDeleteBuffers(1, &_vboID_sel);
        glDeleteBuffers(1, &_iboID_sel_q);
        glDeleteBuffers(1, &_iboID_sel_w);
    }

    // objects of the _graphs container should NOT be
    // explicitly deleted here as the pointers are not owning
}

template<typename T>
void Canvas<T>::AddGraph(const Graph<T>& p_graph)
{
    _graphs.push_back(&p_graph);
}

template<typename T>
void Canvas<T>::AddHistogram(const Histogram1d<T, unsigned long>& p_histo)
{
    _histograms.push_back(&p_histo);
}

template<typename T>
void Canvas<T>::Show(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    glfwShowWindow(_window);

    this->SendFrameVerticesToGPU();

    glProgramUniform1f(prog_c_.GetProgId(), _circle_r_unif_c, (float)circle_r_);

    SizeInfo total_size; // Zeroed on construction
    _total_xy_range = _graphs.at(0)->GetXYrange();

    for (const auto* const gr : _graphs) {
        total_size += gr->GetSizeInfo();
        _total_xy_range.Include(gr->GetXYrange());
    }
    for (const auto* const histo : _histograms) {
        total_size += histo->GetSizeInfo();
        _total_xy_range.Include(histo->GetXYrange());
    }

    // Allocate vertex buffer space for all graphs. ------------------------------
    {
        const unsigned int n_vert = total_size._n_v;
        glBindVertexArray(_vaoID_graphs);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_graphs);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            NULL, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        //glBindVertexArray(0); // Not really needed.
    }

    // Allocate index buffers space for all graphs. ------------------------------
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_m);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_size._n_m * sizeof(marker_t),
            NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_size._n_w * sizeof(wire_t),
            NULL, GL_STATIC_DRAW);
    }

    SizeInfo cur_offset; // Zeroed on construction
    for (const auto* const gr : _graphs) {
        this->SendDrawableToGPU(gr, cur_offset);
        cur_offset += gr->GetSizeInfo();
    }
    for (const auto* const histo : _histograms) {
        this->SendDrawableToGPU(histo, cur_offset);
        cur_offset += histo->GetSizeInfo();
    }

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    const int ch_width = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    const int line_height = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_HEIGHT);
    const size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    int i_label = 0;
    int voffset = _v_offset;

    // Axis titles
    // X axis
    snprintf(&buf[0], BUFSIZE, "%s", x_axis_title_.c_str());
    const int line_len_x = (int)x_axis_title_.size();
    _x_axis_title_label_idx = text_rend_.AddLabel(buf,
        _window_w - margin_xr_pix_ - line_len_x * ch_width,
        _window_h - margin_yb_pix_ + line_height, gen_text_color_, font_size_);
    i_label++;
    // Y axis
    snprintf(&buf[0], BUFSIZE, "%s", y_axis_title_.c_str());
    const int line_len_y = (int)y_axis_title_.size();
    const int lbl_pos_x = (y_axis_title_rotated_ ?
        margin_xl_pix_ - line_height - line_height :
        margin_xl_pix_ - line_height - line_len_y * ch_width);
    const int lbl_pos_y = (y_axis_title_rotated_ ?
        margin_yt_pix_ + line_len_y * ch_width :
        margin_yt_pix_);
    _y_axis_title_label_idx = text_rend_.AddLabel(buf, lbl_pos_x, lbl_pos_y,
        gen_text_color_, font_size_, (y_axis_title_rotated_ ? 90.0f : 0.0f));
    i_label++;

    // Hint
    //const char test_line[] =
    //"\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz";
    //_hint_label_idx = text_rend_.AddLabel(
    //    "_y_axis_title_label_idx = text_rend_.AddLabel(buf, lbl_pos_x, lbl_pos_y,",
    //    //"Ctrl+left click to set the reference.\nPress 'F' to fit all in.",
    //    _h_offset, _window_h - 2 * line_height,
    //    tiny_gl_text_renderer::colors::silver, font_size_);
    //i_label++;

    // Grid parameters
    char buf2[128];
    snprintf(&buf2[0], 128, "(%g;%g) (%g;%g)",
        _grid.GetFineXstep(), _grid.GetFineYstep(),
        _grid.GetCoarseXstep(), _grid.GetCoarseYstep());
    const int grid_line_len = (int)strlen(buf2);
    _grid_params_label_idx = text_rend_.AddLabel(buf2,
        _window_w - margin_xr_pix_ - grid_line_len * ch_width, _v_offset,
        //_window_h - margin_yt_pix_ + line_height,
        gen_text_color_, font_size_);
    i_label++;

    // X axis values labels
    _x_axis_values_lables_start_idx = text_rend_.AddLabel(" ",
        margin_xl_pix_, _window_h - margin_yb_pix_,
        gen_text_color_, font_size_);
    i_label++;
    for (unsigned int i = 1; i < _n_x_axis_value_labels_max; i++) {
        text_rend_.AddLabel("",
            margin_xl_pix_, _window_h - margin_yb_pix_,
            gen_text_color_, font_size_);
        i_label++;
    }

    // Y axis values labels
    _y_axis_values_lables_start_idx = text_rend_.AddLabel("",
        margin_xl_pix_ - line_height, margin_yt_pix_,
        gen_text_color_, font_size_, 90.0f);
    i_label++;
    for (unsigned int i = 1; i < _n_y_axis_value_labels_max; i++) {
        text_rend_.AddLabel("",
            margin_xl_pix_ - line_height, margin_yt_pix_,
            gen_text_color_, font_size_, 90.0f);
        i_label++;
    }

    // Current values

    snprintf(&buf[0], BUFSIZE, "x =");
    _labels_start_idx = text_rend_.AddLabel(buf, _h_offset, voffset, gen_text_color_, font_size_);
    i_label++; voffset += line_height;
    snprintf(&buf[0], BUFSIZE, "y =");
    text_rend_.AddLabel(buf, _h_offset, voffset, gen_text_color_, font_size_);
    i_label++; voffset += line_height;
    int i_gr = 0;
    for (const auto* const gr : _graphs) {
        snprintf(&buf[0], BUFSIZE, "y%d=", i_gr);
        text_rend_.AddLabel(buf, _h_offset, voffset, gr->GetColor(), font_size_);
        i_label++; voffset += line_height;
        i_gr++;
    }

    // Reference values

    voffset += 2 * line_height;

    ref_x_ = static_cast<T>(_total_xy_range.lowx());
    snprintf(&buf[0], BUFSIZE, "rx =% 0.4f", ref_x_);
    text_rend_.AddLabel(buf, _h_offset, voffset, gen_text_color_, font_size_);
    i_label++; voffset += line_height;
    i_gr = 0;
    for (const auto* const gr : _graphs) {
        const T ref_y = gr->Evaluate(ref_x_);
        snprintf(&buf[0], BUFSIZE, "ry%d=% 0.4f", i_gr, ref_y);
        text_rend_.AddLabel(buf, _h_offset, voffset, gr->GetColor(), font_size_);
        i_label++; voffset += line_height;
        i_gr++;
    }

    // Difference

    voffset += 2 * line_height;

    snprintf(&buf[0], BUFSIZE, "dx =");
    text_rend_.AddLabel(buf, _h_offset, voffset, gen_text_color_, font_size_);
    i_label++; voffset += line_height;
    i_gr = 0;
    for (const auto* const gr : _graphs) {
        snprintf(&buf[0], BUFSIZE, "dy%d=", i_gr);
        text_rend_.AddLabel(buf, _h_offset, voffset, gr->GetColor(), font_size_);
        i_label++; voffset += line_height;
        i_gr++;
    }

    // Vertical difference

    voffset += 2 * line_height;

    for (int i = 0; i < (int)(_graphs.size()); i++) {
        for (int j = i+1; j < (int)(_graphs.size()); j++) {
            const Graph<T>* const gri = _graphs.at(i);
            const Graph<T>* const grj = _graphs.at(j);
            snprintf(&buf[0], BUFSIZE, "y%d", j);
            text_rend_.AddLabel(buf, _h_offset, voffset, grj->GetColor(), font_size_);
            i_label++;
            text_rend_.AddLabel("-", _h_offset + 3*ch_width, voffset, gen_text_color_, font_size_);
            i_label++;
            snprintf(&buf[0], BUFSIZE, "y%d", i);
            text_rend_.AddLabel(buf, _h_offset + 5*ch_width, voffset, gri->GetColor(), font_size_);
            i_label++;
            text_rend_.AddLabel("=", _h_offset + 8*ch_width, voffset, gen_text_color_, font_size_);
            i_label++;
            voffset += line_height;
            text_rend_.AddLabel("", _h_offset, voffset, gen_text_color_, font_size_);
            voffset += line_height;
        }
    }

    this->FinalizeTextRenderer();

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    this->ResetCamera();
}

template<typename T>
void Canvas<T>::Draw(void) /*const*/
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    this->SwitchToFrame();
    this->DrawGrid();
    this->DrawAxes();
    this->DrawVref();
    this->SwitchToFullWindow();
    this->DrawFrame();
    this->SwitchToFrame();

    SizeInfo cur_offset; // Zeroed on construction
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw graphs");
    for (const auto* const gr : _graphs) {
        if (gr->GetVisible()) {
            this->DrawDrawable(gr, cur_offset);
        }
        cur_offset += gr->GetSizeInfo();
    }
    glPopDebugGroup();

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw histograms");
    for (const auto* const histo : _histograms) {
        if (histo->GetVisible()) {
            this->DrawDrawable(histo, cur_offset);
        }
        cur_offset += histo->GetSizeInfo();
    }
    glPopDebugGroup();

    this->SwitchToFullWindow();
    this->UpdateTexAxesValues();
    //++++++++++++++++
    text_rend_.Draw();
    //++++++++++++++++
}

template<typename T>
void Canvas<T>::Init(void) 
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // VAOs, VBOs, IBOs ----------------------------------------------------------
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Init buffers");
    {
        {
        glGenVertexArrays(1, &_vaoID_grid);
        glGenBuffers(1, &_vboID_grid);
        glGenBuffers(1, &_iboID_grid_w);
        glGenBuffers(1, &_iboID_grid_coarse_w);

        const std::string name("grid");
        glObjectLabel(GL_VERTEX_ARRAY, _vaoID_grid, -1, (name + std::string("_vao")).c_str());
        glObjectLabel(GL_BUFFER, _vboID_grid, -1, (name + std::string("_vbo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_grid_w, -1, (name + std::string("_ibo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_grid_coarse_w, -1, (name + std::string("_coarse_ibo")).c_str());
        }

        buf_set_axes_.Generate();

        buf_set_vref_.Generate();

        {
        glGenVertexArrays(1, &_vaoID_frame);
        glGenBuffers(1, &_vboID_frame);
        glGenBuffers(1, &_iboID_frame_onscr_w);
        glGenBuffers(1, &_iboID_frame_onscr_q);

        const std::string name("frame");
        glObjectLabel(GL_VERTEX_ARRAY, _vaoID_frame, -1, (name + std::string("_vao")).c_str());
        glObjectLabel(GL_BUFFER, _vboID_frame, -1, (name + std::string("_vbo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_frame_onscr_w, -1, (name + std::string("_w_ibo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_frame_onscr_q, -1, (name + std::string("_q_ibo")).c_str());
        }

        {
        glGenVertexArrays(1, &_vaoID_graphs);
        glGenBuffers(1, &_vboID_graphs);
        glGenBuffers(1, &_iboID_graphs_w);
        glGenBuffers(1, &_iboID_graphs_m);

        const std::string name("graphs");
        glObjectLabel(GL_VERTEX_ARRAY, _vaoID_graphs, -1, (name + std::string("_vao")).c_str());
        glObjectLabel(GL_BUFFER, _vboID_graphs, -1, (name + std::string("_vbo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_graphs_w, -1, (name + std::string("_w_ibo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_graphs_m, -1, (name + std::string("_m_ibo")).c_str());
        }

        buf_set_cursor_.Generate();

        {
        glGenVertexArrays(1, &_vaoID_sel);
        glGenBuffers(1, &_vboID_sel);
        glGenBuffers(1, &_iboID_sel_q);
        glGenBuffers(1, &_iboID_sel_w);

        const std::string name("sel");
        glObjectLabel(GL_VERTEX_ARRAY, _vaoID_sel, -1, (name + std::string("_vao")).c_str());
        glObjectLabel(GL_BUFFER, _vboID_sel, -1, (name + std::string("_vbo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_sel_q, -1, (name + std::string("_q_ibo")).c_str());
        glObjectLabel(GL_BUFFER, _iboID_sel_w, -1, (name + std::string("_w_ibo")).c_str());
        }

        buf_set_circles_.Generate();
    }
    glPopDebugGroup();

    // Programs and uniforms -----------------------------------------------------
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Init programs");
    {
    // Quads / visible range space
    prog_sel_q_.Generate(canvas_sel_q_vp_source, nullptr, canvas_sel_q_fp_source);
    // Quads / screen space
    prog_onscr_q_.Generate(canvas_onscr_q_vp_source, nullptr, canvas_onscr_q_fp_source);
    _fr_bg_unif_onscr_q = glGetUniformLocation(prog_onscr_q_.GetProgId(), "drawcolor");
    // Wires / visible range space
    prog_w_.Generate(canvas_w_vp_source, nullptr, canvas_w_fp_source);
    // Wires / screen space
    prog_onscr_w_.Generate(canvas_onscr_w_vp_source, nullptr, canvas_onscr_w_fp_source);
    // Markers / visible range space
    prog_m_.Generate(canvas_m_vp_source, nullptr, canvas_m_fp_source);
    // Circles / visible range space
    prog_c_.Generate(canvas_c_vp_source, canvas_c_gp_source, canvas_c_fp_source);
    _circle_r_unif_c = glGetUniformLocation(prog_c_.GetProgId(), "circle_r");
    }
    glPopDebugGroup();

    // Send fixed data to GPU ----------------------------------------------------

    this->AllocateBuffersForFixedSizedData();
    this->SendFixedIndicesToGPU();

    // Initialize different OpenGL parameters

    glClearColor(background_color_[0], background_color_[1],
                 background_color_[2], background_color_[3]);
    glEnable(GL_PROGRAM_POINT_SIZE);
    //glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
    // This has to be done after the program has been compiled and the uniform
    // variable located
    glProgramUniform4fv(prog_onscr_q_.GetProgId(), _fr_bg_unif_onscr_q, 1,
        in_frame_bg_color_.GetData());
    // In principle, can be omitted
    //glProgramUniform1f(_progID_c, _circle_r_unif_c, (float)circle_r_);
}

template<typename T>
void Canvas<T>::Clear(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    glClear(GL_COLOR_BUFFER_BIT);
    this->FillInFrame();
}

template<typename T>
void Canvas<T>::Reshape(int p_width, int p_height)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    this->UpdateMatricesReshape();

    this->SendFrameVerticesToGPU();

    this->SendGridToGPU();

    //++++++++++++++++++++++++++++++++++++
    text_rend_.Reshape(p_width, p_height);
    //++++++++++++++++++++++++++++++++++++

    const int ch_width = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    const int line_height = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_HEIGHT);

    const int line_len_x = (int)x_axis_title_.size();
    text_rend_.UpdatePosition(
        _window_w - margin_xr_pix_ - line_len_x * ch_width,
        _window_h - margin_yb_pix_ + line_height,
        _x_axis_title_label_idx);
    //text_rend_.UpdatePosition(10, _window_h - 2 * line_height, _hint_label_idx);

    for (unsigned int i = 0; i < _n_x_axis_value_labels_max; i++) {
        text_rend_.UpdatePositionY(_window_h - margin_yb_pix_,
            _x_axis_values_lables_start_idx + (size_t)i);
    }
    for (unsigned int i = 0; i < _n_y_axis_value_labels_max; i++) {
        text_rend_.UpdatePositionX(margin_xl_pix_ - line_height,
            _y_axis_values_lables_start_idx + (size_t)i);
    }

    this->UpdateTexTextGridSize();
}

template<typename T>
void Canvas<T>::PrintCursorValues(const double xs, const double ys) const
{
    const Vec4f ps = Vec4f((float)xs, (float)ys, 0.0f, 1.0f);
    Vec4f pv;
    Vec4f pc;
    const Vec4f pr = this->TransformToVisrange(ps, &pc, &pv);
    ps.Print("\t|\t"); pv.Print("\t|\t"); pc.Print("\t|\t"); pr.Print("\n");
}

template<typename T>
//inline? //__forceinline?
void Canvas<T>::SwitchToFullWindow(void) const
{
    glViewport(0, 0, _window_w, _window_h);
}

template<typename T>
//inline? //__forceinline?
void Canvas<T>::SwitchToFrame(void) const
{
    glViewport(margin_xl_pix_, margin_yb_pix_,
        _window_w - (margin_xl_pix_ + margin_xr_pix_),
        _window_h - (margin_yb_pix_ + margin_yt_pix_));
}

// ===============================================================================

template<typename T>
void Canvas<T>::AllocateBuffersForFixedSizedData(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // Allocate vertex buffer space for the axes ---------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        buf_set_axes_.Allocate(n_vert);
    }

    // Allocate vertex buffer space for the vref line ----------------------------
    {
        constexpr unsigned int n_vert = 2u;
        buf_set_vref_.Allocate(n_vert);
    }

    // Allocate vertex buffer space for the frame --------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_frame);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            NULL, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        //glBindVertexArray(0); // Not really needed.
    }

    // Allocate vertex buffer space for the cursor -------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        buf_set_cursor_.Allocate(n_vert);
    }

    // Allocate vertex buffer space for the selection rectange -------------------
    {
        constexpr unsigned int n_vert = 4u;
        glBindVertexArray(_vaoID_sel);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_sel);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            NULL, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        //glBindVertexArray(0); // Not really needed.
    }
}

template<typename T>
void Canvas<T>::SendFixedIndicesToGPU(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // Axes
    {
        constexpr unsigned int n_wires_axes = 2u;
        const wire_t wires[n_wires_axes] = { {0, 1}, {2, 3} };
        buf_set_axes_.SendIndices(n_wires_axes, wires);
    }

    // Vref
    {
        constexpr unsigned int n_wires_vref = 1u;
        const wire_t wires[n_wires_vref] = { {0, 1} };
        buf_set_vref_.SendIndices(n_wires_vref, wires);
    }

    // Frame
    {
        constexpr unsigned int n_wires_frame = 4u;
        const wire_t wires[n_wires_frame] = { {0,1}, {1,2}, {2,3}, {3,0} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_frame * sizeof(wire_t),
            wires, GL_STATIC_DRAW);
        constexpr unsigned int n_quads_frame = 1u;
        const quad_t quads[n_quads_frame] = { { 0, 1, 2, 3 } };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_q);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_quads_frame * sizeof(quad_t),
            quads, GL_STATIC_DRAW);
    }

    // Cursor
    {
        constexpr unsigned int n_wires_cursor = 2u;
        const wire_t wires[n_wires_cursor] = { {0, 1}, {2, 3} };
        buf_set_cursor_.SendIndices(n_wires_cursor, wires);
    }

    // Selection rectangle
    {
        constexpr unsigned int n_wires_sel = 4u;
        const wire_t wires[n_wires_sel] = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_sel * sizeof(wire_t),
            wires, GL_STATIC_DRAW);
        constexpr unsigned int n_quads_sel = 1u;
        const quad_t quads[n_quads_sel] = { { 0, 1, 2, 3 } };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_q);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_quads_sel * sizeof(quad_t),
            quads, GL_STATIC_DRAW);
    }
}

// 1. Grid =======================================================================

template<typename T>
int Canvas<T>::SendGridToGPU(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const float vw = (float)(_window_w - (margin_xl_pix_ + margin_xr_pix_));
    const float vh = (float)(_window_h - (margin_yb_pix_ + margin_yt_pix_));
    if (_grid.CalculateStep(_visible_range, vw, vh) == 1) {
        // The range is degenerate
        //TODO decide what to do
        return 1;
    }
    if (_grid.BuildGrid(_visible_range, _total_xy_range) == 1) {
        // No changes have to be made
        return 1;
    }

    // Send vertices and colors. -------------------------------------------------
    {
        unsigned int n_vertices;
        const vertex_colored_t* const vertices = _grid.GetVerticesData(n_vertices);

        glBindVertexArray(_vaoID_grid);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_grid);
        glBufferData(GL_ARRAY_BUFFER, n_vertices * sizeof(vertex_colored_t),
            vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        //glBindVertexArray(0); // Not really needed.
    }

    // Send wires indices. -------------------------------------------------------
    // Fine grid - dotted lines
    {
        unsigned int n_wires_fine_x; unsigned int n_wires_fine_y;
        const wire_t* const wires_fine =
            _grid.GetWiresFineData(n_wires_fine_x, n_wires_fine_y);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            (n_wires_fine_x + n_wires_fine_y) * sizeof(wire_t),
            wires_fine, GL_STATIC_DRAW);
    }
    // Coarse grid - solid lines
    {
        unsigned int n_wires_coarse_x; unsigned int n_wires_coarse_y;
        const wire_t* const wires_coarse =
            _grid.GetWiresCoarseData(n_wires_coarse_x, n_wires_coarse_y);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_coarse_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            (n_wires_coarse_x + n_wires_coarse_y) * sizeof(wire_t),
            wires_coarse, GL_STATIC_DRAW);
    }
    return 0;
}

template<typename T>
void Canvas<T>::DrawGrid(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!enable_hgrid_ && !enable_vgrid_) {
        return;
    }

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw grid");

    // Draw wires. Wires indices have already been sent. -------------------------
    {
        unsigned int n_wires_fine_x; unsigned int n_wires_fine_y;
        const wire_t* const wires_fine =
            _grid.GetWiresFineData(n_wires_fine_x, n_wires_fine_y);
        (void)wires_fine; // unused returned value.
        unsigned int n_wires_coarse_x; unsigned int n_wires_coarse_y;
        const wire_t* const wires_coarse =
            _grid.GetWiresCoarseData(n_wires_coarse_x, n_wires_coarse_y);
        (void)wires_coarse; // unused returned value.

        prog_w_.Use();
        glBindVertexArray(_vaoID_grid);
        if (enable_vgrid_) {
            // Fine grid
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x0101);
            glLineWidth(_grid.GetVGridFineLineWidth());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_w);
            glDrawElements(GL_LINES, 2 * n_wires_fine_x, GL_UNSIGNED_INT, NULL);
            glDisable(GL_LINE_STIPPLE);
            // Coarse grid
            glLineWidth(_grid.GetVGridCoarseLineWidth());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_coarse_w);
            glDrawElements(GL_LINES, 2 * n_wires_coarse_x, GL_UNSIGNED_INT, NULL);
        }
        if (enable_hgrid_) {
            // Fine grid
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x0101);
            glLineWidth(_grid.GetHGridFineLineWidth());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_w);
            glDrawElements(GL_LINES, 2 * n_wires_fine_y, GL_UNSIGNED_INT,
                (GLvoid*)((size_t)(n_wires_fine_x) * sizeof(wire_t)));
            glDisable(GL_LINE_STIPPLE);
            // Coarse grid
            glLineWidth(_grid.GetHGridCoarseLineWidth());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_coarse_w);
            glDrawElements(GL_LINES, 2 * n_wires_coarse_y, GL_UNSIGNED_INT,
                (GLvoid*)((size_t)(n_wires_coarse_x) * sizeof(wire_t)));
        }
        //glBindVertexArray(0); // Not really needed.
    }

    glPopDebugGroup();
}

// 2. Axes =======================================================================

template<typename T>
void Canvas<T>::DrawAxes(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!enable_axes_) {
        return;
    }

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw axes");

    // Send vertices and colors. -------------------------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        vertex_colored_t vertices[n_vert];
        vertices[0].coords_ = point_t(
            static_cast<float>(_total_xy_range.lowx()), 0.0f, 0.0f, 1.0f);
        vertices[1].coords_ = point_t(
            static_cast<float>(_total_xy_range.highx()), 0.0f, 0.0f, 1.0f);
        vertices[2].coords_ = point_t(
            0.0f, static_cast<float>(_total_xy_range.lowy()), 0.0f, 1.0f);
        vertices[3].coords_ = point_t(
            0.0f, static_cast<float>(_total_xy_range.highy()), 0.0f, 1.0f);
        vertices[0].color_ = axes_line_color_;
        vertices[1].color_ = axes_line_color_;
        vertices[2].color_ = axes_line_color_;
        vertices[3].color_ = axes_line_color_;

        buf_set_axes_.SendVertices(n_vert, vertices);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        constexpr unsigned int n_wires = 2u;
        prog_w_.Use();
        glLineWidth(axes_line_width_);
        buf_set_axes_.DrawWires(n_wires);
    }

    glPopDebugGroup();
}

// 3. Vref =======================================================================

template<typename T>
void Canvas<T>::DrawVref(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!enable_vref_) {
        return;
    }

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw Vref");

    // Send vertices and colors. -------------------------------------------------
    {
        constexpr unsigned int n_vert = 2u;
        vertex_colored_t vertices[n_vert];
        vertices[0].coords_ = point_t(static_cast<float>(ref_x_),
            static_cast<float>(_total_xy_range.lowy()), 0.0f, 1.0f);
        vertices[1].coords_ = point_t(static_cast<float>(ref_x_),
            static_cast<float>(_total_xy_range.highy()), 0.0f, 1.0f);
        vertices[0].color_ = vref_line_color_;
        vertices[1].color_ = vref_line_color_;

        buf_set_vref_.SendVertices(n_vert, vertices);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        constexpr unsigned int n_wires = 1u;
        prog_w_.Use();
        glLineWidth(vref_line_width_);
        buf_set_vref_.DrawWires(n_wires);
    }

    glPopDebugGroup();
}

// 4. Frame ======================================================================

template<typename T>
void Canvas<T>::SendFrameVerticesToGPU(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // Send vertices and colors. -------------------------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        vertex_colored_t vertices[n_vert];
        const float blx_ = (float)margin_xl_pix_;
        const float bly_ = (float)margin_yb_pix_;
        const float trx_ = (float)_window_w - (float)margin_xr_pix_;
        const float try_ = (float)_window_h - (float)margin_yt_pix_;
        vertices[0].coords_ = point_t(blx_, bly_, 0.0f, 1.0f);
        vertices[1].coords_ = point_t(trx_, bly_, 0.0f, 1.0f);
        vertices[2].coords_ = point_t(trx_, try_, 0.0f, 1.0f);
        vertices[3].coords_ = point_t(blx_, try_, 0.0f, 1.0f);
        vertices[0].color_ = frame_line_color_;
        vertices[1].color_ = frame_line_color_;
        vertices[2].color_ = frame_line_color_;
        vertices[3].color_ = frame_line_color_;

        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_frame);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            n_vert * sizeof(vertex_colored_t), vertices);
        //glBindVertexArray(0); // Not really needed.
    }
}

template<typename T>
void Canvas<T>::FillInFrame(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // In principle, filling of the in frame space need not necessarily be
    // controlled by this flag, it could be another flag.
    if (!enable_frame_) {
        return;
    }

    this->SwitchToFullWindow(); //TODO move outside?

    // Draw quads. Quads indices have already been sent. -------------------------
    {
        constexpr unsigned int n_quads = 1u;
        prog_onscr_q_.Use();
        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_q);
        glDrawElements(GL_QUADS, 4 * n_quads, GL_UNSIGNED_INT, NULL);
        //glBindVertexArray(0); // Not really needed.
    }
}

template<typename T>
void Canvas<T>::DrawFrame(void) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!enable_frame_) {
        return;
    }

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw frame");

    // Draw wires. Wires indices have already been sent. -------------------------
    {
        constexpr unsigned int n_wires = 4u;
        prog_onscr_w_.Use();
        glLineWidth(2.0f);
        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_w);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        //glBindVertexArray(0); // Not really needed.
    }

    glPopDebugGroup();
}

// 5. Graphs =====================================================================

template<typename T>
void Canvas<T>::SendDrawableToGPU(const Drawable<T>* const p_graph, const SizeInfo& p_offset) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const SizeInfo& cur_size = p_graph->GetSizeInfo();

    // Send vertices and colors. -------------------------------------------------
    {
        vertex_colored_t* vertices = new vertex_colored_t[cur_size._n_v];

        for (unsigned int i = 0; i < cur_size._n_v; i++) {
            const Vec2<T>& cur_pt = p_graph->GetPoint(i);
            vertices[i].coords_[0] = static_cast<float>(cur_pt.x());
            vertices[i].coords_[1] = static_cast<float>(cur_pt.y());
            vertices[i].coords_[2] = 0.0f;
            vertices[i].coords_[3] = 1.0f;
            vertices[i].color_ = p_graph->GetColor();
        }

        glBindVertexArray(_vaoID_graphs);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_graphs);
        glBufferSubData(GL_ARRAY_BUFFER,
            p_offset._n_v * sizeof(vertex_colored_t),
            cur_size._n_v * sizeof(vertex_colored_t), vertices);
        //glBindVertexArray(0); // Not really needed.

        if (vertices != nullptr) delete[] vertices;
    }
    // Send markers and wires indices. -------------------------------------------
    {
        marker_t* markers = new marker_t[cur_size._n_m];
        wire_t* wires = new wire_t[cur_size._n_w];

        for (unsigned int i = 0; i < cur_size._n_m; i++) {
            markers[i].v0 = i;
        }
        for (unsigned int i = 0; i < cur_size._n_w; i++) {
            wires[i].v0 = i;
            wires[i].v1 = i + 1;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_m);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, p_offset._n_m * sizeof(marker_t),
            cur_size._n_m * sizeof(marker_t), markers);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_w);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, p_offset._n_w * sizeof(wire_t),
            cur_size._n_w * sizeof(wire_t), wires);

        if (markers != nullptr) delete[] markers;
        if (wires != nullptr) delete[] wires;
    }
}

template<typename T>
void Canvas<T>::DrawDrawable(const Drawable<T>* const p_graph, const SizeInfo& p_offset) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw drawable");

    const SizeInfo& cur_size = p_graph->GetSizeInfo();

    // Draw markers. Markers indices have already been sent. ---------------------
    {
        prog_m_.Use();
        glPointSize(p_graph->GetMarkerSize());
        glBindVertexArray(_vaoID_graphs);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_m);
        glDrawElementsBaseVertex(GL_POINTS, 1 * cur_size._n_m, GL_UNSIGNED_INT,
            (GLvoid*)(p_offset._n_m * sizeof(marker_t)), (GLint)p_offset._n_v);
        //glBindVertexArray(0); // Not really needed.
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        prog_w_.Use();
        glLineWidth(p_graph->GetLineWidth());
        glBindVertexArray(_vaoID_graphs);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_w);
        glDrawElementsBaseVertex(GL_LINES, 2 * cur_size._n_w, GL_UNSIGNED_INT,
            (GLvoid*)(p_offset._n_w * sizeof(wire_t)), (GLint)p_offset._n_v);
        //glBindVertexArray(0); // Not really needed.
    }

    glPopDebugGroup();
}

// 6. Cursor =====================================================================

template<typename T>
void Canvas<T>::DrawCursor(const double xs, const double ys) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!enable_cursor_) {
        return;
    }

    this->SwitchToFullWindow(); //TODO move outside?

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw cursor");

    // Send vertices and colors. -------------------------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        vertex_colored_t vertices[n_vert];
        const float blx_ = (float)margin_xl_pix_;
        const float bly_ = (float)margin_yb_pix_;
        const float trx_ = (float)_window_w - (float)margin_xr_pix_;
        const float try_ = (float)_window_h - (float)margin_yt_pix_;
        vertices[0].coords_ = point_t((float)xs, bly_, 0.0f, 1.0f);
        vertices[1].coords_ = point_t((float)xs, try_, 0.0f, 1.0f);
        vertices[2].coords_ = point_t(blx_, (float)ys, 0.0f, 1.0f);
        vertices[3].coords_ = point_t(trx_, (float)ys, 0.0f, 1.0f);
        vertices[0].color_ = cursor_color_;
        vertices[1].color_ = cursor_color_;
        vertices[2].color_ = cursor_color_;
        vertices[3].color_ = cursor_color_;

        buf_set_cursor_.SendVertices(n_vert, vertices);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        constexpr unsigned int n_wires = 2u;
        prog_onscr_w_.Use();
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x00FF);
        glLineWidth(cursor_line_width_);
        buf_set_cursor_.DrawWires(n_wires);
        glDisable(GL_LINE_STIPPLE);
    }

    glPopDebugGroup();
}

// 7. Selection rectangle ========================================================

template<typename T>
void Canvas<T>::DrawSelRectangle(const double xs0, const double ys0,
    const double xs1, const double ys1) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    this->SwitchToFrame(); //TODO move outside?

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw selection rectangle");

    // Send vertices and colors. -------------------------------------------------
    {
        constexpr unsigned int n_vert = 4u;
        vertex_colored_t vertices[n_vert];
        const Vec4f p0r = this->TransformToVisrange(xs0, ys0);
        const Vec4f p1r = this->TransformToVisrange(xs1, ys1);
        vertices[0].coords_ = point_t(p0r.x(), p0r.y(), 0.0f, 1.0f);
        vertices[1].coords_ = point_t(p1r.x(), p0r.y(), 0.0f, 1.0f);
        vertices[2].coords_ = point_t(p1r.x(), p1r.y(), 0.0f, 1.0f);
        vertices[3].coords_ = point_t(p0r.x(), p1r.y(), 0.0f, 1.0f);
        vertices[0].color_ = tiny_gl_text_renderer::colors::sel_color;
        vertices[1].color_ = tiny_gl_text_renderer::colors::sel_color;
        vertices[2].color_ = tiny_gl_text_renderer::colors::sel_color;
        vertices[3].color_ = tiny_gl_text_renderer::colors::sel_color;

        glBindVertexArray(_vaoID_sel);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_sel);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            4 * sizeof(vertex_colored_t), vertices);
        //glBindVertexArray(0); // Not really needed.
    }

    // Draw. ---------------------------------------------------------------------
    {
        // Draw wires. Wires indices have already been sent. ---------------------
        constexpr unsigned int n_wires = 4u;
        prog_w_.Use();
        glLineWidth(2.0f);
        glBindVertexArray(_vaoID_sel);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_w);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        //glBindVertexArray(0); // Not really needed.

        // Draw quads. Quads indices have already been sent. ---------------------
        constexpr unsigned int n_quads = 1u;
        prog_sel_q_.Use();
        glBindVertexArray(_vaoID_sel);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_q);
        glDrawElements(GL_QUADS, 4 * n_quads, GL_UNSIGNED_INT, NULL);
        //glBindVertexArray(0); // Not really needed.
    }

    glPopDebugGroup();
}

// 8. Circles ====================================================================

template<typename T>
void Canvas<T>::DrawCircles(const double xs, const double ys) const
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!enable_circles_) {
        return;
    }

    this->SwitchToFrame(); //TODO move outside?

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw circles");

    const unsigned int n_vert = (unsigned int)_graphs.size();
    //const unsigned int n_markers = (unsigned int)_graphs.size();
    unsigned int n_markers = 0;

    // Send vertices and colors. -------------------------------------------------
    {
        vertex_colored_t* vertices = new vertex_colored_t[n_vert];
        marker_t* markers = new marker_t[n_vert];

        const Vec4f pr = this->TransformToVisrange(xs, ys);
        int i_gr = 0;
        for (const auto* const gr : _graphs) {
            if (!gr->GetVisible()) continue;
            const T y = gr->Evaluate(static_cast<T>(pr.x()));
            vertices[i_gr].coords_ = point_t(
                pr.x(), static_cast<float>(y), 0.0f, 1.0f);
            vertices[i_gr].color_ = gr->GetColor();
            markers[i_gr].v0 = i_gr;
            i_gr++;
            n_markers++;
        }

        buf_set_circles_.Allocate(n_vert, vertices);

        buf_set_circles_.SendIndices(n_markers, markers);

        if (vertices != nullptr) delete[] vertices;
        if (markers != nullptr) delete[] markers;
    }
    // Draw. ---------------------------------------------------------------------
    {
        prog_c_.Use();
        buf_set_circles_.DrawMarkers(n_markers);
    }

    glPopDebugGroup();
}

// 9. Text =======================================================================

template<typename T>
void Canvas<T>::UpdateTexTextCur(const double xs, const double ys)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const Vec4f pr = this->TransformToVisrange(xs, ys);
    const T x = static_cast<T>(pr.x());
    const T y = static_cast<T>(pr.y());
    const T& x0 = ref_x_;

    const size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    int i_lab1 = (int)_labels_start_idx;
    int i_lab2 = (int)_labels_start_idx + 1 + 2 * (int)(_graphs.size() + 1);

    snprintf(&buf[0], BUFSIZE, "x =% 0.4f", x);
    text_rend_.UpdateLabel(buf, i_lab1);
    i_lab1++;
    snprintf(&buf[0], BUFSIZE, "y =% 0.4f", y);
    text_rend_.UpdateLabel(buf, i_lab1);
    i_lab1++;
    snprintf(&buf[0], BUFSIZE, "dx =% 0.4f", x - x0);
    text_rend_.UpdateLabel(buf, i_lab2);
    i_lab2++;

    int i_gr = 0;
    for (const auto* const gr : _graphs) {
        const T y0 = gr->Evaluate(x0);
        const T yi = gr->Evaluate(x);
        snprintf(&buf[0], BUFSIZE, "y%d=% 0.4f", i_gr, yi);
        text_rend_.UpdateLabel(buf, i_lab1);
        snprintf(&buf[0], BUFSIZE, "dy%d=% 0.4f", i_gr, yi - y0);
        text_rend_.UpdateLabel(buf, i_lab2);
        i_lab1++; i_lab2++;
        i_gr++;
    }

    // Vertical difference

    int i_lab3 = (int)_labels_start_idx + 1 + 3 * (int)(_graphs.size() + 1) + 4;

    for (int i = 0; i < (int)(_graphs.size()); i++) {
        for (int j = i + 1; j < (int)(_graphs.size()); j++) {
            const Graph<T>* const gri = _graphs.at(i);
            const Graph<T>* const grj = _graphs.at(j);
            const T yi = gri->Evaluate(x);
            const T yj = grj->Evaluate(x);
            const T dy = yj - yi;
            snprintf(&buf[0], BUFSIZE, "% 0.4f", dy);
            text_rend_.UpdateLabel(buf, i_lab3);
            i_lab3 += 5;
        }
    }
}

template<typename T>
void Canvas<T>::UpdateTexTextRef(const double xs, const double ys)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const Vec4f pr = this->TransformToVisrange(xs, ys);

    if (!_total_xy_range.IncludesX(pr.x())) return;

    ref_x_ = pr.x();

    const size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    int i_lab = (int)_labels_start_idx + 1 + (int)(_graphs.size() + 1);
    snprintf(&buf[0], BUFSIZE, "rx =% 0.4f", ref_x_);
    text_rend_.UpdateLabel(buf, i_lab);
    i_lab++;

    int i_gr = 0;
    for (const auto* const gr : _graphs) {
        const T y0 = gr->Evaluate(ref_x_);
        snprintf(&buf[0], BUFSIZE, "ry%d=% 0.4f", i_gr, y0);
        text_rend_.UpdateLabel(buf, i_lab);
        i_lab++;
        i_gr++;
    }
}

template<typename T>
void Canvas<T>::UpdateTexTextGridSize(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const int ch_width = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    //const int line_height = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_HEIGHT);

    char buf2[128];
    snprintf(&buf2[0], 128, "(%g;%g) (%g;%g)",
        _grid.GetFineXstep(), _grid.GetFineYstep(),
        _grid.GetCoarseXstep(), _grid.GetCoarseYstep());
    const int grid_line_len = (int)strlen(buf2);
    text_rend_.UpdateLabel(buf2, _grid_params_label_idx);
    text_rend_.UpdatePosition(_window_w - margin_xr_pix_ -
        grid_line_len * ch_width, _v_offset, _grid_params_label_idx);
}

template<typename T>
void Canvas<T>::UpdateTexAxesValues(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    unsigned int nv_;
    const vertex_colored_t* const vertices = _grid.GetVerticesData(nv_);
    unsigned int nx_;
    unsigned int ny_;
    const wire_t* const wires = _grid.GetWiresCoarseData(nx_, ny_);

    const unsigned int nx = std::min(_n_x_axis_value_labels_max, nx_);
    const unsigned int ny = std::min(_n_y_axis_value_labels_max, ny_);

    const int ch_width = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    //const int line_height = (int)(font_size_ * (float)tiny_gl_text_renderer::CHAR_HEIGHT);
    constexpr size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    // X axis --------------------------------------------------------------------

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Update X axis labels");

    for (unsigned int i = 0; i < nx; i++) {
        const unsigned int idx0 = wires[i].v0;
        //const unsigned int idx1 = wires[i].v1;

        snprintf(buf, BUFSIZE, "%g", vertices[idx0].coords_.x());
        const int offset = -(ch_width * (int)strlen(buf)) / 2;
        text_rend_.UpdateLabel(buf, _x_axis_values_lables_start_idx + (size_t)i);

        const Vec4f vr(vertices[idx0].coords_.x(), 0.0f, 0.0f, 1.0f);
        const Vec4f vc = _visrange_to_clip * vr;
        //const Vec4f vs = _clip_to_screen * vc;
        const Vec4f vv = _clip_to_viewport * vc;
        const Vec4f vs = _viewport_to_screen * vv;

        text_rend_.UpdatePositionX((int)(vs.x()) + offset,
            _x_axis_values_lables_start_idx + (size_t)i);
    }
    for (unsigned int i = nx; i < _n_x_axis_value_labels_max; i++) {
        text_rend_.UpdateLabel(" ", _x_axis_values_lables_start_idx + (size_t)i);
    }

    glPopDebugGroup();

    // Y axis --------------------------------------------------------------------

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Update Y axis labels");

    for (unsigned int i = 0; i < ny; i++) {
        const unsigned int idx0 = wires[nx_+i].v0;
        //const unsigned int idx1 = wires[nx_+i].v1;

        snprintf(buf, BUFSIZE, "%g", vertices[idx0].coords_.y());
        const int offset = (ch_width * (int)strlen(buf)) / 2;
        text_rend_.UpdateLabel(buf, _y_axis_values_lables_start_idx + (size_t)i);

        const Vec4f vr(0.0f, vertices[idx0].coords_.y(), 0.0f, 1.0f);
        const Vec4f vc = _visrange_to_clip * vr;
        //const Vec4f vs = _clip_to_screen * vc;
        const Vec4f vv = _clip_to_viewport * vc;
        const Vec4f vs = _viewport_to_screen * vv;

        text_rend_.UpdatePositionY(_window_h - (int)(vs.y()) + offset,
            _y_axis_values_lables_start_idx + (size_t)i);
    }
    for (unsigned int i = ny; i < _n_y_axis_value_labels_max; i++) {
        text_rend_.UpdateLabel(" ", _y_axis_values_lables_start_idx + (size_t)i);
    }

    glPopDebugGroup();
}

// ===============================================================================

template<typename T>
void Canvas<T>::CenterView(const double xs, const double ys)
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif

    const Vec4f pr = this->TransformToVisrange(xs, ys);
    const float cur_center_x = 0.5f * _visible_range.twoxm();
    const float cur_center_y = 0.5f * _visible_range.twoym();
    _visible_range.MoveX((float)pr.x() - cur_center_x);
    _visible_range.MoveY((float)pr.y() - cur_center_y);

    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::Pan(const double xs, const double ys)
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif

    const Vec4f p2r = this->TransformToVisrange(_xs_prev, _ys_prev);
    const Vec4f p1r = this->TransformToVisrange(xs, ys);
    const float shift_x = ((float)p1r.x() - (float)p2r.x());
    const float shift_y = ((float)p1r.y() - (float)p2r.y());

    _visible_range.MoveX(-shift_x);
    _visible_range.MoveY(-shift_y);

    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::Zoom(const double xs, const double ys)
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif

    const Vec4f deltas((float)(-xs + _xs_start), (float)(-ys + _ys_start), 0.0f, 0.0f);
    Vec4f deltac;
    this->TransformToVisrange(deltas, &deltac);
    const float kx = 1.0f + 0.5f * deltac.x();
    const float ky = 1.0f + 0.5f * deltac.y();

    if (kx < 0.0f || ky < 0.0f) return;

    Vec4f p0c;
    Vec4f p1c;
    const Vec4f p0r = this->TransformToVisrange(_xs_start, _ys_start, &p0c);
    //const Vec4f p1r = this->TransformToVisrange(xs, ys, &p1c);
    //const float kx = 1.0f + 0.5f * ((float)p0c.x() - (float)p1c.x());
    //const float ky = 1.0f + 0.5f * ((float)p0c.y() - (float)p1c.y());
    const float center_x = (float)p0r.x();
    const float center_y = (float)p0r.y();
    const float dxr_down_start = center_x - _visible_range_start.lowx();
    //const float dxr_up_start   = _visible_range_start.highx() - center_x;
    const float dyr_down_start = center_y - _visible_range_start.lowy();
    //const float dyr_up_start   = _visible_range_start.highy() - center_y;
    if (kx * _visible_range_start.dx() > (float)1.0e-7 &&
        kx * _visible_range_start.dx() < (float)1.0e+7) { // Limit zooming
        _visible_range.SetXrange2(center_x - dxr_down_start * kx, kx * _visible_range_start.dx());
    }
    if (ky * _visible_range_start.dy() > (float)1.0e-7 &&
        ky * _visible_range_start.dy() < (float)1.0e+7) { // Limit zooming
        _visible_range.SetYrange2(center_y - dyr_down_start * ky, ky * _visible_range_start.dy());
    }

    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::ZoomF(const double xs, const double ys)
{
    (void)xs;
    const float vw = (float)(_window_w - (margin_xl_pix_ + margin_xr_pix_));
    const float vh = (float)(_window_h - (margin_yb_pix_ + margin_yt_pix_));
    const float asp_rat = vw / vh;
    //const float asp_rat_inv = vh / vw;
    this->Zoom(_xs_start + (ys - _ys_start) * asp_rat, ys);
}

template<typename T>
void Canvas<T>::ZoomX(const double xs, const double ys)
{
    (void)ys;
    this->Zoom(xs, _ys_start);
}

template<typename T>
void Canvas<T>::ZoomY(const double xs, const double ys)
{
    (void)xs;
    this->Zoom(_xs_start, ys);
}

template<typename T>
void Canvas<T>::ZoomTo(const double xs0, const double ys0, const double xs1, const double ys1)
{
    ////#ifdef SET_CONTEXT
    ////    glfwMakeContextCurrent(_window);
    ////#endif

    const Vec4f p0r = this->TransformToVisrange(xs0, ys0);
    const Vec4f p1r = this->TransformToVisrange(xs1, ys1);

    _visible_range.Set1(
        static_cast<float>(std::fmin(p0r.x(), p1r.x())),
        static_cast<float>(std::fmax(p0r.x(), p1r.x())),
        static_cast<float>(std::fmin(p0r.y(), p1r.y())),
        static_cast<float>(std::fmax(p0r.y(), p1r.y())));

    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::ResetCamera(void)
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    _visible_range = _total_xy_range;
    this->UpdateMatricesReshape();
    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::SetPrevViewport(void)
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    _visible_range = _visible_range_start;
    this->UpdateMatricesReshape();
    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::FixedAspRatCamera(void)
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif

    // Everything in double here

    // Just to shorten the notation
    const double vw = (double)(_window_w - (margin_xl_pix_ + margin_xr_pix_));
    const double vh = (double)(_window_h - (margin_yb_pix_ + margin_yt_pix_));
    const double asp_rat_inv = vh / vw;
    const double asp_rat     = vw / vh;

    if (_visible_range.dx() > _visible_range.dy()) {
        const double midy           = 0.5 * _visible_range.twoym();
        const double new_half_range = 0.5 * _visible_range.dx();
        _visible_range.SetYrange1(
            static_cast<float>(midy - new_half_range * asp_rat_inv),
            static_cast<float>(midy + new_half_range * asp_rat_inv));
    } else {
        const double midx           = 0.5 * _visible_range.twoxm();
        const double new_half_range = 0.5 * _visible_range.dy();
        _visible_range.SetXrange1(
            static_cast<float>(midx - new_half_range * asp_rat),
            static_cast<float>(midx + new_half_range * asp_rat));
    }

    this->UpdateMatricesReshape();
    this->UpdateMatricesPanZoom();
    const int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::ExportSnapshot(void)
{
#ifdef _WIN32
    size_t len[2];
    char drive_dir[2][_MAX_PATH];
    getenv_s(&len[0], drive_dir[0], _MAX_PATH, "HOMEDRIVE");
    getenv_s(&len[1], drive_dir[1], _MAX_PATH, "HOMEPATH");
    if (len[0] == 0 || len[1] == 0) return;
    const std::string dir = std::string(drive_dir[0]) + std::string(drive_dir[1]);
    const char filename[] = "snapshot.png";
    this->ExportPNG(dir.c_str(), filename);
#else
    //TODO implement
#endif
}

template<typename T>
bool Canvas<T>::PointerInFrame(const double xs, const double ys) const
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    const int xs_i = (int)xs;
    const int ys_i = (int)ys;
    return (
        (xs_i >= (int)margin_xl_pix_) &&
        (xs_i <= ((int)_window_w - (int)margin_xr_pix_)) &&
        (ys_i >= (int)margin_yb_pix_) &&
        (ys_i <= ((int)_window_h - (int)margin_yt_pix_)));
}

template<typename T>
void Canvas<T>::ClampToFrame(const double xs, const double ys, double& o_xs, double& o_ys) const
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    const double left_boundary = (double)margin_xl_pix_;
    const double right_boundary = (double)_window_w - (double)margin_xr_pix_;
    const double bottom_boundary = (double)margin_yb_pix_;
    const double top_boundary = (double)_window_h - (double)margin_yt_pix_;
    o_xs = std::fmin(std::fmax(left_boundary, xs), right_boundary);
    o_ys = std::fmin(std::fmax(bottom_boundary, ys), top_boundary);
}

template<typename T>
void Canvas<T>::SaveStartState(void)
{
    _visible_range_start = _visible_range;
}

template<typename T>
void Canvas<T>::ToggleGraphVisibility(const int iGraph) const
{
    if (iGraph >= static_cast<int>(_graphs.size())) return;
    const bool curVisibility = _graphs.at(iGraph)->GetVisible();
    _graphs.at(iGraph)->SetVisible(!curVisibility);
}

template<typename T>
void Canvas<T>::ExportPNG(const char* const dir, const char* const filename) const
{
    unsigned char* const data = new unsigned char[_window_h * _window_w * 3];
    if (!data) return;
    glReadPixels(0, 0, _window_w, _window_h, GL_RGB, GL_UNSIGNED_BYTE, data);
    const std::string path = std::string(dir) + std::string("\\") + std::string(filename);
    stbi_flip_vertically_on_write(1);
    stbi_write_png(path.c_str(), _window_w, _window_h, 3, data, _window_w * 3 * sizeof(unsigned char));
    delete[] data;
}

template<typename T>
void Canvas<T>::UpdateMatricesReshape(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const float ax = (float)margin_xl_pix_;
    const float ay = (float)margin_yb_pix_;

    _screen_to_viewport.Set(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -ax,  -ay,  0.0f, 1.0f);
    _viewport_to_screen.Set(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        ax,   ay,   0.0f, 1.0f);

    // Double precision matrix
    //_screen_to_viewport_hp.Set(
    //    1.0, 0.0, 0.0, 0.0,
    //    0.0, 1.0, 0.0, 0.0,
    //    0.0, 0.0, 1.0, 0.0,
    //    -(double)margin_xl_pix_, -(double)margin_yb_pix_, 0.0, 1.0);

    const double vw = (double)(_window_w - (margin_xl_pix_ + margin_xr_pix_));
    const double vh = (double)(_window_h - (margin_yb_pix_ + margin_yt_pix_));
    // hvw - half viewport width, hwh - -"- height
    const float hvw = (float)(0.5 * vw);
    const float hvh = (float)(0.5 * vh);
    const float hvw_inv = (float)(2.0 / vw); // 1/hvw
    const float hvh_inv = (float)(2.0 / vh); // 1/hvh

    _viewport_to_clip.Set(
        hvw_inv, 0.0f,   0.0f, 0.0f,
         0.0f,  hvh_inv, 0.0f, 0.0f,
         0.0f,   0.0f,   1.0f, 0.0f,
        -1.0f,  -1.0f,   0.0f, 1.0f);
    _clip_to_viewport.Set(
        hvw,  0.0f, 0.0f, 0.0f,
        0.0f, hvh,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        hvw,  hvh,  0.0f, 1.0f);

    // Double precision matrix
    //_viewport_to_clip_hp.Set(
    //    2.0 / vw, 0.0,      0.0, 0.0,
    //     0.0,     2.0 / vh, 0.0, 0.0,
    //     0.0,     0.0,      1.0, 0.0,
    //    -1.0,    -1.0,      0.0, 1.0);

    // hww - half window width, hwh - -"- height
    const float hww = (float)(0.5 * (double)_window_w);
    const float hwh = (float)(0.5 * (double)_window_h);
    const float hww_inv = (float)(2.0 / (double)_window_w); // 1/hww
    const float hwh_inv = (float)(2.0 / (double)_window_h); // 1/hwh

    _screen_to_clip.Set(
        hww_inv,  0.0f,    0.0f, 0.0f,
         0.0f,   hwh_inv,  0.0f, 0.0f,
         0.0f,    0.0f,    1.0f, 0.0f,
        -1.0f,   -1.0f,    0.0f, 1.0f);
    _clip_to_screen.Set(
        hww,  0.0f, 0.0f, 0.0f,
        0.0f, hwh,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        hww,  hwh,  0.0f, 1.0f);

    // Double precision matrix
    //_screen_to_clip_hp.Set(
    //     2.0 / (double)_window_w, 0.0,                     0.0, 0.0,
    //     0.0,                     2.0 / (double)_window_h, 0.0, 0.0,
    //     0.0,                     0.0,                     1.0, 0.0,
    //    -1.0,                    -1.0,                     0.0, 1.0);

    prog_sel_q_.CommitCamera1(_screen_to_viewport, _viewport_to_clip, _screen_to_clip);
    prog_onscr_q_.CommitCamera1(_screen_to_viewport, _viewport_to_clip, _screen_to_clip);
    prog_w_.CommitCamera1(_screen_to_viewport, _viewport_to_clip, _screen_to_clip);
    prog_onscr_w_.CommitCamera1(_screen_to_viewport, _viewport_to_clip, _screen_to_clip);
    prog_m_.CommitCamera1(_screen_to_viewport, _viewport_to_clip, _screen_to_clip);
    prog_c_.CommitCamera1(_screen_to_viewport, _viewport_to_clip, _screen_to_clip);
}

template<typename T>
void Canvas<T>::UpdateMatricesPanZoom(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const float hdx     = (float)(0.5 * _visible_range.dx());
    const float hdy     = (float)(0.5 * _visible_range.dy());
    const float xm      = (float)(_visible_range.xm());
    const float ym      = (float)(_visible_range.ym());
    const float hdx_inv = (float)(2.0 / _visible_range.dx()); // 1/hdx
    const float hdy_inv = (float)(2.0 / _visible_range.dy()); // 1/hdy
    const float tx      = (float)(-_visible_range.twoxm() / _visible_range.dx()); // -2*xm/dx
    const float ty      = (float)(-_visible_range.twoym() / _visible_range.dy()); // -2*ym/dy

    _clip_to_visrange.Set(
        hdx,  0.0f, 0.0f, 0.0f,
        0.0f, hdy,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        xm,   ym,   0.0f, 1.0f);
    _visrange_to_clip.Set(
        hdx_inv, 0.0f,    0.0f, 0.0f,
        0.0f,    hdy_inv, 0.0f, 0.0f,
        0.0f,    0.0f,    1.0f, 0.0f,
        tx,      ty,      0.0f, 1.0f);

    // Double precision matrix
    //_clip_to_visrange_hp.Set(
    //    0.5 * _visible_range.dx(), 0.0,                       0.0, 0.0,
    //    0.0,                       0.5 * _visible_range.dy(), 0.0, 0.0,
    //    0.0,                       0.0,                       1.0, 0.0,
    //    _visible_range.xm(),       _visible_range.ym(),       0.0, 1.0);

    // Double precision matrix
    //_screen_to_visrange_hp = clip_to_visrange_hp * _screen_to_clip_hp;
    
    prog_sel_q_.CommitCamera2(_visrange_to_clip);
    prog_onscr_q_.CommitCamera2(_visrange_to_clip);
    prog_w_.CommitCamera2(_visrange_to_clip);
    prog_onscr_w_.CommitCamera2(_visrange_to_clip);
    prog_m_.CommitCamera2(_visrange_to_clip);
    prog_c_.CommitCamera2(_visrange_to_clip);
}

// ===============================================================================

template<typename T>
void Canvas<T>::UpdateSizeLimits(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    const int min_w = margin_xl_pix_ + MINFRAMEWIDTH + margin_xr_pix_;
    const int min_h = margin_yb_pix_ + MINFRAMEHEIGHT + margin_yt_pix_;
    const int min_w2 = (MINWINWIDTH > min_w) ? MINWINWIDTH : min_w;
    const int min_h2 = (MINWINHEIGHT > min_h) ? MINWINHEIGHT : min_h;
    glfwSetWindowSizeLimits(_window, min_w2, min_h2,
        GLFW_DONT_CARE, GLFW_DONT_CARE);
}

// ===============================================================================

//+++++++++++++++++++++++++++++++++++++++++++++
template<typename T>
void Canvas<T>::FinalizeTextRenderer(void)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    int win_w; int win_h;
    glfwGetWindowSize(_window, &win_w, &win_h);
    text_rend_.FirstReshape(win_w, win_h);
}
//+++++++++++++++++++++++++++++++++++++++++++++

// ===============================================================================

template<typename T>
void Canvas<T>::SetDarkColorScheme(void)
{
    _grid.SetDarkColorScheme();
    this->SetBackgroundColor(tiny_gl_text_renderer::colors::gray1);
    this->SetInFrameBackgroundColor(tiny_gl_text_renderer::colors::gray05);
    axes_line_color_  = tiny_gl_text_renderer::colors::gray75;
    vref_line_color_  = tiny_gl_text_renderer::colors::olive;
    frame_line_color_ = tiny_gl_text_renderer::colors::gray5;
    cursor_color_     = tiny_gl_text_renderer::colors::white;
    gen_text_color_   = tiny_gl_text_renderer::colors::white;
}

template<typename T>
void Canvas<T>::SetBrightColorScheme(void)
{
    _grid.SetBrightColorScheme();
    this->SetBackgroundColor(tiny_gl_text_renderer::colors::gray9);
    this->SetInFrameBackgroundColor(tiny_gl_text_renderer::colors::gray95);
    axes_line_color_  = tiny_gl_text_renderer::colors::gray25;
    vref_line_color_  = tiny_gl_text_renderer::colors::olive;
    frame_line_color_ = tiny_gl_text_renderer::colors::gray5;
    cursor_color_     = tiny_gl_text_renderer::colors::black;
    gen_text_color_   = tiny_gl_text_renderer::colors::black;
}

template<typename T>
void Canvas<T>::SetBackgroundColor(const color_t& color)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    background_color_ = color;
    glClearColor(background_color_[0], background_color_[1],
                 background_color_[2], background_color_[3]);
}

template<typename T>
void Canvas<T>::SetInFrameBackgroundColor(const color_t& color)
{
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    in_frame_bg_color_ = color;
    glProgramUniform4fv(prog_onscr_q_.GetProgId(), _fr_bg_unif_onscr_q, 1,
        in_frame_bg_color_.GetData());
}

// ===============================================================================

template class Canvas<float>;
template class Canvas<double>;

} // end of namespace tiny_graph_plot
