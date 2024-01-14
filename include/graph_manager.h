#pragma once

#include <vector>

#include "graph.h"
#include "histogram1d.h"

namespace tiny_graph_plot
{

template<typename T>
class GraphManager
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
public:
	explicit GraphManager<T>() = default;
	~GraphManager<T>() {
		for (Graph<T>* gr : graphs_) {
			delete gr;
		}
		for (Histogram1d<T, unsigned long>* h : histograms_) {
			delete h;
		}
	}
    GraphManager(const GraphManager& other) = delete;
    GraphManager(GraphManager&& other) = delete;
    GraphManager& operator=(const GraphManager& other) = delete;
    GraphManager& operator=(GraphManager&& other) = delete;
public:
	Graph<T>& CreateGraph() {
		Graph<T>* new_gr = new Graph<T>();
		graphs_.push_back(new_gr);
		return *new_gr;
	}
	Histogram1d<T, unsigned long>& CreateHistogram1d() {
		Histogram1d<T, unsigned long>* new_histo = new Histogram1d<T, unsigned long>();
		histograms_.push_back(new_histo);
		return *new_histo;
	}
private:
	std::vector<Graph<T>*> graphs_;
	std::vector<Histogram1d<T, unsigned long>*> histograms_;
};

template class GraphManager<float>;
template class GraphManager<double>;

extern GraphManager<float> global_graph_manager_float;
extern GraphManager<double> global_graph_manager_double;

} // end of namespace tiny_graph_plot
