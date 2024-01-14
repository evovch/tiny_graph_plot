#include "canvas.h"

#include <cstdlib>
#include <cstdio>
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

//#define DEBUG_CALLS
#define SET_CONTEXT

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
:   UserWindow(window, w, h)
{
#ifdef DEBUG_CALLS
    printf("Canvas::Canvas\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    //this->UpdateSizeLimits();
    this->Init();
}

template<typename T>
Canvas<T>::~Canvas(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::~Canvas\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // VAOs, VBOs, IBOs ----------------------------------------------------------
    {
        glDeleteVertexArrays(1, &_vaoID_grid);
        glDeleteBuffers(1, &_vboID_grid);
        glDeleteBuffers(1, &_iboID_grid_w);
        glDeleteBuffers(1, &_iboID_grid_coarse_w);

        glDeleteVertexArrays(1, &_vaoID_axes);
        glDeleteBuffers(1, &_vboID_axes);
        glDeleteBuffers(1, &_iboID_axes_w);

        glDeleteVertexArrays(1, &_vaoID_vref);
        glDeleteBuffers(1, &_vboID_vref);
        glDeleteBuffers(1, &_iboID_vref_w);

        glDeleteVertexArrays(1, &_vaoID_frame);
        glDeleteBuffers(1, &_vboID_frame);
        glDeleteBuffers(1, &_iboID_frame_onscr_w);
        glDeleteBuffers(1, &_iboID_frame_onscr_q);

        glDeleteVertexArrays(1, &_vaoID_graphs);
        glDeleteBuffers(1, &_vboID_graphs);
        glDeleteBuffers(1, &_iboID_graphs_w);
        glDeleteBuffers(1, &_iboID_graphs_m);

        glDeleteVertexArrays(1, &_vaoID_cursor);
        glDeleteBuffers(1, &_vboID_cursor);
        glDeleteBuffers(1, &_iboID_cursor_onscr_w);

        glDeleteVertexArrays(1, &_vaoID_sel);
        glDeleteBuffers(1, &_vboID_sel);
        glDeleteBuffers(1, &_iboID_sel_q);
        glDeleteBuffers(1, &_iboID_sel_w);

        glDeleteVertexArrays(1, &_vaoID_c);
        glDeleteBuffers(1, &_vboID_c);
        glDeleteBuffers(1, &_iboID_c);
    }

    // Programs ------------------------------------------------------------------
    {
        glDeleteProgram(_progID_sel_q);
        glDeleteProgram(_progID_onscr_q);
        //glDeleteProgram(_progID_tr);
        glDeleteProgram(_progID_w);
        glDeleteProgram(_progID_onscr_w);
        glDeleteProgram(_progID_m);
        glDeleteProgram(_progID_c);
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
#ifdef DEBUG_CALLS
    printf("Canvas::Show\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    glfwShowWindow(_window);

    this->SendFrameVerticesToGPU();

    glProgramUniform1f(_progID_c, _circle_r_unif_c, (float)_circle_r);

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
        glBindVertexArray(0);
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

    const int ch_width = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    const int line_height = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_HEIGHT);
    const size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    int i_label = 0;
    int voffset = _v_offset;

    // Axis titles
    // X axis
    snprintf(&buf[0], BUFSIZE, "%s", _x_axis_title.c_str());
    const int line_len_x = (int)_x_axis_title.size();
    _x_axis_title_label_idx = _text_rend.AddLabel(buf,
        _window_w - _margin_xr_pix - line_len_x * ch_width,
        _window_h - _margin_yb_pix + line_height, _gen_text_color, _font_size);
    i_label++;
    // Y axis
    snprintf(&buf[0], BUFSIZE, "%s", _y_axis_title.c_str());
    const int line_len_y = (int)_y_axis_title.size();
    const int lbl_pos_x = (_y_axis_title_rotated ?
        _margin_xl_pix - line_height - line_height :
        _margin_xl_pix - line_height - line_len_y * ch_width);
    const int lbl_pos_y = (_y_axis_title_rotated ?
        _margin_yt_pix + line_len_y * ch_width :
        _margin_yt_pix);
    _y_axis_title_label_idx = _text_rend.AddLabel(buf, lbl_pos_x, lbl_pos_y,
        _gen_text_color, _font_size, (_y_axis_title_rotated ? 90.0f : 0.0f));
    i_label++;

    // Hint
    //const char test_line[] =
    //"\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz";
    //_hint_label_idx = _text_rend.AddLabel(
    //    "_y_axis_title_label_idx = _text_rend.AddLabel(buf, lbl_pos_x, lbl_pos_y,",
    //    //"Ctrl+left click to set the reference.\nPress 'F' to fit all in.",
    //    _h_offset, _window_h - 2 * line_height,
    //    tiny_gl_text_renderer::colors::silver, _font_size);
    //i_label++;

    // Grid parameters
    char buf2[128];
    snprintf(&buf2[0], 128, "(%g;%g) (%g;%g)",
        _grid.GetFineXstep(), _grid.GetFineYstep(),
        _grid.GetCoarseXstep(), _grid.GetCoarseYstep());
    const int grid_line_len = (int)strlen(buf2);
    _grid_params_label_idx = _text_rend.AddLabel(buf2,
        _window_w - _margin_xr_pix - grid_line_len * ch_width, _v_offset,
        //_window_h - _margin_yt_pix + line_height,
        _gen_text_color, _font_size);
    i_label++;

    // X axis values labels
    _x_axis_values_lables_start_idx = _text_rend.AddLabel(" ",
        _margin_xl_pix, _window_h - _margin_yb_pix,
        _gen_text_color, _font_size);
    i_label++;
    for (unsigned int i = 1; i < _n_x_axis_value_labels_max; i++) {
        _text_rend.AddLabel("",
            _margin_xl_pix, _window_h - _margin_yb_pix,
            _gen_text_color, _font_size);
        i_label++;
    }

    // Y axis values labels
    _y_axis_values_lables_start_idx = _text_rend.AddLabel("",
        _margin_xl_pix - line_height, _margin_yt_pix,
        _gen_text_color, _font_size, 90.0f);
    i_label++;
    for (unsigned int i = 1; i < _n_y_axis_value_labels_max; i++) {
        _text_rend.AddLabel("",
            _margin_xl_pix - line_height, _margin_yt_pix,
            _gen_text_color, _font_size, 90.0f);
        i_label++;
    }

    // Current values

    snprintf(&buf[0], BUFSIZE, "x =");
    _labels_start_idx = _text_rend.AddLabel(buf, _h_offset, voffset, _gen_text_color, _font_size);
    i_label++; voffset += line_height;
    snprintf(&buf[0], BUFSIZE, "y =");
    _text_rend.AddLabel(buf, _h_offset, voffset, _gen_text_color, _font_size);
    i_label++; voffset += line_height;
    int i_gr = 0;
    for (const auto* const gr : _graphs) {
        snprintf(&buf[0], BUFSIZE, "y%d=", i_gr);
        _text_rend.AddLabel(buf, _h_offset, voffset, gr->GetColor(), _font_size);
        i_label++; voffset += line_height;
        i_gr++;
    }

    // Reference values

    voffset += 2 * line_height;

    _ref_x = static_cast<T>(_total_xy_range.lowx());
    snprintf(&buf[0], BUFSIZE, "rx =% 0.4f", _ref_x);
    _text_rend.AddLabel(buf, _h_offset, voffset, _gen_text_color, _font_size);
    i_label++; voffset += line_height;
    i_gr = 0;
    for (const auto* const gr : _graphs) {
        const T ref_y = gr->Evaluate(_ref_x);
        snprintf(&buf[0], BUFSIZE, "ry%d=% 0.4f", i_gr, ref_y);
        _text_rend.AddLabel(buf, _h_offset, voffset, gr->GetColor(), _font_size);
        i_label++; voffset += line_height;
        i_gr++;
    }

    // Difference

    voffset += 2 * line_height;

    snprintf(&buf[0], BUFSIZE, "dx =");
    _text_rend.AddLabel(buf, _h_offset, voffset, _gen_text_color, _font_size);
    i_label++; voffset += line_height;
    i_gr = 0;
    for (const auto* const gr : _graphs) {
        snprintf(&buf[0], BUFSIZE, "dy%d=", i_gr);
        _text_rend.AddLabel(buf, _h_offset, voffset, gr->GetColor(), _font_size);
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
            _text_rend.AddLabel(buf, _h_offset, voffset, grj->GetColor(), _font_size);
            i_label++;
            _text_rend.AddLabel("-", _h_offset + 3*ch_width, voffset, _gen_text_color, _font_size);
            i_label++;
            snprintf(&buf[0], BUFSIZE, "y%d", i);
            _text_rend.AddLabel(buf, _h_offset + 5*ch_width, voffset, gri->GetColor(), _font_size);
            i_label++;
            _text_rend.AddLabel("=", _h_offset + 8*ch_width, voffset, _gen_text_color, _font_size);
            i_label++;
            voffset += line_height;
            _text_rend.AddLabel("", _h_offset, voffset, _gen_text_color, _font_size);
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
#ifdef DEBUG_CALLS
    printf("Canvas::Draw\n");
#endif
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
    for (const auto* const gr : _graphs) {
        if (gr->GetVisible()) {
            this->DrawDrawable(gr, cur_offset);
        }
        cur_offset += gr->GetSizeInfo();
    }
    for (const auto* const histo : _histograms) {
        if (histo->GetVisible()) {
            this->DrawDrawable(histo, cur_offset);
        }
        cur_offset += histo->GetSizeInfo();
    }

    this->SwitchToFullWindow();
    this->UpdateTexAxesValues();
    //++++++++++++++++
    _text_rend.Draw();
    //++++++++++++++++
}

template<typename T>
void Canvas<T>::Init(void) 
{
#ifdef DEBUG_CALLS
    printf("Canvas::Init\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // VAOs, VBOs, IBOs ----------------------------------------------------------
    {
        glGenVertexArrays(1, &_vaoID_grid);
        glGenBuffers(1, &_vboID_grid);
        glGenBuffers(1, &_iboID_grid_w);
        glGenBuffers(1, &_iboID_grid_coarse_w);

        glGenVertexArrays(1, &_vaoID_axes);
        glGenBuffers(1, &_vboID_axes);
        glGenBuffers(1, &_iboID_axes_w);

        glGenVertexArrays(1, &_vaoID_vref);
        glGenBuffers(1, &_vboID_vref);
        glGenBuffers(1, &_iboID_vref_w);

        glGenVertexArrays(1, &_vaoID_frame);
        glGenBuffers(1, &_vboID_frame);
        glGenBuffers(1, &_iboID_frame_onscr_w);
        glGenBuffers(1, &_iboID_frame_onscr_q);

        glGenVertexArrays(1, &_vaoID_graphs);
        glGenBuffers(1, &_vboID_graphs);
        glGenBuffers(1, &_iboID_graphs_w);
        glGenBuffers(1, &_iboID_graphs_m);

        glGenVertexArrays(1, &_vaoID_cursor);
        glGenBuffers(1, &_vboID_cursor);
        glGenBuffers(1, &_iboID_cursor_onscr_w);

        glGenVertexArrays(1, &_vaoID_sel);
        glGenBuffers(1, &_vboID_sel);
        glGenBuffers(1, &_iboID_sel_q);
        glGenBuffers(1, &_iboID_sel_w);

        glGenVertexArrays(1, &_vaoID_c);
        glGenBuffers(1, &_vboID_c);
        glGenBuffers(1, &_iboID_c);
    }

    // Programs and uniforms -----------------------------------------------------
    // Quads / visible range space
    {
        _progID_sel_q = glCreateProgram();
        GLuint vp_shader_sel_q = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vp_shader_sel_q, 1, (const GLchar**)&canvas_sel_q_vp_source, NULL);
        glCompileShader(vp_shader_sel_q);
        glAttachShader(_progID_sel_q, vp_shader_sel_q);
        GLuint fp_shader_sel_q = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fp_shader_sel_q, 1, (const GLchar**)&canvas_sel_q_fp_source, NULL);
        glCompileShader(fp_shader_sel_q);
        glAttachShader(_progID_sel_q, fp_shader_sel_q);
        glLinkProgram(_progID_sel_q);
        glDetachShader(_progID_sel_q, vp_shader_sel_q);
        glDetachShader(_progID_sel_q, fp_shader_sel_q);
        glDeleteShader(vp_shader_sel_q);
        glDeleteShader(fp_shader_sel_q);
        _s2v_unif_sel_q = glGetUniformLocation(_progID_sel_q, "screen2viewport");
        _v2c_unif_sel_q = glGetUniformLocation(_progID_sel_q, "viewport2clip");
        _s2c_unif_sel_q = glGetUniformLocation(_progID_sel_q, "screen2clip");
        _r2c_unif_sel_q = glGetUniformLocation(_progID_sel_q, "visrange2clip");
    }
    // Quads / screen space
    {
        _progID_onscr_q = glCreateProgram();
        GLuint vp_shader_onscr_q = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vp_shader_onscr_q, 1, (const GLchar**)&canvas_onscr_q_vp_source, NULL);
        glCompileShader(vp_shader_onscr_q);
        glAttachShader(_progID_onscr_q, vp_shader_onscr_q);
        GLuint fp_shader_onscr_q = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fp_shader_onscr_q, 1, (const GLchar**)&canvas_onscr_q_fp_source, NULL);
        glCompileShader(fp_shader_onscr_q);
        glAttachShader(_progID_onscr_q, fp_shader_onscr_q);
        glLinkProgram(_progID_onscr_q);
        glDetachShader(_progID_onscr_q, vp_shader_onscr_q);
        glDetachShader(_progID_onscr_q, fp_shader_onscr_q);
        glDeleteShader(vp_shader_onscr_q);
        glDeleteShader(fp_shader_onscr_q);
        _fr_bg_unif_onscr_q = glGetUniformLocation(_progID_onscr_q, "drawcolor");
        ////_s2v_unif_onscr_q = glGetUniformLocation(_progID_onscr_q, "screen2viewport");
        ////_v2c_unif_onscr_q = glGetUniformLocation(_progID_onscr_q, "viewport2clip");
        _s2c_unif_onscr_q = glGetUniformLocation(_progID_onscr_q, "screen2clip");
        ////_r2c_unif_onscr_q = glGetUniformLocation(_progID_onscr_q, "visrange2clip");
    }
    // Wires / visible range space
    {
        _progID_w = glCreateProgram();
        GLuint vp_shader_w = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vp_shader_w, 1, (const GLchar**)&canvas_w_vp_source, NULL);
        glCompileShader(vp_shader_w);
        glAttachShader(_progID_w, vp_shader_w);
        GLuint fp_shader_w = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fp_shader_w, 1, (const GLchar**)&canvas_w_fp_source, NULL);
        glCompileShader(fp_shader_w);
        glAttachShader(_progID_w, fp_shader_w);
        glLinkProgram(_progID_w);
        glDetachShader(_progID_w, vp_shader_w);
        glDetachShader(_progID_w, fp_shader_w);
        glDeleteShader(vp_shader_w);
        glDeleteShader(fp_shader_w);
        _s2v_unif_w = glGetUniformLocation(_progID_w, "screen2viewport");
        _v2c_unif_w = glGetUniformLocation(_progID_w, "viewport2clip");
        _s2c_unif_w = glGetUniformLocation(_progID_w, "screen2clip");
        _r2c_unif_w = glGetUniformLocation(_progID_w, "visrange2clip");
    }
    // Wires / screen space
    {
        _progID_onscr_w = glCreateProgram();
        GLuint vp_shader_onscr_w = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vp_shader_onscr_w, 1, (const GLchar**)&canvas_onscr_w_vp_source, NULL);
        glCompileShader(vp_shader_onscr_w);
        glAttachShader(_progID_onscr_w, vp_shader_onscr_w);
        GLuint fp_shader_onscr_w = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fp_shader_onscr_w, 1, (const GLchar**)&canvas_onscr_w_fp_source, NULL);
        glCompileShader(fp_shader_onscr_w);
        glAttachShader(_progID_onscr_w, fp_shader_onscr_w);
        glLinkProgram(_progID_onscr_w);
        glDetachShader(_progID_onscr_w, vp_shader_onscr_w);
        glDetachShader(_progID_onscr_w, fp_shader_onscr_w);
        glDeleteShader(vp_shader_onscr_w);
        glDeleteShader(fp_shader_onscr_w);
        ////_s2v_unif_onscr_w = glGetUniformLocation(_progID_onscr_w, "screen2viewport");
        ////_v2c_unif_onscr_w = glGetUniformLocation(_progID_onscr_w, "viewport2clip");
        _s2c_unif_onscr_w = glGetUniformLocation(_progID_onscr_w, "screen2clip");
        ////_r2c_unif_onscr_w = glGetUniformLocation(_progID_onscr_w, "visrange2clip");
    }
    // Markers / visible range space
    {
        _progID_m = glCreateProgram();
        GLuint vp_shader_m = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vp_shader_m, 1, (const GLchar**)&canvas_m_vp_source, NULL);
        glCompileShader(vp_shader_m);
        glAttachShader(_progID_m, vp_shader_m);
        GLuint fp_shader_m = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fp_shader_m, 1, (const GLchar**)&canvas_m_fp_source, NULL);
        glCompileShader(fp_shader_m);
        glAttachShader(_progID_m, fp_shader_m);
        glLinkProgram(_progID_m);
        glDetachShader(_progID_m, vp_shader_m);
        glDetachShader(_progID_m, fp_shader_m);
        glDeleteShader(vp_shader_m);
        glDeleteShader(fp_shader_m);
        _s2v_unif_m = glGetUniformLocation(_progID_m, "screen2viewport");
        _v2c_unif_m = glGetUniformLocation(_progID_m, "viewport2clip");
        _s2c_unif_m = glGetUniformLocation(_progID_m, "screen2clip");
        _r2c_unif_m = glGetUniformLocation(_progID_m, "visrange2clip");
    }
    // Circles / visible range space
    {
        _progID_c = glCreateProgram();
        GLuint vp_shader_c = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vp_shader_c, 1, (const GLchar**)&canvas_c_vp_source, NULL);
        glCompileShader(vp_shader_c);
        glAttachShader(_progID_c, vp_shader_c);
        GLuint gp_shader_c = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gp_shader_c, 1, (const GLchar**)&canvas_c_gp_source, NULL);
        glCompileShader(gp_shader_c);
        glAttachShader(_progID_c, gp_shader_c);
        GLuint fp_shader_c = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fp_shader_c, 1, (const GLchar**)&canvas_c_fp_source, NULL);
        glCompileShader(fp_shader_c);
        glAttachShader(_progID_c, fp_shader_c);
        glLinkProgram(_progID_c);
        glDetachShader(_progID_c, vp_shader_c);
        glDetachShader(_progID_c, gp_shader_c);
        glDetachShader(_progID_c, fp_shader_c);
        glDeleteShader(vp_shader_c);
        glDeleteShader(gp_shader_c);
        glDeleteShader(fp_shader_c);
        //_s2v_unif_c = glGetUniformLocation(_progID_c, "screen2viewport");
        _v2c_unif_c = glGetUniformLocation(_progID_c, "viewport2clip");
        //_s2c_unif_c = glGetUniformLocation(_progID_c, "screen2clip");
        _r2c_unif_c = glGetUniformLocation(_progID_c, "visrange2clip");
        _circle_r_unif_c = glGetUniformLocation(_progID_c, "circle_r");
    }

    // Send fixed data to GPU ----------------------------------------------------

    this->AllocateBuffersForFixedSizedData();
    this->SendFixedIndicesToGPU();

    // Initialize different OpenGL parameters

    glClearColor(_background_color[0], _background_color[1],
                 _background_color[2], _background_color[3]);
    glEnable(GL_PROGRAM_POINT_SIZE);
    //glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
    // This has to be done after the program has been compiled and the uniform
    // variable located
    glProgramUniform4fv(_progID_onscr_q, _fr_bg_unif_onscr_q, 1,
        _in_frame_bg_color.GetData());
    // In principle, can be omitted
    //glProgramUniform1f(_progID_c, _circle_r_unif_c, (float)_circle_r);
}

