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
    Vec2(void) : _data{ T(0.0), T(0.0) } {}
    Vec2(const T x, const T y)
    :   _data{ x, y } {}
    ~Vec2(void) = default;
    Vec2(const Vec2& other) = default;
    Vec2(Vec2&& other) = default;
    Vec2& operator=(const Vec2& other) = default;
    Vec2& operator=(Vec2&& other) = default;
public:
    const T* GetData(void) const { return _data.data(); }
    T operator[](const size_t i) const {
        assert(i < _data.size());
        return _data[i];
    }
    T& operator[](const size_t i) {
        assert(i < _data.size());
        return _data[i];
    }
    void Print(const char* suffix = "") const {
        printf("% 0.4f\t% 0.4f%s",
            _data[0], _data[1], suffix);
    }
public:
    T x(void) const { return _data[0]; }
    T y(void) const { return _data[1]; }
private:
    std::array<T, 2> _data;
};

template class Vec2<float>;
template class Vec2<double>;

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

} // end of namespace tiny_gl_text_renderer
