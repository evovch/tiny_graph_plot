#pragma once

namespace tiny_graph_plot {

// ===============================================================================
const char* canvas_sel_q_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
uniform mat4 screen2viewport;
uniform mat4 viewport2clip;
uniform mat4 screen2clip;
uniform mat4 visrange2clip;
out vec4 color;
void main() {
    gl_Position = visrange2clip * in_position;
    color = vec4(in_color.xyz, 0.2f);
}
)";
const char* canvas_sel_q_fp_source = R"(#version 400
in vec4 color;
layout(location = 0) out vec4 out_color;
void main() {
    out_color = color;
}
)";
// ===============================================================================
const char* canvas_onscr_q_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
//uniform mat4 screen2viewport;
//uniform mat4 viewport2clip;
uniform mat4 screen2clip;
//uniform mat4 visrange2clip;
out vec4 color;
void main() {
    gl_Position = screen2clip * in_position;
    color = in_color;
}
)";
const char* canvas_onscr_q_fp_source = R"(#version 400
in vec4 color;
uniform vec4 drawcolor;
layout(location = 0) out vec4 out_color;
void main() {
    //out_color = color;
    out_color = drawcolor;
}
)";
// ===============================================================================
const char* canvas_w_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
uniform mat4 screen2viewport;
uniform mat4 viewport2clip;
uniform mat4 screen2clip;
uniform mat4 visrange2clip;
out vec4 color;
void main() {
    gl_Position = visrange2clip * in_position;
    color = in_color;
}
)";
const char* canvas_w_fp_source = R"(#version 400
in vec4 color;
layout(location = 0) out vec4 out_color;
void main() {
    out_color = color;
}
)";
// ===============================================================================
const char* canvas_onscr_w_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
//uniform mat4 screen2viewport;
//uniform mat4 viewport2clip;
uniform mat4 screen2clip;
//uniform mat4 visrange2clip;
out vec4 color;
void main() {
    gl_Position = screen2clip * in_position;
    color = in_color;
}
)";
const char* canvas_onscr_w_fp_source = R"(#version 400
in vec4 color;
layout(location = 0) out vec4 out_color;
void main() {
    out_color = color;
}
)";
// ===============================================================================
const char* canvas_m_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
uniform mat4 screen2viewport;
uniform mat4 viewport2clip;
uniform mat4 screen2clip;
uniform mat4 visrange2clip;
out vec4 color;
void main() {
    gl_Position = visrange2clip * in_position;
    color = in_color;
}
)";
const char* canvas_m_fp_source = R"(#version 400
in vec4 color;
layout(location = 0) out vec4 out_color;
void main() {
    out_color = color;
}
)";
// ===============================================================================
const char* canvas_c_vp_source = R"(#version 400
layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
//uniform mat4 screen2viewport;
//uniform mat4 viewport2clip;
//uniform mat4 screen2clip;
uniform mat4 visrange2clip;
out vec4 color;
void main() {
    gl_Position = visrange2clip * in_position;
    color = in_color;
}
)";
const char* canvas_c_gp_source = R"(#version 400
layout(points) in;
layout(line_strip, max_vertices=9) out;
in vec4 color[];
uniform mat4 viewport2clip;
uniform float circle_r;
out vec4 geom_color;
#define M_PI 3.14159265358979323846
void main() {
    for (int i=0; i<9; i++) {
        // max_vertices=9; (max_vertices-1)/2 = 4, hence M_PI / 4.0f;
        float alpha = float(i) * M_PI / 4.0f;
        vec4 offset = vec4(circle_r * cos(alpha), circle_r * sin(alpha), 0.0f, 0.0f);
        gl_Position = gl_in[0].gl_Position + viewport2clip * offset;
        geom_color = color[0];
        EmitVertex();
    }
    EndPrimitive();
}
)";
const char* canvas_c_fp_source = R"(#version 400
in vec4 geom_color;
layout(location = 0) out vec4 out_color;
void main() {
    out_color = geom_color;
}
)";
// ===============================================================================

} // end of namespace tiny_graph_plot
