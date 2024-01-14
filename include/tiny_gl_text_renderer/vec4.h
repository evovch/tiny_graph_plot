#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <type_traits>

namespace tiny_gl_text_renderer
{

template<typename T>
class Vec4
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    Vec4(void) : _data{ 0.0, 0.0, 0.0, 0.0 } {}
    Vec4(const T x, const T y, const T z, const T w)
    :   _data{ x, y, z, w } {}
    ~Vec4(void) = default;
    Vec4(const Vec4& other) = default;
    Vec4(Vec4&& other) = default;
    Vec4& operator=(const Vec4& other) = default;
    Vec4& operator=(Vec4&& other) = default;
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
        printf("% 0.4f\t% 0.4f\t% 0.4f\t% 0.4f%s",
            _data[0], _data[1], _data[2], _data[3], suffix);
    }
public:
    __forceinline const T x(void) const { return _data[0]; }
    __forceinline const T y(void) const { return _data[1]; }
    __forceinline const T z(void) const { return _data[2]; }
    __forceinline const T w(void) const { return _data[3]; }
    __forceinline const T r(void) const { return _data[0]; }
    __forceinline const T g(void) const { return _data[1]; }
    __forceinline const T b(void) const { return _data[2]; }
    __forceinline const T a(void) const { return _data[3]; }
private:
    std::array<T, 4> _data;
};

template class Vec4<float>;
template class Vec4<double>;

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

} // end of namespace tiny_gl_text_renderer