template<typename T>
void Canvas<T>::Clear(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::Clear\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    glClear(GL_COLOR_BUFFER_BIT);
    this->FillInFrame();
}

template<typename T>
void Canvas<T>::Reshape(int p_width, int p_height)
{
#ifdef DEBUG_CALLS
    printf("Canvas::Reshape\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    this->UpdateMatricesReshape();

    this->SendFrameVerticesToGPU();

    this->SendGridToGPU();

    //++++++++++++++++++++++++++++++++++++
    _text_rend.Reshape(p_width, p_height);
    //++++++++++++++++++++++++++++++++++++

    const int ch_width = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    const int line_height = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_HEIGHT);

    const int line_len_x = (int)_x_axis_title.size();
    _text_rend.UpdatePosition(
        _window_w - _margin_xr_pix - line_len_x * ch_width,
        _window_h - _margin_yb_pix + line_height,
        _x_axis_title_label_idx);
    //_text_rend.UpdatePosition(10, _window_h - 2 * line_height, _hint_label_idx);

    for (unsigned int i = 0; i < _n_x_axis_value_labels_max; i++) {
        _text_rend.UpdatePositionY(_window_h - _margin_yb_pix,
            _x_axis_values_lables_start_idx + (size_t)i);
    }
    for (unsigned int i = 0; i < _n_y_axis_value_labels_max; i++) {
        _text_rend.UpdatePositionX(_margin_xl_pix - line_height,
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
/*__forceinline*/
void Canvas<T>::SwitchToFullWindow(void) const
{
    glViewport(0, 0, _window_w, _window_h);
}

template<typename T>
/*__forceinline*/
void Canvas<T>::SwitchToFrame(void) const
{
    glViewport(_margin_xl_pix, _margin_yb_pix,
        _window_w - (_margin_xl_pix + _margin_xr_pix),
        _window_h - (_margin_yb_pix + _margin_yt_pix));
}

// ===============================================================================

template<typename T>
void Canvas<T>::AllocateBuffersForFixedSizedData(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::AllocateBuffersForFixedSizedData\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // Allocate vertex buffer space for the axes ---------------------------------
    {
        const unsigned int n_vert = 4;
        glBindVertexArray(_vaoID_axes);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_axes);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            NULL, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    // Allocate vertex buffer space for the vref line ----------------------------
    {
        const unsigned int n_vert = 2;
        glBindVertexArray(_vaoID_vref);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_vref);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            NULL, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    // Allocate vertex buffer space for the frame --------------------------------
    {
        const unsigned int n_vert = 4;
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
        glBindVertexArray(0);
    }

    // Allocate vertex buffer space for the cursor -------------------------------
    {
        const unsigned int n_vert = 4;
        glBindVertexArray(_vaoID_cursor);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_cursor);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            NULL, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    // Allocate vertex buffer space for the selection rectange -------------------
    {
        const unsigned int n_vert = 4;
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
        glBindVertexArray(0);
    }
}

template<typename T>
void Canvas<T>::SendFixedIndicesToGPU(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::SendFixedIndicesToGPU\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // Axes
    {
        const unsigned int n_wires_axes = 2;
        const wire_t wires[n_wires_axes] = { {0, 1}, {2, 3} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_axes_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_axes * sizeof(wire_t), wires,
            GL_STATIC_DRAW);
    }

    // Vref
    {
        const unsigned int n_wires_vref = 1;
        const wire_t wires[n_wires_vref] = { {0, 1} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_vref_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_vref * sizeof(wire_t),
            wires, GL_STATIC_DRAW);
    }

    // Frame
    {
        const unsigned int n_wires_frame = 4;
        const wire_t wires[n_wires_frame] = { {0,1}, {1,2}, {2,3}, {3,0} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_frame * sizeof(wire_t),
            wires, GL_STATIC_DRAW);
        const unsigned int n_quads_frame = 1;
        const quad_t quads[n_quads_frame] = { { 0, 1, 2, 3 } };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_q);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_quads_frame * sizeof(quad_t),
            quads, GL_STATIC_DRAW);
    }

    // Cursor
    {
        const unsigned int n_wires_cursor = 2;
        const wire_t wires[n_wires_cursor] = { {0, 1}, {2, 3} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_cursor_onscr_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_cursor * sizeof(wire_t),
            wires, GL_STATIC_DRAW);
    }

    // Selection rectangle
    {
        const unsigned int n_wires_sel = 4;
        const wire_t wires[n_wires_sel] = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_w);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_wires_sel * sizeof(wire_t),
            wires, GL_STATIC_DRAW);
        const unsigned int n_quads_sel = 1;
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
#ifdef DEBUG_CALLS
    printf("Canvas::SendGridToGPU\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const float vw = (float)(_window_w - (_margin_xl_pix + _margin_xr_pix));
    const float vh = (float)(_window_h - (_margin_yb_pix + _margin_yt_pix));
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
        glBindVertexArray(0);
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
#ifdef DEBUG_CALLS
    printf("Canvas::DrawGrid\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!_enable_hgrid && !_enable_vgrid) {
        return;
    }

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

        glUseProgram(_progID_w);
        glBindVertexArray(_vaoID_grid);
        if (_enable_vgrid) {
            // Fine grid
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_w);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x0101);
            glLineWidth(_grid.GetVGridFineLineWidth());
            glDrawElements(GL_LINES, 2 * n_wires_fine_x, GL_UNSIGNED_INT, NULL);
            glDisable(GL_LINE_STIPPLE);
            // Coarse grid
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_coarse_w);
            glLineWidth(_grid.GetVGridCoarseLineWidth());
            glDrawElements(GL_LINES, 2 * n_wires_coarse_x, GL_UNSIGNED_INT, NULL);
        }
        if (_enable_hgrid) {
            // Fine grid
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_w);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x0101);
            glLineWidth(_grid.GetHGridFineLineWidth());
            glDrawElements(GL_LINES, 2 * n_wires_fine_y, GL_UNSIGNED_INT,
                (GLvoid*)((size_t)(n_wires_fine_x) * sizeof(wire_t)));
            glDisable(GL_LINE_STIPPLE);
            // Coarse grid
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_grid_coarse_w);
            glLineWidth(_grid.GetHGridCoarseLineWidth());
            glDrawElements(GL_LINES, 2 * n_wires_coarse_y, GL_UNSIGNED_INT,
                (GLvoid*)((size_t)(n_wires_coarse_x) * sizeof(wire_t)));
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 2. Axes =======================================================================

