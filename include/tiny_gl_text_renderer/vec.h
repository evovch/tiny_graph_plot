#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <type_traits>

namespace tiny_gl_text_renderer
{

template<typename T, size_t DIM>
class Vec
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
    static_assert(DIM == 2u
               || DIM == 3u
               || DIM == 4u, "");
public:
    explicit Vec(const T value = T(0.0)) noexcept { data_.fill(value); }
    template<typename... Args, typename std::enable_if<sizeof...(Args) == DIM, int>::type = 0>
    explicit Vec(Args ...args) noexcept : data_{ args... } {}
    ~Vec() = default;
    Vec(const Vec& other) = default;
    Vec(Vec&& other) = default;
    Vec& operator=(const Vec& other) = default;
    Vec& operator=(Vec&& other) = default;
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
    void Print(const char* suffix = "") const noexcept;
public:
    T x() const noexcept { return data_[0]; }
    T y() const noexcept { return data_[1]; }
private:
    std::array<T, DIM> data_;
};

template<> inline void Vec<float, 2>::Print(const char* suffix) const noexcept {
    printf("% 0.4f\t% 0.4f%s", data_[0], data_[1], suffix);
}
template<> inline void Vec<double, 2>::Print(const char* suffix) const noexcept {
    printf("% 0.4f\t% 0.4f%s", data_[0], data_[1], suffix);
}
template<> inline void Vec<float, 3>::Print(const char* suffix) const noexcept {
    printf("% 0.4f\t% 0.4f\t% 0.4f%s", data_[0], data_[1], data_[2], suffix);
}
template<> inline void Vec<double, 3>::Print(const char* suffix) const noexcept {
    printf("% 0.4f\t% 0.4f\t% 0.4f%s", data_[0], data_[1], data_[2], suffix);
}
template<> inline void Vec<float, 4>::Print(const char* suffix) const noexcept {
    printf("% 0.4f\t% 0.4f\t% 0.4f\t% 0.4f%s", data_[0], data_[1], data_[2], data_[3], suffix);
}
template<> inline void Vec<double, 4>::Print(const char* suffix) const noexcept {
    printf("% 0.4f\t% 0.4f\t% 0.4f\t% 0.4f%s", data_[0], data_[1], data_[2], data_[3], suffix);
}

template<typename T> using Vec2 = Vec<T, 2>;
template<typename T> using Vec3 = Vec<T, 3>;
template<typename T> using Vec4 = Vec<T, 4>;

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;

} // end of namespace tiny_gl_text_renderer
