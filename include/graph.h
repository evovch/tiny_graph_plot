#pragma once

//#include <cmath> // included through xy_range.h
//#include <limits>
#include <type_traits>

#include "drawable.h"

namespace tiny_graph_plot
{

using tiny_gl_text_renderer::Vec2;

template<typename T> class GraphManager;

template<typename T>
class Graph : public Drawable<T>
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
    friend class GraphManager<T>;
private:
    explicit Graph()
    :   Drawable<T>(),
        n_points_(0u),
        shared_points_(false) {}
    virtual ~Graph() {
        if (this->points_ != nullptr && !shared_points_) {
            free(this->points_);
        }
    }
    Graph(const Graph& other) = delete;
    Graph(Graph&& other) = delete;
    Graph& operator=(const Graph& other) = delete;
    Graph& operator=(Graph&& other) = delete;
public:
    /**
        Note that when setting the data of the graph by calling this method
        all the coodrinates in the input collection 'p_xy' must already be
        filled. In this method the graph calculates some values necessary
        for further visualization.
    */
    void SetSharedBuffer(const unsigned int p_size, Vec2<T>* const p_xy) {
        n_points_ = p_size;
        this->size_info_ = SizeInfo(p_size, p_size, p_size - 1u, 0u);
        shared_points_ = true;
        this->points_ = p_xy;
        this->CalculateRanges();
    }
    T Evaluate(const T x) const;
private:
    void CalculateRanges() const;
private:
    unsigned int n_points_; //!< Number of points
    bool shared_points_;
};

template class Graph<float>;
template class Graph<double>;

using GraphF = Graph<float>;
using GraphD = Graph<double>;

} // end of namespace tiny_graph_plot

#include "graph_inline.h"