template<typename T>
void Canvas<T>::DrawAxes(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::DrawAxes\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!_enable_axes) {
        return;
    }

    // Send vertices and colors. -------------------------------------------------
    {
        const unsigned int n_vert = 4;
        vertex_colored_t vertices[n_vert];
        vertices[0].coords_ = point_t(
            static_cast<float>(_total_xy_range.lowx()), 0.0f, 0.0f, 1.0f);
        vertices[1].coords_ = point_t(
            static_cast<float>(_total_xy_range.highx()), 0.0f, 0.0f, 1.0f);
        vertices[2].coords_ = point_t(
            0.0f, static_cast<float>(_total_xy_range.lowy()), 0.0f, 1.0f);
        vertices[3].coords_ = point_t(
            0.0f, static_cast<float>(_total_xy_range.highy()), 0.0f, 1.0f);
        vertices[0].color_ = _axes_line_color;
        vertices[1].color_ = _axes_line_color;
        vertices[2].color_ = _axes_line_color;
        vertices[3].color_ = _axes_line_color;

        glBindVertexArray(_vaoID_axes);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_axes);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            n_vert * sizeof(vertex_colored_t), vertices);
        glBindVertexArray(0);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        const unsigned int n_wires = 2;
        glUseProgram(_progID_w);
        glBindVertexArray(_vaoID_axes);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_axes_w);
        glLineWidth(_axes_line_width);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 3. Vref =======================================================================

