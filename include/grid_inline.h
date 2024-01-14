#pragma once

namespace tiny_graph_plot
{

template<typename T>
inline int Grid<T>::CalculateStep(XYrange<T> visrange, const T vw, const T vh)
{
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

template<typename T>
inline int Grid<T>::BuildGrid(XYrange<T> visrange, XYrange<T> totalrange)
{
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
            _vertices[vertex_offset + i * 2 + 0].coords_ =
                point_t(
                    static_cast<float>(x_start + i * _fine_step_x),
                    static_cast<float>(totalrange.lowy()),
                    0.0f, 1.0f);
            _vertices[vertex_offset + i * 2 + 1].coords_ =
                point_t(
                    static_cast<float>(x_start + i * _fine_step_x),
                    static_cast<float>(totalrange.highy()),
                    0.0f, 1.0f);
            _vertices[vertex_offset + i * 2 + 0].color_ = _vgrid_fine_line_color;
            _vertices[vertex_offset + i * 2 + 1].color_ = _vgrid_fine_line_color;
        }
        vertex_offset = 2 * n_grid_lines_x;
        for (unsigned int i = 0; i < n_grid_lines_y; i++) {
            _vertices[vertex_offset + i * 2 + 0].coords_ =
                point_t(
                    static_cast<float>(totalrange.lowx()),
                    static_cast<float>(y_start + i * _fine_step_y),
                    0.0f, 1.0f);
            _vertices[vertex_offset + i * 2 + 1].coords_ =
                point_t(
                    static_cast<float>(totalrange.highx()),
                    static_cast<float>(y_start + i * _fine_step_y),
                    0.0f, 1.0f);
            _vertices[vertex_offset + i * 2 + 0].color_ = _hgrid_fine_line_color;
            _vertices[vertex_offset + i * 2 + 1].color_ = _hgrid_fine_line_color;
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

} // end of namespace tiny_graph_plot
