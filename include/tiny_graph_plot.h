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

// ===========================================================================
// Use either this to work with single precision
tiny_graph_plot::GraphManager<float>& graph_manager = global_graph_manager_float;
tiny_graph_plot::CanvasManager<float>& canvas_manager = global_canvas_manager_float;
typedef tiny_graph_plot::Graph<float> Graph;
typedef tiny_graph_plot::Canvas<float> Canvas;
typedef tiny_graph_plot::Vec2<float> Vec2;
// or this to work with double precision
//tiny_graph_plot::GraphManager<double>& graph_manager = global_graph_manager_double;
//tiny_graph_plot::CanvasManager<double>& canvas_manager = global_canvas_manager_double;
//typedef tiny_graph_plot::Graph<double> Graph;
//typedef tiny_graph_plot::Canvas<double> Canvas;
//typedef tiny_graph_plot::Vec2<double> Vec2;
// ===========================================================================