template<typename T>
void Canvas<T>::DrawVref(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::DrawVref\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!_enable_vref) {
        return;
    }

    // Send vertices and colors. -------------------------------------------------
    {
        const unsigned int n_vert = 2;
        vertex_colored_t vertices[n_vert];
        vertices[0].coords_ = point_t(static_cast<float>(_ref_x),
            static_cast<float>(_total_xy_range.lowy()), 0.0f, 1.0f);
        vertices[1].coords_ = point_t(static_cast<float>(_ref_x),
            static_cast<float>(_total_xy_range.highy()), 0.0f, 1.0f);
        vertices[0].color_ = _vref_line_color;
        vertices[1].color_ = _vref_line_color;

        glBindVertexArray(_vaoID_vref);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_vref);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            n_vert * sizeof(vertex_colored_t), vertices);
        glBindVertexArray(0);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        const unsigned int n_wires = 1;
        glUseProgram(_progID_w);
        glBindVertexArray(_vaoID_vref);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_vref_w);
        glLineWidth(_vref_line_width);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 4. Frame ======================================================================

template<typename T>
void Canvas<T>::SendFrameVerticesToGPU(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::SendFrameVerticesToGPU\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // Send vertices and colors. -------------------------------------------------
    {
        const unsigned int n_vert = 4;
        vertex_colored_t vertices[n_vert];
        const float blx_ = (float)_margin_xl_pix;
        const float bly_ = (float)_margin_yb_pix;
        const float trx_ = (float)_window_w - (float)_margin_xr_pix;
        const float try_ = (float)_window_h - (float)_margin_yt_pix;
        vertices[0].coords_ = point_t(blx_, bly_, 0.0f, 1.0f);
        vertices[1].coords_ = point_t(trx_, bly_, 0.0f, 1.0f);
        vertices[2].coords_ = point_t(trx_, try_, 0.0f, 1.0f);
        vertices[3].coords_ = point_t(blx_, try_, 0.0f, 1.0f);
        vertices[0].color_ = _frame_line_color;
        vertices[1].color_ = _frame_line_color;
        vertices[2].color_ = _frame_line_color;
        vertices[3].color_ = _frame_line_color;

        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_frame);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            n_vert * sizeof(vertex_colored_t), vertices);
        glBindVertexArray(0);
    }
}

