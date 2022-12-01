#pragma once

#include <cstdio>

#if defined(__linux__)
#define __forceinline inline
#endif

namespace tiny_gl_text_renderer {

template<typename T>
class Vec2
{
public:
    Vec2(void) : _data{ 0.0, 0.0 } {}
    Vec2(const T x, const T y) :
        _data{ x, y } {}
    Vec2(const Vec2& other) :
        _data{ other._data[0], other._data[1] } {}
    Vec2(Vec2&& other) noexcept :
        _data{ other._data[0], other._data[1] } {}
    ~Vec2(void) {}
public:
    __forceinline Vec2& operator=(const Vec2& rhs) {
        _data[0] = rhs._data[0];
        _data[1] = rhs._data[1];
        return *this;
    }
    __forceinline Vec2& operator=(Vec2&& rhs) noexcept {
        _data[0] = rhs._data[0];
        _data[1] = rhs._data[1];
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
        printf("% 0.4f\t% 0.4f%s",
            _data[0], _data[1], suffix);
    }
public:
    __forceinline const T x(void) const { return _data[0]; }
    __forceinline const T y(void) const { return _data[1]; }
private:
    T _data[2];
};

template class Vec2<float>;
template class Vec2<double>;

typedef Vec2<float> Vec2f;
typedef Vec2<double> Vec2d;

} // end of namespace tiny_gl_text_renderer
