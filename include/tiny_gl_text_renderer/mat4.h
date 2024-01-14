#pragma once

#include <array>
#include <cassert>
#include <type_traits>

#include "vec.h"

namespace tiny_gl_text_renderer
{

template<typename T>
class Mat4
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    explicit Mat4() noexcept
    :   data_{ T(1.0), T(0.0), T(0.0), T(0.0),
               T(0.0), T(1.0), T(0.0), T(0.0),
               T(0.0), T(0.0), T(1.0), T(0.0),
               T(0.0), T(0.0), T(0.0), T(1.0) } {}
    explicit Mat4(
        const T m00, const T m01, const T m02, const T m03,
        const T m10, const T m11, const T m12, const T m13,
        const T m20, const T m21, const T m22, const T m23,
        const T m30, const T m31, const T m32, const T m33) noexcept
    :   data_{ m00, m01, m02, m03,
               m10, m11, m12, m13,
               m20, m21, m22, m23,
               m30, m31, m32, m33 } {}
    ~Mat4() = default;
    Mat4(const Mat4& other) = delete;
    Mat4(Mat4&& other) = delete;
    Mat4& operator=(const Mat4& other) = delete;
    Mat4& operator=(Mat4&& other) = delete;
public:
    void Reset() noexcept {
        data_[0*4+0] = T(1.0); data_[0*4+1] = T(0.0); data_[0*4+2] = T(0.0); data_[0*4+3] = T(0.0);
        data_[1*4+0] = T(0.0); data_[1*4+1] = T(1.0); data_[1*4+2] = T(0.0); data_[1*4+3] = T(0.0);
        data_[2*4+0] = T(0.0); data_[2*4+1] = T(0.0); data_[2*4+2] = T(1.0); data_[2*4+3] = T(0.0);
        data_[3*4+0] = T(0.0); data_[3*4+1] = T(0.0); data_[3*4+2] = T(0.0); data_[3*4+3] = T(1.0);
    }
    void Set(
        const T m00, const T m01, const T m02, const T m03,
        const T m10, const T m11, const T m12, const T m13,
        const T m20, const T m21, const T m22, const T m23,
        const T m30, const T m31, const T m32, const T m33) noexcept {
        data_[0*4+0] = m00; data_[0*4+1] = m01; data_[0*4+2] = m02; data_[0*4+3] = m03;
        data_[1*4+0] = m10; data_[1*4+1] = m11; data_[1*4+2] = m12; data_[1*4+3] = m13;
        data_[2*4+0] = m20; data_[2*4+1] = m21; data_[2*4+2] = m22; data_[2*4+3] = m23;
        data_[3*4+0] = m30; data_[3*4+1] = m31; data_[3*4+2] = m32; data_[3*4+3] = m33;
    }
    const T* const GetData() const noexcept { return data_.data(); }
    T operator[](const size_t i) const {
        assert(i < data_.size());
        return data_[i];
    }
    T& operator[](const size_t i) {
        assert(i < data_.size());
        return data_[i];
    }
    Vec4<T> operator*(const Vec4<T>& v) const noexcept {
        return Vec4<T>(
            data_[0*4+0]*v[0] + data_[1*4+0]*v[1] + data_[2*4+0]*v[2] + data_[3*4+0]*v[3],
            data_[0*4+1]*v[0] + data_[1*4+1]*v[1] + data_[2*4+1]*v[2] + data_[3*4+1]*v[3],
            data_[0*4+2]*v[0] + data_[1*4+2]*v[1] + data_[2*4+2]*v[2] + data_[3*4+2]*v[3],
            data_[0*4+3]*v[0] + data_[1*4+3]*v[1] + data_[2*4+3]*v[2] + data_[3*4+3]*v[3]
        );
    }
    Mat4<T> operator*(const Mat4<T>& m) const noexcept {
        return Mat4<T>(
            data_[0*4+0]*m[0*4+0] + data_[0*4+1]*m[1*4+0] + data_[0*4+2]*m[2*4+0] + data_[0*4+3]*m[3*4+0],
            data_[0*4+0]*m[0*4+1] + data_[0*4+1]*m[1*4+1] + data_[0*4+2]*m[2*4+1] + data_[0*4+3]*m[3*4+1],
            data_[0*4+0]*m[0*4+2] + data_[0*4+1]*m[1*4+2] + data_[0*4+2]*m[2*4+2] + data_[0*4+3]*m[3*4+2],
            data_[0*4+0]*m[0*4+3] + data_[0*4+1]*m[1*4+3] + data_[0*4+2]*m[2*4+3] + data_[0*4+3]*m[3*4+3],

            data_[1*4+0]*m[0*4+0] + data_[1*4+1]*m[1*4+0] + data_[1*4+2]*m[2*4+0] + data_[1*4+3]*m[3*4+0],
            data_[1*4+0]*m[0*4+1] + data_[1*4+1]*m[1*4+1] + data_[1*4+2]*m[2*4+1] + data_[1*4+3]*m[3*4+1],
            data_[1*4+0]*m[0*4+2] + data_[1*4+1]*m[1*4+2] + data_[1*4+2]*m[2*4+2] + data_[1*4+3]*m[3*4+2],
            data_[1*4+0]*m[0*4+3] + data_[1*4+1]*m[1*4+3] + data_[1*4+2]*m[2*4+3] + data_[1*4+3]*m[3*4+3],

            data_[2*4+0]*m[0*4+0] + data_[2*4+1]*m[1*4+0] + data_[2*4+2]*m[2*4+0] + data_[2*4+3]*m[3*4+0],
            data_[2*4+0]*m[0*4+1] + data_[2*4+1]*m[1*4+1] + data_[2*4+2]*m[2*4+1] + data_[2*4+3]*m[3*4+1],
            data_[2*4+0]*m[0*4+2] + data_[2*4+1]*m[1*4+2] + data_[2*4+2]*m[2*4+2] + data_[2*4+3]*m[3*4+2],
            data_[2*4+0]*m[0*4+3] + data_[2*4+1]*m[1*4+3] + data_[2*4+2]*m[2*4+3] + data_[2*4+3]*m[3*4+3],

            data_[3*4+0]*m[0*4+0] + data_[3*4+1]*m[1*4+0] + data_[3*4+2]*m[2*4+0] + data_[3*4+3]*m[3*4+0],
            data_[3*4+0]*m[0*4+1] + data_[3*4+1]*m[1*4+1] + data_[3*4+2]*m[2*4+1] + data_[3*4+3]*m[3*4+1],
            data_[3*4+0]*m[0*4+2] + data_[3*4+1]*m[1*4+2] + data_[3*4+2]*m[2*4+2] + data_[3*4+3]*m[3*4+2],
            data_[3*4+0]*m[0*4+3] + data_[3*4+1]*m[1*4+3] + data_[3*4+2]*m[2*4+3] + data_[3*4+3]*m[3*4+3]
        );
    }
private:
    std::array<T, 16> data_;
};

template class Mat4<float>;
template class Mat4<double>;

using Mat4f = Mat4<float>;
using Mat4d = Mat4<double>;

} // end of namespace tiny_gl_text_renderer
