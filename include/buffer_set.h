#pragma once

#include <functional>
#include <string>
#include <type_traits>

#include "tiny_gl_text_renderer/data_types.h"

typedef unsigned int GLuint;

namespace tiny_graph_plot
{

template<typename VERTEX_TYPE>
class BufferSet
{
	static_assert(std::is_same<VERTEX_TYPE, tiny_gl_text_renderer::vertex_colored_t>::value
		       || std::is_same<VERTEX_TYPE, tiny_gl_text_renderer::vertex_textured_t>::value);
public:
	explicit BufferSet(const char* const name);
	~BufferSet();
	BufferSet(const BufferSet& other) = delete;
	BufferSet(BufferSet&& other) = delete;
	BufferSet& operator=(const BufferSet& other) = delete;
	BufferSet& operator=(BufferSet&& other) = delete;
public:
	void Generate();
	void Allocate(const unsigned int n_vert, const void* const data = nullptr) const;
	void SendVertices(const unsigned int n_vert, const void* const data,
	                  const unsigned int offset = 0u) const;
	template<typename PRIMITIVE_TYPE>
	void SendIndices(const unsigned int n_primitives, const PRIMITIVE_TYPE* const data) const;
	void DrawQuads(const unsigned int n_primitives) const;
	void DrawWires(const unsigned int n_primitives) const;
	void DrawMarkers(const unsigned int n_primitives) const;
	void DrawQuadsWithTextures(const size_t n_labels, std::function<GLuint(const size_t)> get_label_tex_id) const;
private:
	const std::string name_;
	GLuint vao_;
	GLuint vbo_;
	GLuint ibo_;
};

} // end of namespace tiny_graph_plot