template<typename T>
void Canvas<T>::FillInFrame(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::FillInFrame\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    // In principle, filling of the in frame space need not necessarily be
    // controlled by this flag, it could be another flag.
    if (!_enable_frame) {
        return;
    }

    this->SwitchToFullWindow(); //TODO move outside?

    // Draw quads. Quads indices have already been sent. -------------------------
    {
        const unsigned int n_quads = 1;
        glUseProgram(_progID_onscr_q);
        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_q);
        glDrawElements(GL_QUADS, 4 * n_quads, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

template<typename T>
void Canvas<T>::DrawFrame(void) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::DrawFrame\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!_enable_frame) {
        return;
    }

    // Draw wires. Wires indices have already been sent. -------------------------
    {
        const unsigned int n_wires = 4;
        glUseProgram(_progID_onscr_w);
        glBindVertexArray(_vaoID_frame);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_frame_onscr_w);
        glLineWidth(2.0f);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 5. Graphs =====================================================================

template<typename T>
void Canvas<T>::SendDrawableToGPU(const Drawable<T>* const p_graph, const SizeInfo& p_offset) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::SendGraphToGPU\n");
#endif
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
        glBindVertexArray(0);

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
#ifdef DEBUG_CALLS
    printf("Canvas::DrawGraph\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const SizeInfo& cur_size = p_graph->GetSizeInfo();

    // Draw markers. Markers indices have already been sent. ---------------------
    {
        glUseProgram(_progID_m);
        glBindVertexArray(_vaoID_graphs);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_m);
        glPointSize(p_graph->GetMarkerSize());
        glDrawElementsBaseVertex(GL_POINTS, 1 * cur_size._n_m, GL_UNSIGNED_INT,
            (GLvoid*)(p_offset._n_m * sizeof(marker_t)), (GLint)p_offset._n_v);
        glBindVertexArray(0);
        glUseProgram(0);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        glUseProgram(_progID_w);
        glBindVertexArray(_vaoID_graphs);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_graphs_w);
        glLineWidth(p_graph->GetLineWidth());
        glDrawElementsBaseVertex(GL_LINES, 2 * cur_size._n_w, GL_UNSIGNED_INT,
            (GLvoid*)(p_offset._n_w * sizeof(wire_t)), (GLint)p_offset._n_v);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 6. Cursor =====================================================================

template<typename T>
void Canvas<T>::DrawCursor(const double xs, const double ys) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::DrawCursor\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!_enable_cursor) {
        return;
    }

    this->SwitchToFullWindow(); //TODO move outside?

    // Send vertices and colors. -------------------------------------------------
    {
        const unsigned int n_vert = 4;
        vertex_colored_t vertices[n_vert];
        const float blx_ = (float)_margin_xl_pix;
        const float bly_ = (float)_margin_yb_pix;
        const float trx_ = (float)_window_w - (float)_margin_xr_pix;
        const float try_ = (float)_window_h - (float)_margin_yt_pix;
        vertices[0].coords_ = point_t((float)xs, bly_, 0.0f, 1.0f);
        vertices[1].coords_ = point_t((float)xs, try_, 0.0f, 1.0f);
        vertices[2].coords_ = point_t(blx_, (float)ys, 0.0f, 1.0f);
        vertices[3].coords_ = point_t(trx_, (float)ys, 0.0f, 1.0f);
        vertices[0].color_ = _cursor_color;
        vertices[1].color_ = _cursor_color;
        vertices[2].color_ = _cursor_color;
        vertices[3].color_ = _cursor_color;

        glBindVertexArray(_vaoID_cursor);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_cursor);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            n_vert * sizeof(vertex_colored_t), vertices);
        glBindVertexArray(0);
    }
    // Draw wires. Wires indices have already been sent. -------------------------
    {
        const unsigned int n_wires = 2;
        glUseProgram(_progID_onscr_w);
        glBindVertexArray(_vaoID_cursor);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_cursor_onscr_w);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x00FF);
        glLineWidth(_cursor_line_width);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        glDisable(GL_LINE_STIPPLE);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 7. Selection rectangle ========================================================

