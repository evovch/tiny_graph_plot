#include "shader_program.h"

#include "GL/glew.h"

namespace tiny_graph_plot
{

ShaderProgram::ShaderProgram(const char* const name)
:   name_(name)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(prog_);
}

void ShaderProgram::Generate(
    const char* const vsh_src,
    const char* const gsh_src,
    const char* const fsh_src)
{
    prog_ = glCreateProgram();

    glObjectLabel(GL_PROGRAM, prog_, -1, name_.c_str());

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glObjectLabel(GL_SHADER, vertex_shader, -1, (name_ + std::string("_vs")).c_str());
    glShaderSource(vertex_shader, 1, (const GLchar**)&vsh_src, NULL);
    glCompileShader(vertex_shader);
    glAttachShader(prog_, vertex_shader);

    GLuint geometry_shader = (GLuint)-1;
    if (gsh_src != nullptr) {
        geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
        glObjectLabel(GL_SHADER, geometry_shader, -1, (name_ + std::string("_gs")).c_str());
        glShaderSource(geometry_shader, 1, (const GLchar**)&gsh_src, NULL);
        glCompileShader(geometry_shader);
        glAttachShader(prog_, geometry_shader);
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glObjectLabel(GL_SHADER, fragment_shader, -1, (name_ + std::string("_fs")).c_str());
    glShaderSource(fragment_shader, 1, (const GLchar**)&fsh_src, NULL);
    glCompileShader(fragment_shader);
    glAttachShader(prog_, fragment_shader);

    glLinkProgram(prog_);

    glDetachShader(prog_, vertex_shader);
    glDeleteShader(vertex_shader);
    if (gsh_src != nullptr) {
        glDetachShader(prog_, geometry_shader);
        glDeleteShader(geometry_shader);
    }
    glDetachShader(prog_, fragment_shader);
    glDeleteShader(fragment_shader);

    s2v_unif_ = glGetUniformLocation(prog_, "screen2viewport");
    v2c_unif_ = glGetUniformLocation(prog_, "viewport2clip");
    s2c_unif_ = glGetUniformLocation(prog_, "screen2clip");
    r2c_unif_ = glGetUniformLocation(prog_, "visrange2clip");
}

void ShaderProgram::Use() const
{
    glUseProgram(prog_);
}

void ShaderProgram::CommitCamera1(
    const Mat4f& screen_to_viewport,
    const Mat4f& viewport_to_clip,
    const Mat4f& screen_to_clip) const
{
    glProgramUniformMatrix4fv(prog_, s2v_unif_, 1, GL_FALSE, screen_to_viewport.GetData());
    glProgramUniformMatrix4fv(prog_, v2c_unif_, 1, GL_FALSE, viewport_to_clip.GetData());
    glProgramUniformMatrix4fv(prog_, s2c_unif_, 1, GL_FALSE, screen_to_clip.GetData());
}

void ShaderProgram::CommitCamera2(const Mat4f& visrange_to_clip) const
{
    glProgramUniformMatrix4fv(prog_, r2c_unif_, 1, GL_FALSE, visrange_to_clip.GetData());
}

} // end of namespace tiny_graph_plot
