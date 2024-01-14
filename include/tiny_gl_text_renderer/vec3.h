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
    Vec3(void) : data_{ T(0.0), T(0.0), T(0.0) } {}
    Vec3(const T x, const T y, const T w)
    :   data_{ x, y, w } {}
    ~Vec3(void) = default;
    Vec3(const Vec3& other) = default;
    Vec3(Vec3&& other) = default;
    Vec3& operator=(const Vec3& other) = default;
    Vec3& operator=(Vec3&& other) = default;
public:
    const T* GetData(void) const { return data_.data(); }
    T operator[](const size_t i) const {
        assert(i < data_.size());
        return data_[i];
    }
    T& operator[](const size_t i) {
        assert(i < data_.size());
        return data_[i];
    }
    void Print(const char* suffix = "") const {
        printf("% 0.4f\t% 0.4f\t% 0.4f%s",
            data_[0], data_[1], data_[2], suffix);
    }
public:
    T x(void) const { return data_[0]; }
    T y(void) const { return data_[1]; }
    T w(void) const { return data_[2]; }
private:
    std::array<T, 3> data_;
};

template class Vec3<float>;
template class Vec3<double>;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

} // end of namespace tiny_gl_text_renderer
