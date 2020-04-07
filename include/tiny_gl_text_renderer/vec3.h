#pragma once

#include <cstdio>

namespace tiny_gl_text_renderer {

template<typename T>
class Vec3
{
public:
    Vec3(void) : _data{ 0.0, 0.0, 0.0 } {}
    Vec3(const T x, const T y, const T w) :
        _data{ x, y, w } {}
    Vec3(const Vec3& other) :
        _data{ other._data[0], other._data[1], other._data[2] } {}
    Vec3(Vec3&& other) noexcept :
        _data{ other._data[0], other._data[1], other._data[2] } {}
    ~Vec3(void) {}
public:
    __forceinline Vec3& operator=(const Vec3& rhs) {
        _data[0] = rhs._data[0];
        _data[1] = rhs._data[1];
        _data[2] = rhs._data[2];
        return *this;
    }
    __forceinline Vec3& operator=(Vec3&& rhs) noexcept {
        _data[0] = rhs._data[0];
        _data[1] = rhs._data[1];
        _data[2] = rhs._data[2];
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
        printf("% 0.4f\t% 0.4f\t% 0.4f%s",
            _data[0], _data[1], _data[2], suffix);
    }
public:
    __forceinline const T x(void) const { return _data[0]; }
    __forceinline const T y(void) const { return _data[1]; }
    __forceinline const T w(void) const { return _data[2]; }
private:
    T _data[3];
};

template class Vec3<float>;
template class Vec3<double>;

typedef Vec3<float> Vec3f;
typedef Vec3<double> Vec3d;

} // end of namespace tiny_gl_text_renderer
