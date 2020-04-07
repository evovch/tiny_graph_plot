#pragma once

// Uncomment to suppress all output which is
// written by the library into the console
// except critical error messages
#define SUPPRESS_ALL_OUTPUT

#include "tiny_gl_text_renderer/vec2.h"
#include "tiny_gl_text_renderer/colors.h"
#include "graph_manager.h"
#include "canvas_manager.h"

tiny_graph_plot::GraphManager<float> global_graph_manager_float;
tiny_graph_plot::GraphManager<double> global_graph_manager_double;
tiny_graph_plot::CanvasManager<float> global_canvas_manager_float;
tiny_graph_plot::CanvasManager<double> global_canvas_manager_double;
