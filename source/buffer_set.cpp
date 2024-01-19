#include "buffer_set.h"

#include "GL/glew.h"

#include "tiny_gl_text_renderer/data_types.h"

namespace tiny_graph_plot
{

BufferSet::BufferSet(const char* const name)
:   name_(name)
{
}

BufferSet::~BufferSet()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ibo_);
}

//TODO this should happen in the constructor.
void BufferSet::Generate()
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);
    
    glObjectLabel(GL_VERTEX_ARRAY, vao_, -1, (name_ + std::string("_vao")).c_str());
    glObjectLabel(GL_BUFFER, vbo_, -1, (name_ + std::string("_vbo")).c_str());
    glObjectLabel(GL_BUFFER, ibo_, -1, (name_ + std::string("_ibo")).c_str());
}

void BufferSet::Allocate(const unsigned int n_vert, const void* const data) const
{
    using v_str_t = tiny_gl_text_renderer::vertex_colored_t;
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(v_str_t), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(v_str_t),
        (void*)offsetof(v_str_t, coords_));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(v_str_t),
        (void*)offsetof(v_str_t, color_));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //glBindVertexArray(0); // Not really needed.
}

void BufferSet::SendVertices(const unsigned int n_vert, const void* const data) const
{
    using v_str_t = tiny_gl_text_renderer::vertex_colored_t;
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, n_vert * sizeof(v_str_t), data);
    //glBindVertexArray(0); // Not really needed.
}

void BufferSet::SendIndicesWires(const unsigned int n_primitives, const void* const data) const
{
    using tiny_gl_text_renderer::wire_t;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(wire_t), data, GL_STATIC_DRAW);
}

void BufferSet::SendIndicesMarkers(const unsigned int n_primitives, const void* const data) const
{
    using tiny_gl_text_renderer::marker_t;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(marker_t), data, GL_STATIC_DRAW);
}

void BufferSet::DrawWires(const unsigned int n_primitives) const
{
    constexpr unsigned int n_indices = 2u; // GL_LINES
    glBindVertexArray(vao_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glDrawElements(GL_LINES, n_indices * n_primitives, GL_UNSIGNED_INT, NULL);
    //glBindVertexArray(0); // Not really needed.
}

void BufferSet::DrawMarkers(const unsigned int n_primitives) const
{
    constexpr unsigned int n_indices = 1u; // GL_POINTS
    glBindVertexArray(vao_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glDrawElements(GL_POINTS, n_indices * n_primitives, GL_UNSIGNED_INT, NULL);
    //glBindVertexArray(0); // Not really needed.
}

} // end of namespace tiny_graph_plot
