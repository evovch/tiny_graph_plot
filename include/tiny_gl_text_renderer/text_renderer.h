#pragma once

#include <vector>

#include "data_types.h"
#include "mat4.h"
#include "label.h"

namespace tiny_gl_text_renderer
{

typedef unsigned int GLuint;

class TextRenderer
{
public:
    TextRenderer(void);
    ~TextRenderer(void);
public:
    void SetCanvasSize(int w, int h) { _w = w; _h = h; }
    void UpdateScreenToClipMatrix(void);
    void FirstReshape(int w, int h);
    void Reshape(int w, int h);
    void Draw(void) const;
    void DrawSingle(const size_t i_label) const;
    size_t AddLabel(const char* string, const int x, const int y,
        const color_t& color, const float scaling = 1.0f,
        const float angle = 0.0f);
    const std::string& GetLabelString(const size_t i_label);
    void UpdateLabel(const char* string, const size_t i_label);
    void UpdatePosition(const int x, const int y, const size_t i_label);
    void UpdatePositionX(const int x, const size_t i_label);
    void UpdatePositionY(const int y, const size_t i_label);
    void UpdateRotation(const float angle, const size_t i_label);
    void AllocateVerticesAndQuadsMemory(void);
    void RecalculateVertices(void);
    void RecalculateVerticesSingle(const size_t i_label);
    void SendToGPU(void) const;
    void SendToGPUverticesSingle(const size_t i_label) const;
    void SendToGPUtextureSingle(const size_t i_label) const;
private:
    unsigned int _w;
    unsigned int _h;
    GLuint _progID;
    GLuint _vaoID;
    GLuint _vboID;
    GLuint _iboID;
    GLuint _s2c_unif;
    Mat4f _screen_to_clip;
private:
    /*static*/ GLuint _labels_counter = 0;
    std::vector<Label> _labels;
    std::vector<vertex_textured_t> _vertices;
    std::vector<quad_t> _quads;
};

} // end of namespace tiny_gl_text_renderer
