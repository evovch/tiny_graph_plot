#pragma once

#include <cmath>

#include "tiny_gl_text_renderer/vec.h"

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::Vec2;

template<typename T>
class XYrange
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    explicit XYrange() = default;
    explicit XYrange(const T x_min, const T dx, const T y_min, const T dy) noexcept
    :   _x_min(x_min), _dx(dx), _y_min(y_min), _dy(dy) {}
    ~XYrange() = default;
    XYrange(const XYrange& other) = default;
    XYrange(XYrange&& other) = default;
    XYrange& operator=(const XYrange& other) = default;
    XYrange& operator=(XYrange&& other) = default;
public:
    template<typename U>
    XYrange& operator=(const XYrange<U>& rhs) {
        _x_min = static_cast<T>(rhs.lowx());
        _dx    = static_cast<T>(rhs.dx());
        _y_min = static_cast<T>(rhs.lowy());
        _dy    = static_cast<T>(rhs.dy());
        return *this;
    }
    template<typename U>
    void Include(const XYrange<U>& other) {
        const T x_max = std::fmax(_x_min + _dx, static_cast<T>(other.lowx() + other.dx()));
        const T y_max = std::fmax(_y_min + _dy, static_cast<T>(other.lowy() + other.dy()));
        _x_min = std::fmin(_x_min, static_cast<T>(other.lowx()));
        _y_min = std::fmin(_y_min, static_cast<T>(other.lowy()));
        _dx = x_max - _x_min;
        _dy = y_max - _y_min;
    }
    void Include(const Vec2<T>& point) {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y())) return;
        const T x_max = std::fmax(_x_min + _dx, point.x());
        const T y_max = std::fmax(_y_min + _dy, point.y());
        _x_min = std::fmin(_x_min, point.x());
        _y_min = std::fmin(_y_min, point.y());
        _dx = x_max - _x_min;
        _dy = y_max - _y_min;

    }
    bool IncludesX(const T x) const noexcept {
        return (x >= _x_min) && (x <= _x_min + _dx);
    }
    void FixDegenerateCases() noexcept {
        if (_dx <= 1.0e-8) { // abs?
            _x_min -= static_cast<T>(0.5);
            _dx     = static_cast<T>(1.0);
        }
        if (_dy <= 1.0e-8) { // abs?
            _y_min -= static_cast<T>(0.5);
            _dy     = static_cast<T>(1.0);
        }
    }
    void Set1(const T x_min, const T x_max, const T y_min, const T y_max) noexcept {
        _x_min = x_min; _dx = x_max - x_min; _y_min = y_min; _dy = y_max - y_min;
    }
    void Set2(const T x_min, const T dx, const T y_min, const T dy) noexcept {
        _x_min = x_min; _dx = dx; _y_min = y_min; _dy = dy;
    }
    void SetXrange1(const T x_min, const T x_max) noexcept { _x_min = x_min; _dx = x_max - x_min; }
    void SetYrange1(const T y_min, const T y_max) noexcept { _y_min = y_min; _dy = y_max - y_min; }
    void SetXrange2(const T x_min, const T dx) noexcept { _x_min = x_min; _dx = dx; }
    void SetYrange2(const T y_min, const T dy) noexcept { _y_min = y_min; _dy = dy; }
    void MoveX(const T shift) noexcept { _x_min += shift; }
    void MoveY(const T shift) noexcept { _y_min += shift; }
    const T& lowx() const noexcept { return _x_min; }
    const T highx() const noexcept { return _x_min + _dx; }
    const T& lowy() const noexcept { return _y_min; }
    const T highy() const noexcept { return _y_min + _dy; }
    const T& dx() const noexcept   { return _dx; }
    const T& dy() const noexcept   { return _dy; }
    T xm() const noexcept    { return _x_min + T(0.5) * _dx; }
    T ym() const noexcept    { return _y_min + T(0.5) * _dy; }
    T twoxm() const noexcept { return T(2.0) * _x_min + _dx; }
    T twoym() const noexcept { return T(2.0) * _y_min + _dy; }
private:
    T _x_min = T(0.0);
    T _dx = T(1.0);
    T _y_min = T(0.0);
    T _dy = T(1.0);
};

template class XYrange<float>;
template class XYrange<double>;

} // end of namespace tiny_graph_plot
