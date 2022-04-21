#pragma once

#include "vec4.h"

namespace tiny_gl_text_renderer {

template<typename T>
class Mat4
{
public:
    Mat4(void) : _data{1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0}
    {}
    Mat4(const T m00, const T m01, const T m02, const T m03,
         const T m10, const T m11, const T m12, const T m13,
         const T m20, const T m21, const T m22, const T m23,
         const T m30, const T m31, const T m32, const T m33) :
        _data{m00, m01, m02, m03, m10, m11, m12, m13,
              m20, m21, m22, m23, m30, m31, m32, m33}
    {}
    ~Mat4(void) {}
    __forceinline void Reset(void) {
        _data[0 * 4 + 0] = 1.0; _data[0 * 4 + 1] = 0.0; _data[0 * 4 + 2] = 0.0; _data[0 * 4 + 3] = 0.0;
        _data[1 * 4 + 0] = 0.0; _data[1 * 4 + 1] = 1.0; _data[1 * 4 + 2] = 0.0; _data[1 * 4 + 3] = 0.0;
        _data[2 * 4 + 0] = 0.0; _data[2 * 4 + 1] = 0.0; _data[2 * 4 + 2] = 1.0; _data[2 * 4 + 3] = 0.0;
        _data[3 * 4 + 0] = 0.0; _data[3 * 4 + 1] = 0.0; _data[3 * 4 + 2] = 0.0; _data[3 * 4 + 3] = 1.0;
    }
    __forceinline void Set(
        const T m00, const T m01, const T m02, const T m03,
        const T m10, const T m11, const T m12, const T m13,
        const T m20, const T m21, const T m22, const T m23,
        const T m30, const T m31, const T m32, const T m33) {
        _data[0 * 4 + 0] = m00; _data[0 * 4 + 1] = m01; _data[0 * 4 + 2] = m02; _data[0 * 4 + 3] = m03;
        _data[1 * 4 + 0] = m10; _data[1 * 4 + 1] = m11; _data[1 * 4 + 2] = m12; _data[1 * 4 + 3] = m13;
        _data[2 * 4 + 0] = m20; _data[2 * 4 + 1] = m21; _data[2 * 4 + 2] = m22; _data[2 * 4 + 3] = m23;
        _data[3 * 4 + 0] = m30; _data[3 * 4 + 1] = m31; _data[3 * 4 + 2] = m32; _data[3 * 4 + 3] = m33;
    }
    __forceinline const T* const GetData(void) const { return _data; }
    __forceinline const T operator[](const size_t idx) const { //TODO return byref?
        return _data[idx];
    }
    __forceinline T& operator[](const size_t idx) {
        return _data[idx];
    }
    __forceinline Vec4<T> operator*(const Vec4<T>& op2) const {
        Vec4<T> ret;
        ret[0] = _data[0*4+0]*op2[0] + _data[1*4+0]*op2[1] + _data[2*4+0]*op2[2] + _data[3*4+0]*op2[3];
        ret[1] = _data[0*4+1]*op2[0] + _data[1*4+1]*op2[1] + _data[2*4+1]*op2[2] + _data[3*4+1]*op2[3];
        ret[2] = _data[0*4+2]*op2[0] + _data[1*4+2]*op2[1] + _data[2*4+2]*op2[2] + _data[3*4+2]*op2[3];
        ret[3] = _data[0*4+3]*op2[0] + _data[1*4+3]*op2[1] + _data[2*4+3]*op2[2] + _data[3*4+3]*op2[3];
        return ret;
    }
    __forceinline Mat4<T> operator*(const Mat4<T>& rhs) const {
        return Mat4<T>(
            _data[0*4+0]*rhs[0*4+0] + _data[0*4+1]*rhs[1*4+0] + _data[0*4+2]*rhs[2*4+0] + _data[0*4+3]*rhs[3*4+0],
            _data[0*4+0]*rhs[0*4+1] + _data[0*4+1]*rhs[1*4+1] + _data[0*4+2]*rhs[2*4+1] + _data[0*4+3]*rhs[3*4+1],
            _data[0*4+0]*rhs[0*4+2] + _data[0*4+1]*rhs[1*4+2] + _data[0*4+2]*rhs[2*4+2] + _data[0*4+3]*rhs[3*4+2],
            _data[0*4+0]*rhs[0*4+3] + _data[0*4+1]*rhs[1*4+3] + _data[0*4+2]*rhs[2*4+3] + _data[0*4+3]*rhs[3*4+3],

            _data[1*4+0]*rhs[0*4+0] + _data[1*4+1]*rhs[1*4+0] + _data[1*4+2]*rhs[2*4+0] + _data[1*4+3]*rhs[3*4+0],
            _data[1*4+0]*rhs[0*4+1] + _data[1*4+1]*rhs[1*4+1] + _data[1*4+2]*rhs[2*4+1] + _data[1*4+3]*rhs[3*4+1],
            _data[1*4+0]*rhs[0*4+2] + _data[1*4+1]*rhs[1*4+2] + _data[1*4+2]*rhs[2*4+2] + _data[1*4+3]*rhs[3*4+2],
            _data[1*4+0]*rhs[0*4+3] + _data[1*4+1]*rhs[1*4+3] + _data[1*4+2]*rhs[2*4+3] + _data[1*4+3]*rhs[3*4+3],

            _data[2*4+0]*rhs[0*4+0] + _data[2*4+1]*rhs[1*4+0] + _data[2*4+2]*rhs[2*4+0] + _data[2*4+3]*rhs[3*4+0],
            _data[2*4+0]*rhs[0*4+1] + _data[2*4+1]*rhs[1*4+1] + _data[2*4+2]*rhs[2*4+1] + _data[2*4+3]*rhs[3*4+1],
            _data[2*4+0]*rhs[0*4+2] + _data[2*4+1]*rhs[1*4+2] + _data[2*4+2]*rhs[2*4+2] + _data[2*4+3]*rhs[3*4+2],
            _data[2*4+0]*rhs[0*4+3] + _data[2*4+1]*rhs[1*4+3] + _data[2*4+2]*rhs[2*4+3] + _data[2*4+3]*rhs[3*4+3],

            _data[3*4+0]*rhs[0*4+0] + _data[3*4+1]*rhs[1*4+0] + _data[3*4+2]*rhs[2*4+0] + _data[3*4+3]*rhs[3*4+0],
            _data[3*4+0]*rhs[0*4+1] + _data[3*4+1]*rhs[1*4+1] + _data[3*4+2]*rhs[2*4+1] + _data[3*4+3]*rhs[3*4+1],
            _data[3*4+0]*rhs[0*4+2] + _data[3*4+1]*rhs[1*4+2] + _data[3*4+2]*rhs[2*4+2] + _data[3*4+3]*rhs[3*4+2],
            _data[3*4+0]*rhs[0*4+3] + _data[3*4+1]*rhs[1*4+3] + _data[3*4+2]*rhs[2*4+3] + _data[3*4+3]*rhs[3*4+3]);
    }
private:
    T _data[16];
};

template class Mat4<float>;
template class Mat4<double>;

typedef Mat4<float> Mat4f;
typedef Mat4<double> Mat4d;

} // end of namespace tiny_gl_text_renderer
