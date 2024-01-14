#pragma once

#include <vector>

#include "canvas.h"

namespace tiny_graph_plot
{

template<typename T>
class CanvasManager
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
	explicit CanvasManager();
	~CanvasManager();
    CanvasManager(const CanvasManager& other) = delete;
    CanvasManager(CanvasManager&& other) = delete;
    CanvasManager& operator=(const CanvasManager& other) = delete;
    CanvasManager& operator=(CanvasManager&& other) = delete;
public:
	Canvas<T>& CreateCanvas(const char* name,
		const unsigned int w = 800, const unsigned int h = 600,
		const unsigned int x = 50, const unsigned int y = 50);
	void WaitForTheWindowsToClose();
private:
	std::vector<Canvas<T>*> canvases_;
	bool glew_initialized_ = false;
};

extern CanvasManager<float> global_canvas_manager_float;
extern CanvasManager<double> global_canvas_manager_double;

} // end of namespace tiny_graph_plot
