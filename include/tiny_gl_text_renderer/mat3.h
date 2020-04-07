#pragma once

#include "vec3.h"

namespace tiny_gl_text_renderer {

template<typename T>
class Mat3
{
public:
    Mat3(void) : _data{1.0, 0.0, 0.0,
                       0.0, 1.0, 0.0,
                       0.0, 0.0, 1.0}
    {}
    Mat3(const T m00, const T m01, const T m02,
         const T m10, const T m11, const T m12,
         const T m20, const T m21, const T m22) :
        _data{m00, m01, m02, m10, m11, m12, m20, m21, m22}
    {}
    ~Mat3(void) {}
    __forceinline void Reset(void) {
        _data[0 * 3 + 0] = 1.0; _data[0 * 3 + 1] = 0.0; _data[0 * 3 + 2] = 0.0;
        _data[1 * 3 + 0] = 0.0; _data[1 * 3 + 1] = 1.0; _data[1 * 3 + 2] = 0.0;
        _data[2 * 3 + 0] = 0.0; _data[2 * 3 + 1] = 0.0; _data[2 * 3 + 2] = 1.0;
    }
    __forceinline void Set(
        const T m00, const T m01, const T m02,
        const T m10, const T m11, const T m12,
        const T m20, const T m21, const T m22) {
        _data[0 * 3 + 0] = m00; _data[0 * 3 + 1] = m01; _data[0 * 3 + 2] = m02;
        _data[1 * 3 + 0] = m10; _data[1 * 3 + 1] = m11; _data[1 * 3 + 2] = m12;
        _data[2 * 3 + 0] = m20; _data[2 * 3 + 1] = m21; _data[2 * 3 + 2] = m22;
    }
    __forceinline const T* const GetData(void) const { return _data; }
    __forceinline const T operator[](const size_t idx) const { //TODO return byref?
        return _data[idx];
    }
    __forceinline T& operator[](const size_t idx) {
        return _data[idx];
    }
    __forceinline Vec3<T> operator*(const Vec3<T>& op2) const {
        Vec3<T> ret;
        ret[0] = _data[0 * 3 + 0] * op2[0] + _data[1 * 3 + 0] * op2[1] + _data[2 * 3 + 0] * op2[2];
        ret[1] = _data[0 * 3 + 1] * op2[0] + _data[1 * 3 + 1] * op2[1] + _data[2 * 3 + 1] * op2[2];
        ret[2] = _data[0 * 3 + 2] * op2[0] + _data[1 * 3 + 2] * op2[1] + _data[2 * 3 + 2] * op2[2];
        return ret;
    }
private:
    T _data[9];
};

template class Mat3<float>;
template class Mat3<double>;

typedef Mat3<float> Mat3f;
typedef Mat3<double> Mat3d;

} // end of namespace tiny_gl_text_renderer
