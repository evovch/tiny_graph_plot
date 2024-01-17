#pragma once

#include <string>

#include "tiny_gl_text_renderer/mat4.h"

typedef unsigned int GLuint;
typedef int GLint;

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::Mat4f;

class ShaderProgram
{
public:
    explicit ShaderProgram(const char* const name);
    ~ShaderProgram();
    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram(ShaderProgram&& other) = delete;
    ShaderProgram& operator=(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(ShaderProgram&& other) = delete;
public:
    void Generate(const char* const vsh_src, const char* const gsh_src, const char* const fsh_src);
    void Use() const;
    void CommitCamera1(
        const Mat4f& screen_to_viewport,
        const Mat4f& viewport_to_clip,
        const Mat4f& screen_to_clip) const;
    void CommitCamera2(const Mat4f& visrange_to_clip) const;
    const GLuint& GetProgId() const noexcept { return prog_; } //TODO get rid somehow
private:
    const std::string name_;
    GLuint prog_;
    GLint s2v_unif_; //!< screen-to-viewport matrix
    GLint v2c_unif_; //!< viewport-to-clip matrix
    GLint s2c_unif_; //!< screen-to-clip matrix
    GLint r2c_unif_; //!< visrange-to-clip matrix
};

} // end of namespace tiny_graph_plot
