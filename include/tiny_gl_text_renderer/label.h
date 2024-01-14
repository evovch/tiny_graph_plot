#pragma once

#include <string>
#include <cmath>
#include <utility>

#include "data_types.h"
#include "mat3.h"
#include "texture_filler.h"

namespace tiny_gl_text_renderer
{

typedef unsigned int GLuint;

class Label
{
public:
    explicit Label(const char* string, const int x, const int y,
        const color_t& color, const float scaling, const float angle = 0.0f)
    :   _string(string), _x(x), _y(y), _color(color),
        _scaling(scaling), _angle(angle), _texture_data(nullptr) {
        const size_t newSize = GetRequiredTextureSize(string,
            _texture_w, _texture_h) * 4 * sizeof(float);
        _texture_data = (float*)malloc(newSize);
        tiny_gl_text_renderer::FillString(string, _texture_data, 4,
            _texture_w, _texture_h, 0, 0, color.GetData(), 4);
    }
    ~Label(void) {
        if (_texture_data != nullptr) free(_texture_data);
    }
    Label(const Label& other) = delete;
    Label(Label&& other) noexcept
    :   _string(std::move(other._string)), _x(other._x), _y(other._y),
        _color(std::move(other._color)), _scaling(other._scaling),
        _angle(other._angle), _texture_w(other._texture_w),
        _texture_h(other._texture_h),
        _texture_data(std::exchange(other._texture_data, nullptr)),
        tex_id_(other.tex_id_)
    {}
    Label& operator=(const Label& other) = delete;
    Label& operator=(Label&& other) = delete;
public:
    void UpdateString(const char* string) {
        std::string new_string(string);
        if (_string == new_string) return;
        _string = new_string;
        const size_t newSize = GetRequiredTextureSize(string,
            _texture_w, _texture_h) * 4 * sizeof(float);
        _texture_data = (float*)realloc(_texture_data, newSize);
        if (_texture_data == nullptr) return; //ERROR
        tiny_gl_text_renderer::FillString(string, _texture_data, 4,
            _texture_w, _texture_h, 0, 0, _color.GetData(), 4);
    }
    void UpdatePosition(const int x, const int y) noexcept { _x = x; _y = y; }
    void UpdatePositionX(const int x) noexcept { _x = x; }
    void UpdatePositionY(const int y) noexcept { _y = y; }
    void UpdateRotation(const float angle) noexcept { _angle = angle; }
    Mat3f GetMatrix() const noexcept {
        constexpr double deg_to_rad = 3.14159265358979323846 / 180.0;
        const float angle_rad = _angle * (float)deg_to_rad;
        const float sintheta = std::sin(angle_rad);
        const float costheta = std::cos(angle_rad);
        return Mat3f(
            _scaling * costheta, -_scaling * sintheta, 0.0f,
            _scaling * sintheta,  _scaling * costheta, 0.0f,
            static_cast<float>(_x), static_cast<float>(_y), 1.0f
        );
    }
    const std::string& GetString() const noexcept { return _string; }
    int GetTexX() const noexcept { return _x; }
    int GetTexY() const noexcept { return _y; }
    float GetScaling() const noexcept { return _scaling; }
    float GetAngle() const noexcept { return _angle; }
    size_t GetTexW() const noexcept { return _texture_w; }
    size_t GetTexH() const noexcept { return _texture_h; }
    const float* GetTexData() const noexcept { return _texture_data; }
private:
    std::string _string;
    int _x;
    int _y;
    color_t _color;
    float _scaling;
    float _angle;
    size_t _texture_w;
    size_t _texture_h;
    float* _texture_data;
public:
    GLuint tex_id_ = 0; //TODO invent proper initialization
};

} // end of namespace tiny_gl_text_renderer