template<typename T>
void Canvas<T>::DrawSelRectangle(const double xs0, const double ys0,
    const double xs1, const double ys1) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::DrawSelRectangle\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    this->SwitchToFrame(); //TODO move outside?

    // Send vertices and colors. -------------------------------------------------
    {
        const unsigned int n_vert = 4;
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
        glBindVertexArray(0);
    }

    // Draw. ---------------------------------------------------------------------
    {
        // Draw wires. Wires indices have already been sent. ---------------------
        const unsigned int n_wires = 4;
        glUseProgram(_progID_w);
        glBindVertexArray(_vaoID_sel);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_w);
        glLineWidth(2.0f);
        glDrawElements(GL_LINES, 2 * n_wires, GL_UNSIGNED_INT, NULL);
        //glBindVertexArray(0);
        //glUseProgram(0);

        // Draw quads. Quads indices have already been sent. ---------------------
        const unsigned int n_quads = 1;
        glUseProgram(_progID_sel_q);
        glBindVertexArray(_vaoID_sel);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_sel_q);
        glDrawElements(GL_QUADS, 4 * n_quads, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 8. Circles ====================================================================

template<typename T>
void Canvas<T>::DrawCircles(const double xs, const double ys) const
{
#ifdef DEBUG_CALLS
    printf("Canvas::DrawCircles\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    if (!_enable_circles) {
        return;
    }

    this->SwitchToFrame(); //TODO move outside?

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

        glBindVertexArray(_vaoID_c);
        glBindBuffer(GL_ARRAY_BUFFER, _vboID_c);
        glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(vertex_colored_t),
            vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, coords_));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_colored_t),
            (void*)offsetof(vertex_colored_t, color_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_c);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_markers * sizeof(vertex_colored_t),
            markers, GL_STATIC_DRAW);

        if (vertices != nullptr) delete[] vertices;
        if (markers != nullptr) delete[] markers;
    }
    // Draw. ---------------------------------------------------------------------
    {
        glUseProgram(_progID_c);
        glBindVertexArray(_vaoID_c);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID_c);
        glDrawElements(GL_POINTS, 1 * n_markers, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

// 9. Text =======================================================================

template<typename T>
void Canvas<T>::UpdateTexTextCur(const double xs, const double ys)
{
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateTexTextCur\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const Vec4f pr = this->TransformToVisrange(xs, ys);
    const T x = static_cast<T>(pr.x());
    const T y = static_cast<T>(pr.y());
    const T& x0 = _ref_x;

    const size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    int i_lab1 = (int)_labels_start_idx;
    int i_lab2 = (int)_labels_start_idx + 1 + 2 * (int)(_graphs.size() + 1);

    snprintf(&buf[0], BUFSIZE, "x =% 0.4f", x);
    _text_rend.UpdateLabel(buf, i_lab1);
    i_lab1++;
    snprintf(&buf[0], BUFSIZE, "y =% 0.4f", y);
    _text_rend.UpdateLabel(buf, i_lab1);
    i_lab1++;
    snprintf(&buf[0], BUFSIZE, "dx =% 0.4f", x - x0);
    _text_rend.UpdateLabel(buf, i_lab2);
    i_lab2++;

    int i_gr = 0;
    for (const auto* const gr : _graphs) {
        const T y0 = gr->Evaluate(x0);
        const T yi = gr->Evaluate(x);
        snprintf(&buf[0], BUFSIZE, "y%d=% 0.4f", i_gr, yi);
        _text_rend.UpdateLabel(buf, i_lab1);
        snprintf(&buf[0], BUFSIZE, "dy%d=% 0.4f", i_gr, yi - y0);
        _text_rend.UpdateLabel(buf, i_lab2);
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
            _text_rend.UpdateLabel(buf, i_lab3);
            i_lab3 += 5;
        }
    }
}

template<typename T>
void Canvas<T>::UpdateTexTextRef(const double xs, const double ys)
{
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateTexTextRef\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const Vec4f pr = this->TransformToVisrange(xs, ys);

    if (!_total_xy_range.IncludesX(pr.x())) return;

    _ref_x = pr.x();

    const size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    int i_lab = (int)_labels_start_idx + 1 + (int)(_graphs.size() + 1);
    snprintf(&buf[0], BUFSIZE, "rx =% 0.4f", _ref_x);
    _text_rend.UpdateLabel(buf, i_lab);
    i_lab++;

    int i_gr = 0;
    for (const auto* const gr : _graphs) {
        const T y0 = gr->Evaluate(_ref_x);
        snprintf(&buf[0], BUFSIZE, "ry%d=% 0.4f", i_gr, y0);
        _text_rend.UpdateLabel(buf, i_lab);
        i_lab++;
        i_gr++;
    }
}

template<typename T>
void Canvas<T>::UpdateTexTextGridSize(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateTUpdateTexTextGridSizeexTextRef\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const int ch_width = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    //const int line_height = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_HEIGHT);

    char buf2[128];
    snprintf(&buf2[0], 128, "(%g;%g) (%g;%g)",
        _grid.GetFineXstep(), _grid.GetFineYstep(),
        _grid.GetCoarseXstep(), _grid.GetCoarseYstep());
    const int grid_line_len = (int)strlen(buf2);
    _text_rend.UpdateLabel(buf2, _grid_params_label_idx);
    _text_rend.UpdatePosition(_window_w - _margin_xr_pix -
        grid_line_len * ch_width, _v_offset, _grid_params_label_idx);
}

