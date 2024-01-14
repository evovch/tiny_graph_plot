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
        if (vertices_ != nullptr)     { delete[] vertices_; }
        if (wires_fine_ != nullptr)   { delete[] wires_fine_; }
        if (wires_coarse_ != nullptr) { delete[] wires_coarse_; }
    }
    Grid(const Grid& other) = delete;
    Grid(Grid&& other) = delete;
    Grid& operator=(const Grid& other) = delete;
    Grid& operator=(Grid&& other) = delete;
public:
    int CalculateStep(XYrange<T> visrange, const T vw, const T vh);
    int BuildGrid(XYrange<T> visrange, XYrange<T> totalrange);
    const vertex_colored_t* GetVerticesData(unsigned int& o_n) const noexcept {
        o_n = _n_vertices;
        return vertices_;
    }
    const wire_t* GetWiresFineData(unsigned int& o_n_x, unsigned int& o_n_y) const noexcept {
        o_n_x = _n_wires_fine_x;
        o_n_y = _n_wires_fine_y;
        return wires_fine_;
    }
    const wire_t* GetWiresCoarseData(unsigned int& o_n_x, unsigned int& o_n_y) const noexcept {
        o_n_x = _n_wires_coarse_x;
        o_n_y = _n_wires_coarse_y;
        return wires_coarse_;
    }
    T GetFineXstep() const   noexcept { return fine_step_x_; }
    T GetFineYstep() const   noexcept { return fine_step_y_; }
    T GetCoarseXstep() const noexcept { return coarse_step_x_; }
    T GetCoarseYstep() const noexcept { return coarse_step_y_; }
private:
    static constexpr unsigned int coarse_grid_factor_ = 5u;
    static constexpr unsigned int cell_size_in_pix_min_ = 20u;
    static constexpr unsigned int cell_size_in_pix_max_ = 200u;
    static constexpr unsigned int axis_value_step_in_pix_min_ = 100u;
    static constexpr unsigned int axis_value_step_in_pix_max_ = 500u;
    T fine_step_x_ = T(0.2);
    T fine_step_y_ = T(0.2);
    T coarse_step_x_ = T(1.0); //!< fine_step_x_ * coarse_grid_factor_
    T coarse_step_y_ = T(1.0); //!< fine_step_y_ * coarse_grid_factor_
    unsigned int _n_vertices = 0u;
    unsigned int _n_wires_fine_x = 0u; //!< Number of vertical wires of the fine grid along X direction
    unsigned int _n_wires_fine_y = 0u; //!< Number of horizontal wires of the fine grid along Y direction
    unsigned int _n_wires_coarse_x = 0u; //!< Number of vertical wires of the coarse grid along X direction
    unsigned int _n_wires_coarse_y = 0u; //!< Number of horizontal wires of the coarse grid along Y direction
    unsigned int _n_wires_fine = 0u;   //!< _n_wires_fine_x + _n_wires_fine_y
    unsigned int _n_wires_coarse = 0u; //!< _n_wires_coarse_x + _n_wires_coarse_y
    vertex_colored_t* vertices_ = nullptr;
    wire_t* wires_fine_ = nullptr;
    wire_t* wires_coarse_ = nullptr;
public: // visual parameters
    void SetDarkColorScheme() noexcept {
        hgrid_fine_line_color_   = tiny_gl_text_renderer::colors::gray6;
        vgrid_fine_line_color_   = tiny_gl_text_renderer::colors::gray6;
        hgrid_coarse_line_color_ = tiny_gl_text_renderer::colors::gray5;
        vgrid_coarse_line_color_ = tiny_gl_text_renderer::colors::gray5;
    }
    void SetBrightColorScheme() noexcept {
        hgrid_fine_line_color_   = tiny_gl_text_renderer::colors::gray3;
        vgrid_fine_line_color_   = tiny_gl_text_renderer::colors::gray3;
        hgrid_coarse_line_color_ = tiny_gl_text_renderer::colors::gray2;
        vgrid_coarse_line_color_ = tiny_gl_text_renderer::colors::gray2;
    }
    void SetHGridFineColor(const color_t& color)    noexcept { hgrid_fine_line_color_ = color; }
    void SetVGridFineColor(const color_t& color)    noexcept { vgrid_fine_line_color_ = color; }
    void SetHGridCoarseColor(const color_t& color)  noexcept { hgrid_coarse_line_color_ = color; }
    void SetVGridCoarseColor(const color_t& color)  noexcept { vgrid_coarse_line_color_ = color; }
    void SetHGridFineLineWidth(const float width)   noexcept { hgrid_fine_line_width_ = width; }
    void SetVGridFineLineWidth(const float width)   noexcept { vgrid_fine_line_width_ = width; }
    void SetHGridCoarseLineWidth(const float width) noexcept { hgrid_coarse_line_width_ = width; }
    void SetVGridCoarseLineWidth(const float width) noexcept { vgrid_coarse_line_width_ = width; }
    color_t GetHGridFineColor()     const noexcept { return hgrid_fine_line_color_; }
    color_t GetVGridFineColor()     const noexcept { return vgrid_fine_line_color_; }
    color_t GetHGridCoarseColor()   const noexcept { return hgrid_coarse_line_color_; }
    color_t GetVGridCoarseColor()   const noexcept { return vgrid_coarse_line_color_; }
    float GetHGridFineLineWidth()   const noexcept { return hgrid_fine_line_width_; }
    float GetVGridFineLineWidth()   const noexcept { return vgrid_fine_line_width_; }
    float GetHGridCoarseLineWidth() const noexcept { return hgrid_coarse_line_width_; }
    float GetVGridCoarseLineWidth() const noexcept { return vgrid_coarse_line_width_; }
private: // visual parameters
    color_t hgrid_fine_line_color_   = tiny_gl_text_renderer::colors::gray6;
    color_t vgrid_fine_line_color_   = tiny_gl_text_renderer::colors::gray6;
    color_t hgrid_coarse_line_color_ = tiny_gl_text_renderer::colors::gray5;
    color_t vgrid_coarse_line_color_ = tiny_gl_text_renderer::colors::gray5;
    float hgrid_fine_line_width_   = 1.0f;
    float vgrid_fine_line_width_   = 1.0f;
    float hgrid_coarse_line_width_ = 1.0f;
    float vgrid_coarse_line_width_ = 1.0f;
};

template class Grid<float>;
template class Grid<double>;

} // end of namespace tiny_graph_plot

#include "grid_inline.h"
