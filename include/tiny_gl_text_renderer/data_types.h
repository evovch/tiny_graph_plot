#pragma once

#include "vec2.h"
#include "vec4.h"

namespace tiny_gl_text_renderer
{

using point_t = Vec4f;
using color_t = Vec4f;
using tex_coords_t = Vec2f;

class vertex_colored_t
{
public:
    explicit vertex_colored_t() = default;
    explicit vertex_colored_t(
        const float x, const float y, const float z, const float w,
        const float r, const float g, const float b, const float a) noexcept
    :   coords_(x, y, z, w),
        color_(r, g, b, a) {}
    ~vertex_colored_t() = default;
    vertex_colored_t(const vertex_colored_t& other) = default;
    vertex_colored_t(vertex_colored_t&& other) = default;
    vertex_colored_t& operator=(const vertex_colored_t& other) = default;
    vertex_colored_t& operator=(vertex_colored_t&& other) = default;
public:
    point_t coords_;
    color_t color_;
};

class vertex_textured_t
{
public:
    explicit vertex_textured_t() = default;
    explicit vertex_textured_t(
        const float x, const float y, const float z, const float w,
        const float u, const float v) noexcept
    :   coords_(x, y, z, w),
        tex_coords_(u, v) {}
    ~vertex_textured_t() = default;
    vertex_textured_t(const vertex_textured_t& other) = default;
    vertex_textured_t(vertex_textured_t&& other) = default;
    vertex_textured_t& operator=(const vertex_textured_t& other) = default;
    vertex_textured_t& operator=(vertex_textured_t&& other) = default;
public:
    point_t coords_;
    tex_coords_t tex_coords_;
};

class marker_t
{
public:
    explicit marker_t() = default;
    marker_t(const unsigned int p0) noexcept
    :   v0(p0) {}
    ~marker_t() = default;
    marker_t(const marker_t& other) = default;
    marker_t(marker_t&& other) = default;
    marker_t& operator=(const marker_t& other) = default;
    marker_t& operator=(marker_t&& other) = default;
public:
    unsigned int v0 = 0u;
};

class wire_t
{
public:
    explicit wire_t() = default;
    wire_t(const unsigned int p0, const unsigned int p1) noexcept
    :   v0(p0), v1(p1) {}
    ~wire_t() = default;
    wire_t(const wire_t& other) = default;
    wire_t(wire_t&& other) = default;
    wire_t& operator=(const wire_t& other) = default;
    wire_t& operator=(wire_t&& other) = default;
public:
    unsigned int v0 = 0u;
    unsigned int v1 = 0u;
};

class triangle_t
{
public:
    explicit triangle_t() = default;
    triangle_t(const unsigned int p0, const unsigned int p1,
               const unsigned int p2) noexcept
    :   v0(p0), v1(p1), v2(p2) {}
    ~triangle_t() = default;
    triangle_t(const triangle_t& other) = default;
    triangle_t(triangle_t&& other) = default;
    triangle_t& operator=(const triangle_t& other) = default;
    triangle_t& operator=(triangle_t&& other) = default;
public:
    unsigned int v0 = 0u;
    unsigned int v1 = 0u;
    unsigned int v2 = 0u;
};

class quad_t
{
public:
    explicit quad_t() = default;
    quad_t(const unsigned int p0, const unsigned int p1,
           const unsigned int p2, const unsigned int p3) noexcept
    :   v0(p0), v1(p1), v2(p2), v3(p3) {}
    ~quad_t() = default;
    quad_t(const quad_t& other) = default;
    quad_t(quad_t&& other) = default;
    quad_t& operator=(const quad_t& other) = default;
    quad_t& operator=(quad_t&& other) = default;
public:
    unsigned int v0 = 0u;
    unsigned int v1 = 0u;
    unsigned int v2 = 0u;
    unsigned int v3 = 0u;
};

} // end of namespace tiny_gl_text_renderer