template<typename T>
void Canvas<T>::UpdateTexAxesValues(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateTexAxesValues\n");
#endif
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

    const int ch_width = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_WIDTH);
    //const int line_height = (int)(_font_size * (float)tiny_gl_text_renderer::CHAR_HEIGHT);
    constexpr size_t BUFSIZE = 32;
    char buf[BUFSIZE];

    // X axis --------------------------------------------------------------------

    for (unsigned int i = 0; i < nx; i++) {
        const unsigned int idx0 = wires[i].v0;
        //const unsigned int idx1 = wires[i].v1;

        snprintf(buf, BUFSIZE, "%g", vertices[idx0].coords_.x());
        const int offset = -(ch_width * (int)strlen(buf)) / 2;
        _text_rend.UpdateLabel(buf, _x_axis_values_lables_start_idx + (size_t)i);

        const Vec4f vr(vertices[idx0].coords_.x(), 0.0f, 0.0f, 1.0f);
        const Vec4f vc = _visrange_to_clip * vr;
        //const Vec4f vs = _clip_to_screen * vc;
        const Vec4f vv = _clip_to_viewport * vc;
        const Vec4f vs = _viewport_to_screen * vv;

        _text_rend.UpdatePositionX((int)(vs.x()) + offset,
            _x_axis_values_lables_start_idx + (size_t)i);
    }
    for (unsigned int i = nx; i < _n_x_axis_value_labels_max; i++) {
        _text_rend.UpdateLabel(" ", _x_axis_values_lables_start_idx + (size_t)i);
    }

    // Y axis --------------------------------------------------------------------

    for (unsigned int i = 0; i < ny; i++) {
        const unsigned int idx0 = wires[nx_+i].v0;
        //const unsigned int idx1 = wires[nx_+i].v1;

        snprintf(buf, BUFSIZE, "%g", vertices[idx0].coords_.y());
        const int offset = (ch_width * (int)strlen(buf)) / 2;
        _text_rend.UpdateLabel(buf, _y_axis_values_lables_start_idx + (size_t)i);

        const Vec4f vr(0.0f, vertices[idx0].coords_.y(), 0.0f, 1.0f);
        const Vec4f vc = _visrange_to_clip * vr;
        //const Vec4f vs = _clip_to_screen * vc;
        const Vec4f vv = _clip_to_viewport * vc;
        const Vec4f vs = _viewport_to_screen * vv;

        _text_rend.UpdatePositionY(_window_h - (int)(vs.y()) + offset,
            _y_axis_values_lables_start_idx + (size_t)i);
    }
    for (unsigned int i = ny; i < _n_y_axis_value_labels_max; i++) {
        _text_rend.UpdateLabel(" ", _y_axis_values_lables_start_idx + (size_t)i);
    }
}

// ===============================================================================

