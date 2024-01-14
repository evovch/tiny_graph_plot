#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <type_traits>

#if defined(__linux__)
#define __forceinline inline
#endif

namespace tiny_gl_text_renderer
{

template<typename T>
class Vec2
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    Vec2(void) : _data{ 0.0, 0.0 } {}
    Vec2(const T x, const T y)
    :   _data{ x, y } {}
    ~Vec2(void) = default;
    Vec2(const Vec2& other) = default;
    Vec2(Vec2&& other) = default;
    Vec2& operator=(const Vec2& other) = default;
    Vec2& operator=(Vec2&& other) = default;
public:
    __forceinline const T* const GetData(void) const { return _data.data(); }
    __forceinline const T operator[](const size_t i) const { //TODO return byref?
        assert(i < _data.size());
        return _data[i];
    }
    __forceinline T& operator[](const size_t i) {
        assert(i < _data.size());
        return _data[i];
    }
    void Print(const char* suffix = "") const {
        printf("% 0.4f\t% 0.4f%s",
            _data[0], _data[1], suffix);
    }
public:
    __forceinline const T x(void) const { return _data[0]; }
    __forceinline const T y(void) const { return _data[1]; }
private:
    std::array<T, 2> _data;
};

template class Vec2<float>;
template class Vec2<double>;

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

} // end of namespace tiny_gl_text_renderer
