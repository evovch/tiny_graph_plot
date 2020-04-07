#pragma once

#include "vec2.h"
#include "vec4.h"

#include <utility> // required for std::move

namespace tiny_gl_text_renderer {

typedef Vec4f point_t;
typedef Vec4f color_t;
typedef Vec2f tex_coords_t;

class vertex_colored_t {
public:
    vertex_colored_t(void) {}
    vertex_colored_t(
        const float x, const float y, const float z, const float w,
        const float r, const float g, const float b, const float a) :
        _coords(x, y, z, w),
        _color(r, g, b, a) {}
    vertex_colored_t(const vertex_colored_t& other) :
        _coords(other._coords),
        _color(other._color) {}
    vertex_colored_t(vertex_colored_t&& other) noexcept :
        _coords(std::move(other._coords)),
        _color(std::move(other._color)) {}
    ~vertex_colored_t(void) {}
    __forceinline vertex_colored_t& operator=(const vertex_colored_t& rhs) {
        _coords = rhs._coords;
        _color = rhs._color;
        return *this;
    }
public:
    point_t _coords;
    color_t _color;
};

class vertex_textured_t {
public:
    vertex_textured_t(void) {}
    vertex_textured_t(
        const float x, const float y, const float z,
        const float w, const float u, const float v) :
        _coords(x, y, z, w),
        _tex_coords(u, v) {}
    vertex_textured_t(const vertex_textured_t& other) :
        _coords(other._coords),
        _tex_coords(other._tex_coords) {}
    vertex_textured_t(vertex_textured_t&& other) noexcept :
        _coords(std::move(other._coords)),
        _tex_coords(std::move(other._tex_coords)) {}
    ~vertex_textured_t(void) {}
    __forceinline vertex_textured_t& operator=(const vertex_textured_t& rhs) {
        _coords = rhs._coords;
        _tex_coords = rhs._tex_coords;
        return *this;
    }
public:
    point_t _coords;
    tex_coords_t _tex_coords;
};

class marker_t {
public:
    marker_t(void) : v0(0) {}
    marker_t(const unsigned int p0, const unsigned int p1) : v0(p0) {}
    marker_t(const marker_t& other) : v0(other.v0) {}
    marker_t(marker_t&& other) noexcept : v0(other.v0) {}
    ~marker_t(void) {}
    __forceinline marker_t& operator=(const marker_t& rhs) {
        v0 = rhs.v0;
        return *this;
    }
public:
    unsigned int v0;
};

class wire_t {
public:
    wire_t(void) : v0(0), v1(0) {}
    wire_t(const unsigned int p0, const unsigned int p1) : v0(p0), v1(p1) {}
    wire_t(const wire_t& other) : v0(other.v0), v1(other.v1) {}
    wire_t(wire_t&& other) noexcept : v0(other.v0), v1(other.v1) {}
    ~wire_t(void) {}
    __forceinline wire_t& operator=(const wire_t& rhs) {
        v0 = rhs.v0;
        v1 = rhs.v1;
        return *this;
    }
public:
    unsigned int v0;
    unsigned int v1;
};

class triangle_t {
public:
    triangle_t(void) :
        v0(0), v1(0), v2(0) {}
    triangle_t(const unsigned int p0, const unsigned int p1,
        const unsigned int p2) :
        v0(p0), v1(p1), v2(p2) {}
    triangle_t(const triangle_t& other) :
        v0(other.v0), v1(other.v1), v2(other.v2) {}
    triangle_t(triangle_t&& other) noexcept :
        v0(other.v0), v1(other.v1), v2(other.v2) {}
    ~triangle_t(void) {}
    __forceinline triangle_t& operator=(const triangle_t& rhs) {
        v0 = rhs.v0;
        v1 = rhs.v1;
        v2 = rhs.v2;
        return *this;
    }
public:
    unsigned int v0;
    unsigned int v1;
    unsigned int v2;
};

class quad_t {
public:
    quad_t(void) :
        v0(0), v1(0), v2(0), v3(0) {}
    quad_t(const unsigned int p0, const unsigned int p1,
        const unsigned int p2, const unsigned int p3) :
        v0(p0), v1(p1), v2(p2), v3(p3) {}
    quad_t(const quad_t& other) :
        v0(other.v0), v1(other.v1), v2(other.v2), v3(other.v3) {}
    quad_t(quad_t&& other) noexcept :
        v0(other.v0), v1(other.v1),
        v2(other.v2), v3(other.v3) {}
    ~quad_t(void) {}
    __forceinline quad_t& operator=(const quad_t& rhs) {
        v0 = rhs.v0;
        v1 = rhs.v1;
        v2 = rhs.v2;
        v3 = rhs.v3;
        return *this;
    }
public:
    unsigned int v0;
    unsigned int v1;
    unsigned int v2;
    unsigned int v3;
};

} // end of namespace tiny_gl_text_renderer
