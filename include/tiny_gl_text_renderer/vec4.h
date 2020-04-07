#pragma once

#include <cstdio>

namespace tiny_gl_text_renderer {

template<typename T>
class Vec4
{
public:
    Vec4(void) : _data{ 0.0, 0.0, 0.0, 0.0 } {}
    Vec4(const T x, const T y, const T z, const T w) :
        _data{ x, y, z, w } {}
    Vec4(const Vec4& other) :
        _data{ other._data[0], other._data[1], other._data[2], other._data[3] } {}
    Vec4(Vec4&& other) noexcept :
        _data{ other._data[0], other._data[1], other._data[2], other._data[3] } {}
    ~Vec4(void) {}
public:
    __forceinline Vec4& operator=(const Vec4& rhs) {
        _data[0] = rhs._data[0];
        _data[1] = rhs._data[1];
        _data[2] = rhs._data[2];
        _data[3] = rhs._data[3];
        return *this;
    }
    __forceinline Vec4& operator=(Vec4&& rhs) noexcept {
        _data[0] = rhs._data[0];
        _data[1] = rhs._data[1];
        _data[2] = rhs._data[2];
        _data[3] = rhs._data[3];
        return *this;
    }
    __forceinline const T* const GetData(void) const { return _data; }
    __forceinline const T operator[](const size_t idx) const { //TODO return byref?
        return _data[idx];
    }
    __forceinline T& operator[](const size_t idx) {
        return _data[idx];
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
    T _data[4];
};

template class Vec4<float>;
template class Vec4<double>;

typedef Vec4<float> Vec4f;
typedef Vec4<double> Vec4d;

} // end of namespace tiny_gl_text_renderer
