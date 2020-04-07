#pragma once

#include "tiny_gl_text_renderer/data_types.h"
#include "xy_range.h"

#define GRID_PRECISION 1.0e-10

namespace tiny_graph_plot {

using tiny_gl_text_renderer::point_t;
using tiny_gl_text_renderer::color_t;
using tiny_gl_text_renderer::vertex_colored_t;
using tiny_gl_text_renderer::wire_t;

template<typename T>
class Grid
{
public:
    Grid(void) :
        _fine_step_x(static_cast<T>(0.2)),
        _fine_step_y(static_cast<T>(0.2)),
        _coarse_step_x(static_cast<T>(1.0)),
        _coarse_step_y(static_cast<T>(1.0))
    {}
    ~Grid(void) {
        if (_vertices != nullptr)     { delete[] _vertices; }
        if (_wires_fine != nullptr)   { delete[] _wires_fine; }
        if (_wires_coarse != nullptr) { delete[] _wires_coarse; }
    }
public:
    int CalculateStep(XYrange<T> visrange, const T vw, const T vh) {
        const T dx = std::fabs(visrange.dx());
        const T dy = std::fabs(visrange.dy());

        // Check that the input visible range is not degenerate
        if (dx < GRID_PRECISION || dy < GRID_PRECISION ||
            !std::isfinite(dx)  || !std::isfinite(dy)) {
            return 1;
        }

        unsigned int pix_per_unit_x = static_cast<unsigned int>(_fine_step_x * vw / dx);
        unsigned int pix_per_unit_y = static_cast<unsigned int>(_fine_step_y * vh / dy);

        if ((pix_per_unit_x >= _cell_size_in_pix_min) &&
            (pix_per_unit_x <= _cell_size_in_pix_max) &&
            (pix_per_unit_y >= _cell_size_in_pix_min) &&
            (pix_per_unit_y <= _cell_size_in_pix_max)) {
            // Everything is already fine, just quit
            return 0;
        }

        if (pix_per_unit_x < _cell_size_in_pix_min) {
            while (pix_per_unit_x < _cell_size_in_pix_min) {
                _fine_step_x *= 10.0;
                pix_per_unit_x = static_cast<int>(_fine_step_x * vw / dx);
            }
        } else {
            while (pix_per_unit_x > _cell_size_in_pix_max) {
                _fine_step_x /= 10.0;
                pix_per_unit_x = static_cast<int>(_fine_step_x * vw / dx);
            }
        }

        if (pix_per_unit_y < _cell_size_in_pix_min) {
            while (pix_per_unit_y < _cell_size_in_pix_min) {
                _fine_step_y *= 10.0;
                pix_per_unit_y = static_cast<int>(_fine_step_y * vh / dy);
            }
        } else {
            while (pix_per_unit_y > _cell_size_in_pix_max) {
                _fine_step_y /= 10.0;
                pix_per_unit_y = static_cast<int>(_fine_step_y * vh / dy);
            }
        }

        _coarse_step_x = static_cast<T>(_coarse_grid_factor) * _fine_step_x;
        _coarse_step_y = static_cast<T>(_coarse_grid_factor) * _fine_step_y;

        return 0;
    }
    int BuildGrid(XYrange<T> visrange, XYrange<T> totalrange) {
        const T& xmin = std::fmax(visrange.lowx(),  totalrange.lowx());
        const T& xmax = std::fmin(visrange.highx(), totalrange.highx());
        const T& ymin = std::fmax(visrange.lowy(),  totalrange.lowy());
        const T& ymax = std::fmin(visrange.highy(), totalrange.highy());

        // Looking at some region outside of the graphs area. Nothing to draw.
        if (xmin >= xmax || ymin >= ymax) return 1;

        // Fine - lowest
        const T   xmin_fine_scaled = xmin / _fine_step_x;
        const int xlow_fine_scaled = static_cast<int>(std::ceil(xmin_fine_scaled));
        const T   ymin_fine_scaled = ymin / _fine_step_y;
        const int ylow_fine_scaled = static_cast<int>(std::ceil(ymin_fine_scaled));
        // Fine - highest
        const T   xmax_fine_scaled = xmax / _fine_step_x;
        const int xhig_fine_scaled = static_cast<int>(std::floor(xmax_fine_scaled));
        const T   ymax_fine_scaled = ymax / _fine_step_y;
        const int yhig_fine_scaled = static_cast<int>(std::floor(ymax_fine_scaled));

        const unsigned int n_grid_lines_x = xhig_fine_scaled - xlow_fine_scaled + 1;
        const unsigned int n_grid_lines_y = yhig_fine_scaled - ylow_fine_scaled + 1;

        if (_n_vertices == 2 * (n_grid_lines_x + n_grid_lines_y)) {
            // The grid should not be changed
            //TODO check
            return 1;
        }

        // Vertices --------------------------------------------------------------
        {
            if (_vertices == nullptr) {
                _vertices = new vertex_colored_t[2 * (n_grid_lines_x + n_grid_lines_y)];
            } else {
                // If the size did not change - do not reallocate
                if (_n_vertices != 2 * (n_grid_lines_x + n_grid_lines_y)) {
                    delete[] _vertices;
                    _vertices = new vertex_colored_t[2 * (n_grid_lines_x + n_grid_lines_y)];
                }
            }
            // Should be done after reallocation because _n_vertices
            // stores the old array size
            _n_vertices = 2 * (n_grid_lines_x + n_grid_lines_y);

            const T x_start = static_cast<T>(xlow_fine_scaled) * _fine_step_x;
            const T y_start = static_cast<T>(ylow_fine_scaled) * _fine_step_y;

            unsigned int vertex_offset = 0;
            for (unsigned int i = 0; i < n_grid_lines_x; i++) {
                _vertices[vertex_offset + i * 2 + 0]._coords =
                    point_t(
                        static_cast<float>(x_start + i * _fine_step_x),
                        static_cast<float>(totalrange.lowy()),
                        0.0f, 1.0f);
                _vertices[vertex_offset + i * 2 + 1]._coords =
                    point_t(
                        static_cast<float>(x_start + i * _fine_step_x),
                        static_cast<float>(totalrange.highy()),
                        0.0f, 1.0f);
                _vertices[vertex_offset + i * 2 + 0]._color = _vgrid_fine_line_color;
                _vertices[vertex_offset + i * 2 + 1]._color = _vgrid_fine_line_color;
            }
            vertex_offset = 2 * n_grid_lines_x;
            for (unsigned int i = 0; i < n_grid_lines_y; i++) {
                _vertices[vertex_offset + i * 2 + 0]._coords =
                    point_t(
                        static_cast<float>(totalrange.lowx()),
                        static_cast<float>(y_start + i * _fine_step_y),
                        0.0f, 1.0f);
                _vertices[vertex_offset + i * 2 + 1]._coords =
                    point_t(
                        static_cast<float>(totalrange.highx()),
                        static_cast<float>(y_start + i * _fine_step_y),
                        0.0f, 1.0f);
                _vertices[vertex_offset + i * 2 + 0]._color = _hgrid_fine_line_color;
                _vertices[vertex_offset + i * 2 + 1]._color = _hgrid_fine_line_color;
            }
        }
        // Wires fine ------------------------------------------------------------
        {
            _n_wires_fine_x = n_grid_lines_x;
            _n_wires_fine_y = n_grid_lines_y;
            if (_wires_fine == nullptr) {
                _wires_fine = new wire_t[_n_wires_fine_x + _n_wires_fine_y];
            } else {
                // If the size did not change - do not reallocate
                if (_n_wires_fine != _n_wires_fine_x + _n_wires_fine_y) {
                    delete[] _wires_fine;
                    _wires_fine = new wire_t[_n_wires_fine_x + _n_wires_fine_y];
                }
            }
            // Should be done after reallocation because _n_wires_fine
            // stores the old array size
            _n_wires_fine = _n_wires_fine_x + _n_wires_fine_y;

            for (unsigned int i = 0; i < _n_wires_fine_x + _n_wires_fine_y; i++) {
                _wires_fine[i].v0 = 2 * i + 0;
                _wires_fine[i].v1 = 2 * i + 1;
            }
        }

        // Wires coarse ----------------------------------------------------------
        {
            // Coarse - lowest
            const T   xmin_coarse_scaled = xmin / _coarse_step_x;
            const int xlow_coarse_scaled = static_cast<int>(std::ceil(xmin_coarse_scaled));
            const T   ymin_coarse_scaled = ymin / _coarse_step_y;
            const int ylow_coarse_scaled = static_cast<int>(std::ceil(ymin_coarse_scaled));
            // Coarse - highest
            const T   xmax_coarse_scaled = xmax / _coarse_step_x;
            const int xhig_coarse_scaled = static_cast<int>(std::floor(xmax_coarse_scaled));
            const T   ymax_coarse_scaled = ymax / _coarse_step_y;
            const int yhig_coarse_scaled = static_cast<int>(std::floor(ymax_coarse_scaled));

            const int offset_x = xlow_coarse_scaled * _coarse_grid_factor - xlow_fine_scaled;
            const int offset_y = ylow_coarse_scaled * _coarse_grid_factor - ylow_fine_scaled;

            _n_wires_coarse_x = xhig_coarse_scaled - xlow_coarse_scaled + 1;
            _n_wires_coarse_y = yhig_coarse_scaled - ylow_coarse_scaled + 1;
            if (_wires_coarse == nullptr) {
                _wires_coarse = new wire_t[_n_wires_coarse_x + _n_wires_coarse_y];
            } else {
                // If the size did not change - do not reallocate
                if (_n_wires_coarse != _n_wires_coarse_x + _n_wires_coarse_y) {
                    delete[] _wires_coarse;
                    _wires_coarse = new wire_t[_n_wires_coarse_x + _n_wires_coarse_y];
                }
            }
            // Should be done after reallocation because _n_wires_coarse
            // stores the old array size
            _n_wires_coarse = _n_wires_coarse_x + _n_wires_coarse_y;

            unsigned int vertex_offset = 0;
            for (unsigned int i = 0; i < _n_wires_coarse_x; i++) {
                _wires_coarse[i].v0 = vertex_offset + 2 * offset_x + 2 * i * _coarse_grid_factor + 0;
                _wires_coarse[i].v1 = vertex_offset + 2 * offset_x + 2 * i * _coarse_grid_factor + 1;
            }
            vertex_offset = 2 * n_grid_lines_x;
            for (unsigned int i = 0; i < _n_wires_coarse_y; i++) {
                _wires_coarse[_n_wires_coarse_x + i].v0 = vertex_offset + 2 * offset_y + 2 * i * _coarse_grid_factor + 0;
                _wires_coarse[_n_wires_coarse_x + i].v1 = vertex_offset + 2 * offset_y + 2 * i * _coarse_grid_factor + 1;
            }
        }
        return 0;
    }
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
    const T& GetFineXstep(void) const { return _fine_step_x; }
    const T& GetFineYstep(void) const { return _fine_step_y; }
    const T& GetCoarseXstep(void) const { return _coarse_step_x; }
    const T& GetCoarseYstep(void) const { return _coarse_step_y; }
private:
    const unsigned int _coarse_grid_factor = 5;
    const unsigned int _cell_size_in_pix_min = 20;
    const unsigned int _cell_size_in_pix_max = 200;
    const unsigned int _axis_value_step_in_pix_min = 100;
    const unsigned int _axis_value_step_in_pix_max = 500;
    T _fine_step_x;
    T _fine_step_y;
    T _coarse_step_x; //!< _fine_step_x * _coarse_grid_factor
    T _coarse_step_y; //!< _fine_step_y * _coarse_grid_factor
    unsigned int _n_vertices = 0;
    unsigned int _n_wires_fine_x = 0; //!< Number of vertical wires of the fine grid along X direction
    unsigned int _n_wires_fine_y = 0; //!< Number of horizontal wires of the fine grid along Y direction
    unsigned int _n_wires_coarse_x = 0; //!< Number of vertical wires of the coarse grid along X direction
    unsigned int _n_wires_coarse_y = 0; //!< Number of horizontal wires of the coarse grid along Y direction
    unsigned int _n_wires_fine = 0;   //!< _n_wires_fine_x + _n_wires_fine_y
    unsigned int _n_wires_coarse = 0; //!< _n_wires_coarse_x + _n_wires_coarse_y
    vertex_colored_t* _vertices = nullptr;
    wire_t* _wires_fine = nullptr;
    wire_t* _wires_coarse = nullptr;
public: // visual parameters
    void SetDarkColorScheme(void) {
        _hgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray6;
        _vgrid_fine_line_color   = tiny_gl_text_renderer::colors::gray6;
        _hgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray5;
        _vgrid_coarse_line_color = tiny_gl_text_renderer::colors::gray5;
    }
    void SetBrightColorScheme(void) {
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
    const color_t& GetHGridFineColor(void)   const { return _hgrid_fine_line_color; }
    const color_t& GetVGridFineColor(void)   const { return _vgrid_fine_line_color; }
    const color_t& GetHGridCoarseColor(void) const { return _hgrid_coarse_line_color; }
    const color_t& GetVGridCoarseColor(void) const { return _vgrid_coarse_line_color; }
    float GetHGridFineLineWidth(void)   const { return _hgrid_fine_line_width; }
    float GetVGridFineLineWidth(void)   const { return _vgrid_fine_line_width; }
    float GetHGridCoarseLineWidth(void) const { return _hgrid_coarse_line_width; }
    float GetVGridCoarseLineWidth(void) const { return _vgrid_coarse_line_width; }
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
