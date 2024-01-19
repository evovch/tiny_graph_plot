#include "tiny_gl_text_renderer/text_renderer.h"

#include "GL/glew.h"

#include "tiny_gl_text_renderer/text_rend_shader_sources.h"
#include "tiny_gl_text_renderer/mat3.h"

namespace tiny_gl_text_renderer
{

/*static*/
/*GLuint TextRenderer::_labels_counter = 0;*/

TextRenderer::TextRenderer()
:   prog_text_("prog_text")
{
    // Buffers.
    glGenVertexArrays(1, &_vaoID);
    glGenBuffers(1, &_vboID);
    glGenBuffers(1, &_iboID);
    // Programs.
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Init program text");
    prog_text_.Generate(text_rend_vp_source, nullptr, text_rend_fp_source);
    glPopDebugGroup();
}

TextRenderer::~TextRenderer()
{
    // Buffers.
    glDeleteVertexArrays(1, &_vaoID);
    glDeleteBuffers(1, &_vboID);
    glDeleteBuffers(1, &_iboID);
    // Textures.
    for (const Label& label : _labels) {
        glDeleteTextures(1, &label.tex_id_);
    }
}

void TextRenderer::UpdateScreenToClipMatrix()
{
    _screen_to_clip.Set(
         2.0f / (float)_w, 0.0f,             0.0f, 0.0f,
         0.0f,             2.0f / (float)_h, 0.0f, 0.0f,
         0.0f,             0.0f,             1.0f, 0.0f,
        -1.0f,            -1.0f,             0.0f, 1.0f);
    prog_text_.CommitCamera1(Mat4f(), Mat4f(), _screen_to_clip);
}

void TextRenderer::FirstReshape(int w, int h)
{
    this->SetCanvasSize(w, h);
    this->AllocateVerticesAndQuadsMemory();
    this->SendToGPU();
    this->UpdateScreenToClipMatrix();
}

void TextRenderer::Reshape(int w, int h)
{
    this->SetCanvasSize(w, h);
    this->RecalculateVertices();
    this->SendToGPU();
    this->UpdateScreenToClipMatrix();
}

void TextRenderer::Draw() const
{
    prog_text_.Use();
    constexpr unsigned int n_quads = 1u;
    glBindVertexArray(_vaoID);
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID);
        size_t i_label = 0u;
        for (const Label& label : _labels) {
            glBindTexture(GL_TEXTURE_2D, label.tex_id_);
            glDrawElements(GL_QUADS, 4u * n_quads, GL_UNSIGNED_INT,
                           (GLvoid*)(i_label * sizeof(quad_t)));
            i_label++;
        }
    }
    //glBindVertexArray(0); // Not really needed.
}

