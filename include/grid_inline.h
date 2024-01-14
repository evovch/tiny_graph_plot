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

    unsigned int pix_per_unit_x = static_cast<unsigned int>(fine_step_x_ * vw / dx);
    unsigned int pix_per_unit_y = static_cast<unsigned int>(fine_step_y_ * vh / dy);

    if ((pix_per_unit_x >= cell_size_in_pix_min_) &&
        (pix_per_unit_x <= cell_size_in_pix_max_) &&
        (pix_per_unit_y >= cell_size_in_pix_min_) &&
        (pix_per_unit_y <= cell_size_in_pix_max_)) {
        // Everything is already fine, just quit
        return 0;
    }

    if (pix_per_unit_x < cell_size_in_pix_min_) {
        while (pix_per_unit_x < cell_size_in_pix_min_) {
            fine_step_x_ *= 10.0;
            pix_per_unit_x = static_cast<int>(fine_step_x_ * vw / dx);
        }
    } else {
        while (pix_per_unit_x > cell_size_in_pix_max_) {
            fine_step_x_ /= 10.0;
            pix_per_unit_x = static_cast<int>(fine_step_x_ * vw / dx);
        }
    }

    if (pix_per_unit_y < cell_size_in_pix_min_) {
        while (pix_per_unit_y < cell_size_in_pix_min_) {
            fine_step_y_ *= 10.0;
            pix_per_unit_y = static_cast<int>(fine_step_y_ * vh / dy);
        }
    } else {
        while (pix_per_unit_y > cell_size_in_pix_max_) {
            fine_step_y_ /= 10.0;
            pix_per_unit_y = static_cast<int>(fine_step_y_ * vh / dy);
        }
    }

    coarse_step_x_ = static_cast<T>(coarse_grid_factor_) * fine_step_x_;
    coarse_step_y_ = static_cast<T>(coarse_grid_factor_) * fine_step_y_;

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
    const T   xmin_fine_scaled = xmin / fine_step_x_;
    const int xlow_fine_scaled = static_cast<int>(std::ceil(xmin_fine_scaled));
    const T   ymin_fine_scaled = ymin / fine_step_y_;
    const int ylow_fine_scaled = static_cast<int>(std::ceil(ymin_fine_scaled));
    // Fine - highest
    const T   xmax_fine_scaled = xmax / fine_step_x_;
    const int xhig_fine_scaled = static_cast<int>(std::floor(xmax_fine_scaled));
    const T   ymax_fine_scaled = ymax / fine_step_y_;
    const int yhig_fine_scaled = static_cast<int>(std::floor(ymax_fine_scaled));

    const unsigned int n_grid_lines_x = xhig_fine_scaled - xlow_fine_scaled + 1u;
    const unsigned int n_grid_lines_y = yhig_fine_scaled - ylow_fine_scaled + 1u;
    const unsigned int new_n_vertices = 2u * (n_grid_lines_x + n_grid_lines_y);

    if (n_vertices_ == new_n_vertices) {
        // The grid should not be changed
        //TODO check
        return 1;
    }

    // Vertices --------------------------------------------------------------
    {
        if (vertices_ == nullptr) {
            vertices_ = new vertex_colored_t[new_n_vertices];
        } else {
            // If the size did not change - do not reallocate
            if (n_vertices_ != new_n_vertices) {
                delete[] vertices_;
                vertices_ = new vertex_colored_t[new_n_vertices];
            }
        }
        // Should be done after reallocation because n_vertices_
        // stores the old array size
        n_vertices_ = new_n_vertices;

        const T x_start = static_cast<T>(xlow_fine_scaled) * fine_step_x_;
        const T y_start = static_cast<T>(ylow_fine_scaled) * fine_step_y_;

        unsigned int vertex_offset = 0u;
        for (unsigned int i = 0u; i < n_grid_lines_x; i++) {
            vertices_[vertex_offset + i * 2 + 0].coords_ =
                point_t(
                    static_cast<float>(x_start + i * fine_step_x_),
                    static_cast<float>(totalrange.lowy()),
                    0.0f, 1.0f);
            vertices_[vertex_offset + i * 2 + 1].coords_ =
                point_t(
                    static_cast<float>(x_start + i * fine_step_x_),
                    static_cast<float>(totalrange.highy()),
                    0.0f, 1.0f);
            vertices_[vertex_offset + i * 2 + 0].color_ = vgrid_fine_line_color_;
            vertices_[vertex_offset + i * 2 + 1].color_ = vgrid_fine_line_color_;
        }
        vertex_offset = 2 * n_grid_lines_x;
        for (unsigned int i = 0u; i < n_grid_lines_y; i++) {
            vertices_[vertex_offset + i * 2 + 0].coords_ =
                point_t(
                    static_cast<float>(totalrange.lowx()),
                    static_cast<float>(y_start + i * fine_step_y_),
                    0.0f, 1.0f);
            vertices_[vertex_offset + i * 2 + 1].coords_ =
                point_t(
                    static_cast<float>(totalrange.highx()),
                    static_cast<float>(y_start + i * fine_step_y_),
                    0.0f, 1.0f);
            vertices_[vertex_offset + i * 2 + 0].color_ = hgrid_fine_line_color_;
            vertices_[vertex_offset + i * 2 + 1].color_ = hgrid_fine_line_color_;
        }
    }
    // Wires fine ------------------------------------------------------------
    {
        n_wires_fine_x_ = n_grid_lines_x;
        n_wires_fine_y_ = n_grid_lines_y;
        if (wires_fine_ == nullptr) {
            wires_fine_ = new wire_t[n_wires_fine_x_ + n_wires_fine_y_];
        } else {
            // If the size did not change - do not reallocate
            if (n_wires_fine_ != n_wires_fine_x_ + n_wires_fine_y_) {
                delete[] wires_fine_;
                wires_fine_ = new wire_t[n_wires_fine_x_ + n_wires_fine_y_];
            }
        }
        // Should be done after reallocation because n_wires_fine_
        // stores the old array size
        n_wires_fine_ = n_wires_fine_x_ + n_wires_fine_y_;

        for (unsigned int i = 0u; i < n_wires_fine_x_ + n_wires_fine_y_; i++) {
            wires_fine_[i].v0 = 2 * i + 0;
            wires_fine_[i].v1 = 2 * i + 1;
        }
    }

    // Wires coarse ----------------------------------------------------------
    {
        // Coarse - lowest
        const T   xmin_coarse_scaled = xmin / coarse_step_x_;
        const int xlow_coarse_scaled = static_cast<int>(std::ceil(xmin_coarse_scaled));
        const T   ymin_coarse_scaled = ymin / coarse_step_y_;
        const int ylow_coarse_scaled = static_cast<int>(std::ceil(ymin_coarse_scaled));
        // Coarse - highest
        const T   xmax_coarse_scaled = xmax / coarse_step_x_;
        const int xhig_coarse_scaled = static_cast<int>(std::floor(xmax_coarse_scaled));
        const T   ymax_coarse_scaled = ymax / coarse_step_y_;
        const int yhig_coarse_scaled = static_cast<int>(std::floor(ymax_coarse_scaled));

        const int offset_x = xlow_coarse_scaled * coarse_grid_factor_ - xlow_fine_scaled;
        const int offset_y = ylow_coarse_scaled * coarse_grid_factor_ - ylow_fine_scaled;

        n_wires_coarse_x_ = xhig_coarse_scaled - xlow_coarse_scaled + 1;
        n_wires_coarse_y_ = yhig_coarse_scaled - ylow_coarse_scaled + 1;
        if (wires_coarse_ == nullptr) {
            wires_coarse_ = new wire_t[n_wires_coarse_x_ + n_wires_coarse_y_];
        } else {
            // If the size did not change - do not reallocate
            if (n_wires_coarse_ != n_wires_coarse_x_ + n_wires_coarse_y_) {
                delete[] wires_coarse_;
                wires_coarse_ = new wire_t[n_wires_coarse_x_ + n_wires_coarse_y_];
            }
        }
        // Should be done after reallocation because n_wires_coarse_
        // stores the old array size
        n_wires_coarse_ = n_wires_coarse_x_ + n_wires_coarse_y_;

        unsigned int vertex_offset = 0u;
        for (unsigned int i = 0u; i < n_wires_coarse_x_; i++) {
            wires_coarse_[i].v0 = vertex_offset + 2 * offset_x + 2 * i * coarse_grid_factor_ + 0;
            wires_coarse_[i].v1 = vertex_offset + 2 * offset_x + 2 * i * coarse_grid_factor_ + 1;
        }
        vertex_offset = 2 * n_grid_lines_x;
        for (unsigned int i = 0u; i < n_wires_coarse_y_; i++) {
            wires_coarse_[n_wires_coarse_x_ + i].v0 = vertex_offset + 2 * offset_y + 2 * i * coarse_grid_factor_ + 0;
            wires_coarse_[n_wires_coarse_x_ + i].v1 = vertex_offset + 2 * offset_y + 2 * i * coarse_grid_factor_ + 1;
        }
    }
    return 0;
}

} // end of namespace tiny_graph_plot
