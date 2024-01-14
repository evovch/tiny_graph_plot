#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <type_traits>

namespace tiny_gl_text_renderer
{

template<typename T>
class Vec3
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    Vec3(void) : _data{ 0.0, 0.0, 0.0 } {}
    Vec3(const T x, const T y, const T w)
    :   _data{ x, y, w } {}
    ~Vec3(void) = default;
    Vec3(const Vec3& other) = default;
    Vec3(Vec3&& other) = default;
    Vec3& operator=(const Vec3& other) = default;
    Vec3& operator=(Vec3&& other) = default;
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
        printf("% 0.4f\t% 0.4f\t% 0.4f%s",
            _data[0], _data[1], _data[2], suffix);
    }
public:
    __forceinline const T x(void) const { return _data[0]; }
    __forceinline const T y(void) const { return _data[1]; }
    __forceinline const T w(void) const { return _data[2]; }
private:
    std::array<T, 3> _data;
};

template class Vec3<float>;
template class Vec3<double>;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

} // end of namespace tiny_gl_text_renderer
