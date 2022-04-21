#pragma once

#include "tiny_gl_text_renderer/colors.h"
#include "size_info.h"
#include "xy_range.h"

namespace tiny_graph_plot {

using tiny_gl_text_renderer::color_t;

template<typename T>
class Drawable
{
protected:
    Drawable(void) :
        _points(nullptr),
        _size_info(0, 0, 0, 0),
        _xy_range(0.0, 1.0, 0.0, 1.0) {
    }
    virtual ~Drawable(void) = 0 {}
public:
    const Vec2<T>& GetPoint(const size_t idx) const { return _points[idx]; }
    const SizeInfo& GetSizeInfo(void) const { return _size_info; }
    const XYrange<T>& GetXYrange(void) const { return _xy_range; }
protected:
    Vec2<T>* _points;
    SizeInfo _size_info;
    mutable XYrange<T> _xy_range;
public: // visual parameters
    void SetColor(const color_t& color)  { _color = color; }
    void SetMarkerSize(const float size) { _marker_size = size; }
    void SetLineWidth(const float width) { _line_width = width; }
    const color_t& GetColor(void) const   { return _color; }
    const float GetMarkerSize(void) const { return _marker_size; }
    const float GetLineWidth(void) const  { return _line_width; }
private: // visual parameters
    color_t _color = tiny_gl_text_renderer::colors::blue;
    float _marker_size = 5.0f;
    float _line_width = 3.0f;
public: // mutable visual parameters
    void SetVisible(const bool visible) const { _visible = visible; }
    const bool GetVisible(void) const { return _visible; }
private: // mutable visual parameters
    mutable bool _visible = true;
};

template class Drawable<float>;
template class Drawable<double>;

} // end of namespace tiny_graph_plot
