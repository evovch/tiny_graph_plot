#pragma once

//#include <cmath> // included through xy_range.h
#include <limits>

#include "drawable.h"

namespace tiny_graph_plot {

using tiny_gl_text_renderer::Vec2;

template<typename T>
class Graph : public Drawable<T>
{
    template<typename T>
    friend class GraphManager;
private:
    Graph(void) : Drawable<T>(),
        _n_p(0),
        _shared_points(false)
    {
    }
    virtual ~Graph(void) {
        if (this->_points != nullptr && !_shared_points) {
            free(this->_points);
        }
    }
public:
    /**
        Note that when setting the data of the graph by calling this method
        all the coodrinates in the input collection 'p_xy' must already be
        filled. In this method the graph calculates some values necessary
        for further visualization.
    */
    void SetSharedBuffer(const unsigned int p_size, Vec2<T>* const p_xy) {
        _n_p = p_size;
        this->_size_info = SizeInfo(p_size, p_size, p_size - 1, 0);
        _shared_points = true;
        this->_points = p_xy;
        this->CalculateRanges();
    }
    T Evaluate(const T x) const {
        if (!this->_xy_range.IncludesX(x)) return std::numeric_limits<T>::quiet_NaN();

        const T xmin = this->_points[0].x();
        const T xmax = this->_points[_n_p - 1].x();
        if (x == xmin) return this->_points[0].y();
        if (x == xmax) return this->_points[_n_p - 1].y();
        // estimate the location of the needed segment
        // assuming uniform distribution of the samples
        const T t = (x - xmin) / (xmax - xmin);
        int idxl = static_cast<int>(std::floor(t * (double)(_n_p - 1)));
        while (!((x >= this->_points[idxl].x()) && (x <= this->_points[idxl + 1].x()))) {
            if (x < this->_points[idxl].x()) {
                idxl--;
            } else {
                idxl++;
            }
            if (idxl < 0 || idxl >(int)_n_p - 1) return std::numeric_limits<T>::quiet_NaN();
        }
        const T p = (x - this->_points[idxl].x()) / (this->_points[idxl + 1].x() - this->_points[idxl].x());
        const T y = p * (this->_points[idxl + 1].y() - this->_points[idxl].y()) + this->_points[idxl].y();
        return y;
    }
private:
    void CalculateRanges(void) const {
        unsigned int start_i = 0;
        bool start_i_found = false;
        for (start_i = 0; start_i < _n_p; start_i++) {
            if (std::isfinite(this->_points[start_i].x()) &&
                std::isfinite(this->_points[start_i].y())) {
                start_i_found = true;
                break;
            }
        }
        unsigned int start_j = 0;
        bool start_j_found = false;
        for (start_j = 0; start_j < _n_p; start_j++) {
            if (std::isfinite(this->_points[start_j].x()) &&
                std::isfinite(this->_points[start_j].y())) {
                start_j_found = true;
                break;
            }
        }

        if (!start_i_found || !start_j_found) {
            fprintf(stderr, "ERROR: something is definitely wrong with the input data.\n");
            return;
        }

        this->_xy_range = XYrange<T>(
            this->_points[start_i].x(), this->_points[start_j].x() - this->_points[start_i].x(),
            this->_points[start_i].y(), this->_points[start_j].y() - this->_points[start_i].y());
        for (unsigned int i = 0; i < _n_p; i++) {
            this->_xy_range.Include(this->_points[i]);
        }
        this->_xy_range.FixDegenerateCases();
    }
private:
    unsigned int _n_p; //!< Number of points
    bool _shared_points;
};

template class Graph<float>;
template class Graph<double>;

typedef Graph<float> GraphF;
typedef Graph<double> GraphD;

} // end of namespace tiny_graph_plot
