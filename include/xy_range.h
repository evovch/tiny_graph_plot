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
    :   x_min_(x_min), dx_(dx), y_min_(y_min), dy_(dy) {}
    ~XYrange() = default;
    XYrange(const XYrange& other) = default;
    XYrange(XYrange&& other) = default;
    XYrange& operator=(const XYrange& other) = default;
    XYrange& operator=(XYrange&& other) = default;
public:
    template<typename U>
    XYrange& operator=(const XYrange<U>& rhs) {
        x_min_ = static_cast<T>(rhs.lowx());
        dx_    = static_cast<T>(rhs.dx());
        y_min_ = static_cast<T>(rhs.lowy());
        dy_    = static_cast<T>(rhs.dy());
        return *this;
    }
    template<typename U>
    void Include(const XYrange<U>& other) {
        const T x_max = std::fmax(x_min_ + dx_, static_cast<T>(other.lowx() + other.dx()));
        const T y_max = std::fmax(y_min_ + dy_, static_cast<T>(other.lowy() + other.dy()));
        x_min_ = std::fmin(x_min_, static_cast<T>(other.lowx()));
        y_min_ = std::fmin(y_min_, static_cast<T>(other.lowy()));
        dx_ = x_max - x_min_;
        dy_ = y_max - y_min_;
    }
    void Include(const Vec2<T>& point) {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y())) return;
        const T x_max = std::fmax(x_min_ + dx_, point.x());
        const T y_max = std::fmax(y_min_ + dy_, point.y());
        x_min_ = std::fmin(x_min_, point.x());
        y_min_ = std::fmin(y_min_, point.y());
        dx_ = x_max - x_min_;
        dy_ = y_max - y_min_;

    }
    bool IncludesX(const T x) const noexcept {
        return (x >= x_min_) && (x <= x_min_ + dx_);
    }
    void FixDegenerateCases() noexcept {
        constexpr T epsilon = T(1.0e-8);
        if (dx_ <= epsilon) { // abs?
            x_min_ -= T(0.5);
            dx_     = T(1.0);
        }
        if (dy_ <= epsilon) { // abs?
            y_min_ -= T(0.5);
            dy_     = T(1.0);
        }
    }
    void Set1(const T x_min, const T x_max, const T y_min, const T y_max) noexcept {
        x_min_ = x_min; dx_ = x_max - x_min; y_min_ = y_min; dy_ = y_max - y_min;
    }
    void Set2(const T x_min, const T dx, const T y_min, const T dy) noexcept {
        x_min_ = x_min; dx_ = dx; y_min_ = y_min; dy_ = dy;
    }
    void SetXrange1(const T x_min, const T x_max) noexcept { x_min_ = x_min; dx_ = x_max - x_min; }
    void SetYrange1(const T y_min, const T y_max) noexcept { y_min_ = y_min; dy_ = y_max - y_min; }
    void SetXrange2(const T x_min, const T dx) noexcept { x_min_ = x_min; dx_ = dx; }
    void SetYrange2(const T y_min, const T dy) noexcept { y_min_ = y_min; dy_ = dy; }
    void MoveX(const T shift) noexcept { x_min_ += shift; }
    void MoveY(const T shift) noexcept { y_min_ += shift; }
    T lowx()  const noexcept { return x_min_; }
    T highx() const noexcept { return x_min_ + dx_; }
    T lowy()  const noexcept { return y_min_; }
    T highy() const noexcept { return y_min_ + dy_; }
    T dx()    const noexcept { return dx_; }
    T dy()    const noexcept { return dy_; }
    T xm()    const noexcept { return x_min_ + T(0.5) * dx_; }
    T ym()    const noexcept { return y_min_ + T(0.5) * dy_; }
    T twoxm() const noexcept { return T(2.0) * x_min_ + dx_; }
    T twoym() const noexcept { return T(2.0) * y_min_ + dy_; }
private:
    T x_min_ = T(0.0);
    T dx_    = T(1.0);
    T y_min_ = T(0.0);
    T dy_    = T(1.0);
};

template class XYrange<float>;
template class XYrange<double>;

} // end of namespace tiny_graph_plot
