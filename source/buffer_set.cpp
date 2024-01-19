#include "buffer_set.h"

#include "GL/glew.h"

namespace tiny_graph_plot
{

template<typename VERTEX_TYPE>
BufferSet<VERTEX_TYPE>::BufferSet(const char* const name)
:   name_(name)
{
}

template<typename VERTEX_TYPE>
BufferSet<VERTEX_TYPE>::~BufferSet()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ibo_);
}

//TODO this should happen in the constructor.
template<typename VERTEX_TYPE>
void BufferSet<VERTEX_TYPE>::Generate()
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);
    
    glObjectLabel(GL_VERTEX_ARRAY, vao_, -1, (name_ + std::string("_vao")).c_str());
    glObjectLabel(GL_BUFFER, vbo_, -1, (name_ + std::string("_vbo")).c_str());
    glObjectLabel(GL_BUFFER, ibo_, -1, (name_ + std::string("_ibo")).c_str());
}

template<>
void BufferSet<tiny_gl_text_renderer::vertex_colored_t>::Allocate(
    const unsigned int n_vert, const void* const data) const
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

template<>
void BufferSet<tiny_gl_text_renderer::vertex_textured_t>::Allocate(
    const unsigned int n_vert, const void* const data) const
{
    using v_str_t = tiny_gl_text_renderer::vertex_textured_t;
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, n_vert * sizeof(v_str_t), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(v_str_t),
        (void*)offsetof(v_str_t, coords_));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v_str_t),
        (void*)offsetof(v_str_t, tex_coords_));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //glBindVertexArray(0); // Not really needed.
}

template<typename VERTEX_TYPE>
void BufferSet<VERTEX_TYPE>::SendVertices(const unsigned int n_vert, const void* const data,
                             const unsigned int offset) const
{
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(VERTEX_TYPE), n_vert * sizeof(VERTEX_TYPE), data);
    //glBindVertexArray(0); // Not really needed.
}

// ================================================================================================

//template<typename VERTEX_TYPE>
//template<typename PRIMITIVE_TYPE>
//void BufferSet<VERTEX_TYPE>::SendIndices(const unsigned int n_primitives, const PRIMITIVE_TYPE* const data) const
//{
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(PRIMITIVE_TYPE), data, GL_STATIC_DRAW);
//}

// ================================================================================================

template<>
template<>
void BufferSet<tiny_gl_text_renderer::vertex_colored_t>::SendIndices(
    const unsigned int n_primitives, const tiny_gl_text_renderer::quad_t* const data) const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(tiny_gl_text_renderer::quad_t), data, GL_STATIC_DRAW);
}

template<>
template<>
void BufferSet<tiny_gl_text_renderer::vertex_colored_t>::SendIndices(
    const unsigned int n_primitives, const tiny_gl_text_renderer::wire_t* const data) const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(tiny_gl_text_renderer::wire_t), data, GL_STATIC_DRAW);
}

template<>
template<>
void BufferSet<tiny_gl_text_renderer::vertex_colored_t>::SendIndices(
    const unsigned int n_primitives, const tiny_gl_text_renderer::marker_t* const data) const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(tiny_gl_text_renderer::marker_t), data, GL_STATIC_DRAW);
}

template<>
template<>
void BufferSet<tiny_gl_text_renderer::vertex_textured_t>::SendIndices(
    const unsigned int n_primitives, const tiny_gl_text_renderer::quad_t* const data) const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(tiny_gl_text_renderer::quad_t), data, GL_STATIC_DRAW);
}

template<>
template<>
void BufferSet<tiny_gl_text_renderer::vertex_textured_t>::SendIndices(
    const unsigned int n_primitives, const tiny_gl_text_renderer::wire_t* const data) const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(tiny_gl_text_renderer::wire_t), data, GL_STATIC_DRAW);
}

template<>
template<>
void BufferSet<tiny_gl_text_renderer::vertex_textured_t>::SendIndices(
    const unsigned int n_primitives, const tiny_gl_text_renderer::marker_t* const data) const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_primitives * sizeof(tiny_gl_text_renderer::marker_t), data, GL_STATIC_DRAW);
}

// ================================================================================================

template<unsigned int n_indices, GLenum mode>
static inline void DrawPrimitives(const unsigned int n_primitives, const GLuint vao, const GLuint ibo)
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(mode, n_indices * n_primitives, GL_UNSIGNED_INT, NULL);
    //glBindVertexArray(0); // Not really needed.
}

template<typename VERTEX_TYPE>
void BufferSet<VERTEX_TYPE>::DrawQuads(const unsigned int n_primitives) const
{
    DrawPrimitives<4u, GL_QUADS>(n_primitives, vao_, ibo_);
}

template<typename VERTEX_TYPE>
void BufferSet<VERTEX_TYPE>::DrawWires(const unsigned int n_primitives) const
{
    DrawPrimitives<2u, GL_LINES>(n_primitives, vao_, ibo_);
}

template<typename VERTEX_TYPE>
void BufferSet<VERTEX_TYPE>::DrawMarkers(const unsigned int n_primitives) const
{
    DrawPrimitives<1u, GL_POINTS>(n_primitives, vao_, ibo_);
}

template<typename VERTEX_TYPE>
void BufferSet<VERTEX_TYPE>::DrawQuadsWithTextures(const size_t n_labels,
    std::function<GLuint(const size_t)> get_label_tex_id) const
{
    constexpr unsigned int n_primitives = 1u;
    constexpr unsigned int n_indices = 4u;
    glBindVertexArray(vao_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    for (size_t i_label = 0u; i_label < n_labels; i_label++) {
        glBindTexture(GL_TEXTURE_2D, get_label_tex_id(i_label));
        glDrawElements(GL_QUADS, n_indices * n_primitives, GL_UNSIGNED_INT,
                        (GLvoid*)(i_label * sizeof(tiny_gl_text_renderer::quad_t)));
    }
    //glBindVertexArray(0); // Not really needed.
}

template class BufferSet<tiny_gl_text_renderer::vertex_colored_t>;
template class BufferSet<tiny_gl_text_renderer::vertex_textured_t>;

} // end of namespace tiny_graph_plot
