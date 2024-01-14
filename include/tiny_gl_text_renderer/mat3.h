#pragma once

#include <array>
#include <cassert>
#include <type_traits>

#include "vec3.h"

namespace tiny_gl_text_renderer
{

template<typename T>
class Mat3
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
    explicit Mat3() noexcept
    :   data_{ T(1.0), T(0.0), T(0.0),
               T(0.0), T(1.0), T(0.0),
               T(0.0), T(0.0), T(1.0) } {}

    explicit Mat3(
        const T m00, const T m01, const T m02,
        const T m10, const T m11, const T m12,
        const T m20, const T m21, const T m22) noexcept

    :   data_{ m00, m01, m02,
               m10, m11, m12,
               m20, m21, m22 } {}

    ~Mat3() = default;
    Mat3(const Mat3& other) = delete;
    Mat3(Mat3&& other) = delete;
    Mat3& operator=(const Mat3& other) = delete;
    Mat3& operator=(Mat3&& other) = delete;
public:
    void Reset() noexcept {
        data_[0*3+0] = T(1.0); data_[0*3+1] = T(0.0); data_[0*3+2] = T(0.0);
        data_[1*3+0] = T(0.0); data_[1*3+1] = T(1.0); data_[1*3+2] = T(0.0);
        data_[2*3+0] = T(0.0); data_[2*3+1] = T(0.0); data_[2*3+2] = T(1.0);

    }
    void Set(
        const T m00, const T m01, const T m02,
        const T m10, const T m11, const T m12,
        const T m20, const T m21, const T m22) noexcept {

        data_[0*3+0] = m00; data_[0*3+1] = m01; data_[0*3+2] = m02;
        data_[1*3+0] = m10; data_[1*3+1] = m11; data_[1*3+2] = m12;
        data_[2*3+0] = m20; data_[2*3+1] = m21; data_[2*3+2] = m22;

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
    Vec3<T> operator*(const Vec3<T>& v) const noexcept {
        return Vec3<T>(
            data_[0*3+0] * v[0] + data_[1*3+0] * v[1] + data_[2*3+0] * v[2],
            data_[0*3+1] * v[0] + data_[1*3+1] * v[1] + data_[2*3+1] * v[2],
            data_[0*3+2] * v[0] + data_[1*3+2] * v[1] + data_[2*3+2] * v[2]
        
        );
    }
private:
    std::array<T, 9> data_;
};

template class Mat3<float>;
template class Mat3<double>;

using Mat3f = Mat3<float>;
using Mat3d = Mat3<double>;

} // end of namespace tiny_gl_text_renderer
