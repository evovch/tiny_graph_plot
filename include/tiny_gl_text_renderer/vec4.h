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
    Vec4() noexcept : data_{ T(0.0), T(0.0), T(0.0), T(0.0) } {}
    Vec4(const T x, const T y, const T z, const T w) noexcept
    :   data_{ x, y, z, w } {}
    ~Vec4() = default;
    Vec4(const Vec4& other) = default;
    Vec4(Vec4&& other) = default;
    Vec4& operator=(const Vec4& other) = default;
    Vec4& operator=(Vec4&& other) = default;
public:
    const T* GetData() const noexcept { return data_.data(); }
    T operator[](const size_t i) const {
        assert(i < data_.size());
        return data_[i];
    }
    T& operator[](const size_t i) {
        assert(i < data_.size());
        return data_[i];
    }
    void Print(const char* suffix = "") const noexcept {
        printf("% 0.4f\t% 0.4f\t% 0.4f\t% 0.4f%s",
            data_[0], data_[1], data_[2], data_[3], suffix);
    }
public:
    T x() const noexcept { return data_[0]; }
    T y() const noexcept { return data_[1]; }
private:
    std::array<T, 4> data_;
};

template class Vec4<float>;
template class Vec4<double>;

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

} // end of namespace tiny_gl_text_renderer