template<typename T>
void Canvas<T>::CenterView(const double xs, const double ys)
{
#ifdef DEBUG_CALLS
    printf("Canvas::Center: % 0.6f\t% 0.6f\n", xs, ys);
#endif
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif

    const Vec4f pr = this->TransformToVisrange(xs, ys);
    const float cur_center_x = 0.5f * _visible_range.twoxm();
    const float cur_center_y = 0.5f * _visible_range.twoym();
    _visible_range.MoveX((float)pr.x() - cur_center_x);
    _visible_range.MoveY((float)pr.y() - cur_center_y);

    this->UpdateMatricesPanZoom();
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::Pan(const double xs, const double ys)
{
#ifdef DEBUG_CALLS
    printf("Canvas::Pan: % 0.6f\t% 0.6f\n", xs, ys);
#endif
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
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::Zoom(const double xs, const double ys)
{
#ifdef DEBUG_CALLS
    printf("Canvas::Zoom: % 0.6f\t% 0.6f\n", xs, ys);
#endif
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
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::ZoomF(const double xs, const double ys)
{
    (void)xs;
    const float vw = (float)(_window_w - (_margin_xl_pix + _margin_xr_pix));
    const float vh = (float)(_window_h - (_margin_yb_pix + _margin_yt_pix));
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
#ifdef DEBUG_CALLS
    printf("Canvas::ZoomTo: % 0.6f\t% 0.6f\t% 0.6f\t% 0.6f\n",
        xs0, ys0, xs1, ys1);
#endif
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
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::ResetCamera(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::ResetCamera\n");
#endif
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    _visible_range = _total_xy_range;
    this->UpdateMatricesReshape();
    this->UpdateMatricesPanZoom();
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::SetPrevViewport(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::SetPrevViewport\n");
#endif
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    _visible_range = _visible_range_start;
    this->UpdateMatricesReshape();
    this->UpdateMatricesPanZoom();
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::FixedAspRatCamera(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::FixedAspRatCamera\n");
#endif
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif

    // Everything in double here

    // Just to shorten the notation
    const double vw = (double)(_window_w - (_margin_xl_pix + _margin_xr_pix));
    const double vh = (double)(_window_h - (_margin_yb_pix + _margin_yt_pix));
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
    int res = this->SendGridToGPU();
    if (res == 0) { this->UpdateTexTextGridSize(); }
}

template<typename T>
void Canvas<T>::ExportSnapshot(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::ExportSnapshot\n");
#endif

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
        (xs_i >= (int)_margin_xl_pix) &&
        (xs_i <= ((int)_window_w - (int)_margin_xr_pix)) &&
        (ys_i >= (int)_margin_yb_pix) &&
        (ys_i <= ((int)_window_h - (int)_margin_yt_pix)));
}

template<typename T>
void Canvas<T>::ClampToFrame(const double xs, const double ys, double& o_xs, double& o_ys) const
{
////#ifdef SET_CONTEXT
////    glfwMakeContextCurrent(_window);
////#endif
    const double left_boundary = (double)_margin_xl_pix;
    const double right_boundary = (double)_window_w - (double)_margin_xr_pix;
    const double bottom_boundary = (double)_margin_yb_pix;
    const double top_boundary = (double)_window_h - (double)_margin_yt_pix;
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
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateMatricesReshape\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif

    const float ax = (float)_margin_xl_pix;
    const float ay = (float)_margin_yb_pix;

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
    //    -(double)_margin_xl_pix, -(double)_margin_yb_pix, 0.0, 1.0);

    const double vw = (double)(_window_w - (_margin_xl_pix + _margin_xr_pix));
    const double vh = (double)(_window_h - (_margin_yb_pix + _margin_yt_pix));
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

    glProgramUniformMatrix4fv(_progID_sel_q, _s2v_unif_sel_q, 1, GL_FALSE, _screen_to_viewport.GetData());
    glProgramUniformMatrix4fv(_progID_sel_q, _v2c_unif_sel_q, 1, GL_FALSE, _viewport_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_sel_q, _s2c_unif_sel_q, 1, GL_FALSE, _screen_to_clip.GetData());

    //glProgramUniformMatrix4fv(_progID_onscr_q, _s2v_unif_onscr_q, 1, GL_FALSE, _screen_to_viewport.GetData());
    //glProgramUniformMatrix4fv(_progID_onscr_q, _v2c_unif_onscr_q, 1, GL_FALSE, _viewport_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_onscr_q, _s2c_unif_onscr_q, 1, GL_FALSE, _screen_to_clip.GetData());

    //glProgramUniformMatrix4fv(_progID_tr, _s2v_unif_tr, 1, GL_FALSE, _screen_to_viewport.GetData());
    //glProgramUniformMatrix4fv(_progID_tr, _v2c_unif_tr, 1, GL_FALSE, _viewport_to_clip.GetData());
    //glProgramUniformMatrix4fv(_progID_tr, _s2c_unif_tr, 1, GL_FALSE, _screen_to_clip.GetData());

    glProgramUniformMatrix4fv(_progID_w, _s2v_unif_w, 1, GL_FALSE, _screen_to_viewport.GetData());
    glProgramUniformMatrix4fv(_progID_w, _v2c_unif_w, 1, GL_FALSE, _viewport_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_w, _s2c_unif_w, 1, GL_FALSE, _screen_to_clip.GetData());

    ////glProgramUniformMatrix4fv(_progID_onscr_w, _s2v_unif_onscr_w, 1, GL_FALSE, _screen_to_viewport.GetData());
    ////glProgramUniformMatrix4fv(_progID_onscr_w, _v2c_unif_onscr_w, 1, GL_FALSE, _viewport_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_onscr_w, _s2c_unif_onscr_w, 1, GL_FALSE, _screen_to_clip.GetData());

    glProgramUniformMatrix4fv(_progID_m, _s2v_unif_m, 1, GL_FALSE, _screen_to_viewport.GetData());
    glProgramUniformMatrix4fv(_progID_m, _v2c_unif_m, 1, GL_FALSE, _viewport_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_m, _s2c_unif_m, 1, GL_FALSE, _screen_to_clip.GetData());

    //glProgramUniformMatrix4fv(_progID_c, _s2v_unif_c, 1, GL_FALSE, _screen_to_viewport.GetData());
    glProgramUniformMatrix4fv(_progID_c, _v2c_unif_c, 1, GL_FALSE, _viewport_to_clip.GetData());
    //glProgramUniformMatrix4fv(_progID_c, _s2c_unif_c, 1, GL_FALSE, _screen_to_clip.GetData());
}

template<typename T>
void Canvas<T>::UpdateMatricesPanZoom(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateMatricesPanZoom\n");
#endif
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

    glProgramUniformMatrix4fv(_progID_sel_q,   _r2c_unif_sel_q,   1, GL_FALSE, _visrange_to_clip.GetData());
    ////glProgramUniformMatrix4fv(_progID_onscr_q, _r2c_unif_onscr_q, 1, GL_FALSE, _visrange_to_clip.GetData());
    //glProgramUniformMatrix4fv(_progID_tr,      _r2c_unif_tr,      1, GL_FALSE, _visrange_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_w,       _r2c_unif_w,       1, GL_FALSE, _visrange_to_clip.GetData());
    ////glProgramUniformMatrix4fv(_progID_onscr_w, _r2c_unif_onscr_w, 1, GL_FALSE, _visrange_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_m,       _r2c_unif_m,       1, GL_FALSE, _visrange_to_clip.GetData());
    glProgramUniformMatrix4fv(_progID_c,       _r2c_unif_c,       1, GL_FALSE, _visrange_to_clip.GetData());
}

// ===============================================================================

template<typename T>
void Canvas<T>::UpdateSizeLimits(void)
{
#ifdef DEBUG_CALLS
    printf("Canvas::UpdateSizeLimits\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    const int min_w = _margin_xl_pix + MINFRAMEWIDTH + _margin_xr_pix;
    const int min_h = _margin_yb_pix + MINFRAMEHEIGHT + _margin_yt_pix;
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
#ifdef DEBUG_CALLS
    printf("Canvas::FinalizeTextRenderer\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    int win_w; int win_h;
    glfwGetWindowSize(_window, &win_w, &win_h);
    _text_rend.FirstReshape(win_w, win_h);
}
//+++++++++++++++++++++++++++++++++++++++++++++

// ===============================================================================

template<typename T>
void Canvas<T>::SetDarkColorScheme(void)
{
    _grid.SetDarkColorScheme();
    this->SetBackgroundColor(tiny_gl_text_renderer::colors::gray1);
    this->SetInFrameBackgroundColor(tiny_gl_text_renderer::colors::gray05);
    _axes_line_color   = tiny_gl_text_renderer::colors::gray75;
    _vref_line_color   = tiny_gl_text_renderer::colors::olive;
    _frame_line_color  = tiny_gl_text_renderer::colors::gray5;
    _cursor_color      = tiny_gl_text_renderer::colors::white;
    _gen_text_color    = tiny_gl_text_renderer::colors::white;
}

template<typename T>
void Canvas<T>::SetBrightColorScheme(void)
{
    _grid.SetBrightColorScheme();
    this->SetBackgroundColor(tiny_gl_text_renderer::colors::gray9);
    this->SetInFrameBackgroundColor(tiny_gl_text_renderer::colors::gray95);
    _axes_line_color   = tiny_gl_text_renderer::colors::gray25;
    _vref_line_color   = tiny_gl_text_renderer::colors::olive;
    _frame_line_color  = tiny_gl_text_renderer::colors::gray5;
    _cursor_color      = tiny_gl_text_renderer::colors::black;
    _gen_text_color    = tiny_gl_text_renderer::colors::black;
}

template<typename T>
void Canvas<T>::SetBackgroundColor(const color_t& color)
{
#ifdef DEBUG_CALLS
    printf("Canvas::SetBackgroundColor\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    _background_color = color;
    glClearColor(_background_color[0], _background_color[1],
                 _background_color[2], _background_color[3]);
}

template<typename T>
void Canvas<T>::SetInFrameBackgroundColor(const color_t& color)
{
#ifdef DEBUG_CALLS
    printf("Canvas::SetInFrameBackgroundColor\n");
#endif
#ifdef SET_CONTEXT
    glfwMakeContextCurrent(_window);
#endif
    _in_frame_bg_color = color;
    glProgramUniform4fv(_progID_onscr_q, _fr_bg_unif_onscr_q, 1,
        _in_frame_bg_color.GetData());
}

// ===============================================================================

template class Canvas<float>;
template class Canvas<double>;

} // end of namespace tiny_graph_plot
