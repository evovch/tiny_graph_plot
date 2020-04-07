#pragma once

#include <vector>

#include "graph.h"

namespace tiny_graph_plot {

template<typename T>
class GraphManager
{
public:
	GraphManager<T>(void) {}
	~GraphManager<T>(void) {
		for (Graph<T>* gr : _graphs) {
			delete gr;
		}
	}
	Graph<T>& CreateGraph(void) {
		Graph<T>* new_gr = new Graph<T>();
		_graphs.push_back(new_gr);
		return *new_gr;
	}
private:
	std::vector<Graph<T>*> _graphs;
};

template class GraphManager<float>;
template class GraphManager<double>;

extern GraphManager<float> global_graph_manager_float;
extern GraphManager<double> global_graph_manager_double;

} // end of namespace tiny_graph_plot
