#pragma once

#include <string>

typedef unsigned int GLuint;

namespace tiny_graph_plot
{

class BufferSet
{
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
	void SendVertices(const unsigned int n_vert, const void* const data) const;
	void SendIndicesWires(const unsigned int n_primitives, const void* const data) const;
	void SendIndicesMarkers(const unsigned int n_primitives, const void* const data) const;
	void DrawWires(const unsigned int n_primitives) const;
	void DrawMarkers(const unsigned int n_primitives) const;
private:
	const std::string name_;
	GLuint vao_;
	GLuint vbo_;
	GLuint ibo_;
};

} // end of namespace tiny_graph_plot
