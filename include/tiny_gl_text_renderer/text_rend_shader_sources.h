#pragma once

namespace tiny_gl_text_renderer {

const char* text_rend_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_tex_coord;
uniform mat4 screen2clip;
out vec2 v_tex_coord;
void main() {
    gl_Position = screen2clip * in_position;
    v_tex_coord = in_tex_coord;
}
)";

const char* text_rend_fp_source = R"(#version 400
in vec2 v_tex_coord;
layout(location = 0) out vec4 f_color;
uniform sampler2D textureSampler;
void main() {
    f_color = texture(textureSampler, v_tex_coord);
}
)";

} // end of namespace tiny_gl_text_renderer
