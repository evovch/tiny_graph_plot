#pragma once

#include "tiny_gl_text_renderer/data_types.h"
#include "xy_range.h"

#define GRID_PRECISION 1.0e-10

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::point_t;
using tiny_gl_text_renderer::color_t;
using tiny_gl_text_renderer::vertex_colored_t;
using tiny_gl_text_renderer::wire_t;

template<typename T>
class Grid
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    explicit Grid() = default;
    ~Grid() {
        if (_vertices != nullptr)     { delete[] _vertices; }
        if (_wires_fine != nullptr)   { delete[] _wires_fine; }
        if (_wires_coarse != nullptr) { delete[] _wires_coarse; }
    }
    Grid(const Grid& other) = delete;
    Grid(Grid&& other) = delete;
    Grid& operator=(const Grid& other) = delete;
    Grid& operator=(Grid&& other) = delete;
public:
    int CalculateStep(XYrange<T> visrange, const T vw, const T vh);
    int BuildGrid(XYrange<T> visrange, XYrange<T> totalrange);
    const vertex_colored_t* const GetVerticesData(unsigned int& o_n) const {
        o_n = _n_vertices;
        return _vertices;
    }
    const wire_t* const GetWiresFineData(unsigned int& o_n_x,
        unsigned int& o_n_y) const {
        o_n_x = _n_wires_fine_x;
        o_n_y = _n_wires_fine_y;
        return _wires_fine;
    }
    const wire_t* const GetWiresCoarseData(unsigned int& o_n_x,
        unsigned int& o_n_y) const {
        o_n_x = _n_wires_coarse_x;
        o_n_y = _n_wires_coarse_y;
        return _wires_coarse;
    }
    const T& GetFineXstep() const { return _fine_step_x; }
    const T& GetFineYstep() const { return _fine_step_y; }
    const T& GetCoarseXstep() const { return _coarse_step_x; }
    const T& GetCoarseYstep() const { return _coarse_step_y; }
private:
    static constexpr unsigned int _coarse_grid_factor = 5u;
    static constexpr unsigned int _cell_size_in_pix_min = 20u;
    static constexpr unsigned int _cell_size_in_pix_max = 200u;
    static constexpr unsigned int _axis_value_step_in_pix_min = 100u;
    static constexpr unsigned int _axis_value_step_in_pix_max = 500u;
    T _fine_step_x = T(0.2);
    T _fine_step_y = T(0.2);
    T _coarse_step_x = T(1.0); //!< _fine_step_x * _coarse_grid_factor
    T _coarse_step_y = T(1.0); //!< _fine_step_y * _coarse_grid_factor
    unsigned int _n_vertices = 0u;
    unsigned int _n_wires_fine_x = 0u; //!< Number of vertical wires of the fine grid along X direction
    unsigned int _n_wires_fine_y = 0u; //!< Number of horizontal wires of the fine grid along Y direction
    unsigned int _n_wires_coarse_x = 0u; //!< Number of vertical wires of the coarse grid along X direction
    unsigned int _n_wires_coarse_y = 0u; //!< Number of horizontal wires of the coarse grid along Y direction
    unsigned int _n_wires_fine = 0u;   //!< _n_wires_fine_x + _n_wires_fine_y
    unsigned int _n_wires_coarse = 0u; //!< _n_wires_coarse_x + _n_wires_coarse_y
    vertex_colored_t* _vertices = nullptr;
    wire_t* _wires_fine = nullptr;
    wire_t* _wires_coarse = nullptr;
public: // visual parameters
    void SetDarkColorScheme() {
        _hgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray6;
        _vgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray6;
        _hgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray5;
        _vgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray5;
    }
    void SetBrightColorScheme() {
        _hgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray3;
        _vgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray3;
        _hgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray2;
        _vgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray2;
    }
    void SetHGridFineColor(const color_t& color)    { _hgrid_fine_line_color = color; }
    void SetVGridFineColor(const color_t& color)    { _vgrid_fine_line_color = color; }
    void SetHGridCoarseColor(const color_t& color)  { _hgrid_coarse_line_color = color; }
    void SetVGridCoarseColor(const color_t& color)  { _vgrid_coarse_line_color = color; }
    void SetHGridFineLineWidth(const float width)   { _hgrid_fine_line_width = width; }
    void SetVGridFineLineWidth(const float width)   { _vgrid_fine_line_width = width; }
    void SetHGridCoarseLineWidth(const float width) { _hgrid_coarse_line_width = width; }
    void SetVGridCoarseLineWidth(const float width) { _vgrid_coarse_line_width = width; }
    const color_t& GetHGridFineColor()   const { return _hgrid_fine_line_color; }
    const color_t& GetVGridFineColor()   const { return _vgrid_fine_line_color; }
    const color_t& GetHGridCoarseColor() const { return _hgrid_coarse_line_color; }
    const color_t& GetVGridCoarseColor() const { return _vgrid_coarse_line_color; }
    float GetHGridFineLineWidth()   const { return _hgrid_fine_line_width; }
    float GetVGridFineLineWidth()   const { return _vgrid_fine_line_width; }
    float GetHGridCoarseLineWidth() const { return _hgrid_coarse_line_width; }
    float GetVGridCoarseLineWidth() const { return _vgrid_coarse_line_width; }
private: // visual parameters
    color_t _hgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray6;
    color_t _vgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray6;
    color_t _hgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray5;
    color_t _vgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray5;
    float _hgrid_fine_line_width   = 1.0f;
    float _vgrid_fine_line_width   = 1.0f;
    float _hgrid_coarse_line_width = 1.0f;
    float _vgrid_coarse_line_width = 1.0f;
};

template class Grid<float>;
template class Grid<double>;

} // end of namespace tiny_graph_plot

#include "grid_inline.h"
