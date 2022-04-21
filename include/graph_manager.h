#pragma once

#include <vector>

#include "graph.h"
#include "histogram1d.h"

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
		for (Histogram1d<T, unsigned long>* h : _histograms) {
			delete h;
		}
	}
	Graph<T>& CreateGraph(void) {
		Graph<T>* new_gr = new Graph<T>();
		_graphs.push_back(new_gr);
		return *new_gr;
	}
	Histogram1d<T, unsigned long>& CreateHistogram1d(void) {
		Histogram1d<T, unsigned long>* new_histo = new Histogram1d<T, unsigned long>();
		_histograms.push_back(new_histo);
		return *new_histo;
	}
private:
	std::vector<Graph<T>*> _graphs;
	std::vector<Histogram1d<T, unsigned long>*> _histograms;
};

template class GraphManager<float>;
template class GraphManager<double>;

extern GraphManager<float> global_graph_manager_float;
extern GraphManager<double> global_graph_manager_double;

} // end of namespace tiny_graph_plot
