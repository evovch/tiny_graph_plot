#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <type_traits>

namespace tiny_gl_text_renderer
{

template<typename T>
class Vec2
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    explicit Vec2() noexcept : data_{ T(0.0), T(0.0) } {}
    explicit Vec2(const T x, const T y) noexcept
    :   data_{ x, y } {}
    ~Vec2() = default;
    Vec2(const Vec2& other) = default;
    Vec2(Vec2&& other) = default;
    Vec2& operator=(const Vec2& other) = default;
    Vec2& operator=(Vec2&& other) = default;
public:
    const T* GetData() const noexcept { return data_.data(); }
    T operator[](const size_t i) const {
        assert(i < data_.size());
        return data_[i];
    }
    T& operator[](const size_t i) {
        assert(i < data_.size());
        return data_[i];
    }
    void Print(const char* suffix = "") const noexcept {
        printf("% 0.4f\t% 0.4f%s",
            data_[0], data_[1], suffix);
    }
public:
    T x() const noexcept { return data_[0]; }
    T y() const noexcept { return data_[1]; }
private:
    std::array<T, 2> data_;
};

template class Vec2<float>;
template class Vec2<double>;

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

} // end of namespace tiny_gl_text_renderer