size_t TextRenderer::AddLabel(const char* string, const int x, const int y,
    const color_t& color, const float scaling, const float angle)
{
    _labels.emplace_back(string, x, y, color, scaling, angle);
    glGenTextures(1, &_labels.back().tex_id_);
    glBindTexture(GL_TEXTURE_2D, _labels.back().tex_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _labels_counter++;
    return (size_t)(_labels_counter - 1); // not nice but should work
}

const std::string& TextRenderer::GetLabelString(const size_t i_label)
{
    const Label& label = _labels.at(i_label);
    return label.GetString();
}

void TextRenderer::UpdateLabel(const char* string, const size_t i_label)
{
    Label& label = _labels.at(i_label);
    label.UpdateString(string);
    this->RecalculateVerticesSingle(i_label);
    this->SendToGPUverticesSingle(i_label);
    this->SendToGPUtextureSingle(i_label);
}

void TextRenderer::UpdatePosition(const int x, const int y, const size_t i_label)
{
    Label& label = _labels.at(i_label);
    label.UpdatePosition(x, y);
    this->RecalculateVerticesSingle(i_label);
    this->SendToGPUverticesSingle(i_label);
}

void TextRenderer::UpdatePositionX(const int x, const size_t i_label)
{
    Label& label = _labels.at(i_label);
    label.UpdatePositionX(x);
    this->RecalculateVerticesSingle(i_label);
    this->SendToGPUverticesSingle(i_label);
}

void TextRenderer::UpdatePositionY(const int y, const size_t i_label)
{
    Label& label = _labels.at(i_label);
    label.UpdatePositionY(y);
    this->RecalculateVerticesSingle(i_label);
    this->SendToGPUverticesSingle(i_label);
}

void TextRenderer::UpdateRotation(const float angle, const size_t i_label)
{
    Label& label = _labels.at(i_label);
    label.UpdateRotation(angle);
    this->RecalculateVerticesSingle(i_label);
    this->SendToGPUverticesSingle(i_label);
}

void TextRenderer::AllocateVerticesAndQuadsMemory()
{
    _vertices.reserve(4 * _labels_counter);

    size_t i_label = 0u;
    for (const Label& label : _labels) {
        const size_t& tex_w = label.GetTexW();
        const size_t& tex_h = label.GetTexH();
        const Mat3f m = label.GetMatrix();
        const Vec3f v0c = m * Vec3f(0.0f, 0.0f, 1.0f);
        const Vec3f v1c = m * Vec3f((float)tex_w, 0.0f, 1.0f);
        const Vec3f v2c = m * Vec3f((float)tex_w, (float)tex_h, 1.0f);
        const Vec3f v3c = m * Vec3f(0.0f, (float)tex_h, 1.0f);

        _vertices.emplace_back(v0c.x(), (float)_h - v0c.y(), 0.0f, 1.0f, 0.0f, 0.0f);
        _vertices.emplace_back(v1c.x(), (float)_h - v1c.y(), 0.0f, 1.0f, 1.0f, 0.0f);
        _vertices.emplace_back(v2c.x(), (float)_h - v2c.y(), 0.0f, 1.0f, 1.0f, 1.0f);
        _vertices.emplace_back(v3c.x(), (float)_h - v3c.y(), 0.0f, 1.0f, 0.0f, 1.0f);
        _quads.emplace_back((unsigned int)i_label * 4u + 0u,
                            (unsigned int)i_label * 4u + 1u,
                            (unsigned int)i_label * 4u + 2u,
                            (unsigned int)i_label * 4u + 3u);
        i_label++;
    }
}

void TextRenderer::RecalculateVertices()
{
    size_t i_label = 0u;
    for (const Label& label : _labels) {
        (void)label; // unused iterator variable.
        this->RecalculateVerticesSingle(i_label);
        i_label++;
    }
}

void TextRenderer::RecalculateVerticesSingle(const size_t i_label)
{
    using v_str_t = vertex_textured_t;
    const Label& label = _labels.at(i_label);

    const size_t& tex_w = label.GetTexW();
    const size_t& tex_h = label.GetTexH();
    const Mat3f m = label.GetMatrix();
    const Vec3f v0c = m * Vec3f(0.0f, 0.0f, 1.0f);
    const Vec3f v1c = m * Vec3f((float)tex_w, 0.0f, 1.0f);
    const Vec3f v2c = m * Vec3f((float)tex_w, (float)tex_h, 1.0f);
    const Vec3f v3c = m * Vec3f(0.0f, (float)tex_h, 1.0f);

    v_str_t& va0 = _vertices.at(i_label * 4 + 0);
    v_str_t& va1 = _vertices.at(i_label * 4 + 1);
    v_str_t& va2 = _vertices.at(i_label * 4 + 2);
    v_str_t& va3 = _vertices.at(i_label * 4 + 3);
    va0 = v_str_t(v0c.x(), (float)_h - v0c.y(), 0.0f, 1.0f, 0.0f, 0.0f);
    va1 = v_str_t(v1c.x(), (float)_h - v1c.y(), 0.0f, 1.0f, 1.0f, 0.0f);
    va2 = v_str_t(v2c.x(), (float)_h - v2c.y(), 0.0f, 1.0f, 1.0f, 1.0f);
    va3 = v_str_t(v3c.x(), (float)_h - v3c.y(), 0.0f, 1.0f, 0.0f, 1.0f);
}

void TextRenderer::SendToGPU() const
{
    using v_str_t = vertex_textured_t;
    // Vertices.
    const v_str_t* const vertices = _vertices.data();
    glBindVertexArray(_vaoID);
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vboID);
        glBufferData(GL_ARRAY_BUFFER, _labels_counter * 4u * sizeof(v_str_t),
            vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(v_str_t),
            (void*)offsetof(v_str_t, coords_));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v_str_t),
            (void*)offsetof(v_str_t, tex_coords_));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    //glBindVertexArray(0); // Not really needed.
    // Indices.
    const quad_t* const quads = _quads.data();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _labels_counter * 1u * sizeof(quad_t),
        quads, GL_STATIC_DRAW);
    // Textures.
    size_t i_label = 0u;
    for (const Label& label : _labels) {
        (void)label; // unused iterator variable.
        this->SendToGPUtextureSingle(i_label);
        i_label++;
    }
}

void TextRenderer::SendToGPUverticesSingle(const size_t i_label) const
{
    // Vertices.
    const vertex_textured_t* const vertices = _vertices.data() + i_label * 4u;
    glBindVertexArray(_vaoID);
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vboID);
        glBufferSubData(GL_ARRAY_BUFFER, i_label * 4u * sizeof(vertex_textured_t),
            4u * sizeof(vertex_textured_t), vertices);
    }
    //glBindVertexArray(0); // Not really needed.
}

void TextRenderer::SendToGPUtextureSingle(const size_t i_label) const
{
    // Textures.
    const Label& label = _labels.at(i_label);
    const size_t& tex_w = label.GetTexW();
    const size_t& tex_h = label.GetTexH();
    const float* const tex_data = label.GetTexData();
    glBindTexture(GL_TEXTURE_2D, label.tex_id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)tex_w, (GLsizei)tex_h, 0,
        GL_RGBA, GL_FLOAT, tex_data);
}

} // end of namespace tiny_gl_text_renderer
