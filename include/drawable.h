#pragma once

//#include <cassert> //TODO
#include "tiny_gl_text_renderer/colors.h"
#include "size_info.h"
#include "xy_range.h"

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::color_t;

template<typename T>
class Drawable
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
protected:
    explicit Drawable() noexcept
    :   points_(nullptr),
        size_info_(0u, 0u, 0u, 0u),
        xy_range_(T(0.0), T(1.0), T(0.0), T(1.0)) {
    }
    virtual ~Drawable() {}
    Drawable(const Drawable& other) = delete;
    Drawable(Drawable&& other) = delete;
    Drawable& operator=(const Drawable& other) = delete;
    Drawable& operator=(Drawable&& other) = delete;
public:
    const Vec2<T>& GetPoint(const size_t i) const {
        //assert(i < ); //TODO
        return points_[i];
    }
    const SizeInfo& GetSizeInfo() const noexcept { return size_info_; }
    const XYrange<T>& GetXYrange() const noexcept { return xy_range_; }
protected:
    Vec2<T>* points_;
    SizeInfo size_info_;
    mutable XYrange<T> xy_range_;
public: // visual parameters
    void SetColor(const color_t& color) noexcept  { color_ = color; }
    void SetMarkerSize(const float size) noexcept { marker_size_ = size; }
    void SetLineWidth(const float width) noexcept { line_width_ = width; }
    const color_t& GetColor() const noexcept   { return color_; }
    const float GetMarkerSize() const noexcept { return marker_size_; }
    const float GetLineWidth() const noexcept  { return line_width_; }
private: // visual parameters
    color_t color_ = tiny_gl_text_renderer::colors::blue;
    float marker_size_ = 5.0f;
    float line_width_ = 3.0f;
public: // mutable visual parameters
    void SetVisible(const bool visible) const noexcept { visible_ = visible; }
    const bool GetVisible() const noexcept { return visible_; }
private: // mutable visual parameters
    mutable bool visible_ = true;
};

template class Drawable<float>;
template class Drawable<double>;

} // end of namespace tiny_graph_plot
