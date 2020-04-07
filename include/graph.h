#pragma once

//#include <cmath> // included through xy_range.h
#include <limits>

#include "tiny_gl_text_renderer/data_types.h"
#include "tiny_gl_text_renderer/colors.h"
#include "size_info.h"
#include "xy_range.h"

namespace tiny_graph_plot {

using tiny_gl_text_renderer::color_t;
using tiny_gl_text_renderer::Vec2;

template<typename T>
class Graph
{
    template<typename T>
    friend class GraphManager;
private:
    Graph(void) :
        _n_p(0),
        _size_info(0, 0, 0, 0),
        _shared_points(false),
        _points(nullptr),
        _xy_range(0.0, 1.0, 0.0, 1.0) {
    }
    ~Graph(void) {
        if (_points != nullptr && !_shared_points) {
            free(_points);
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
        _size_info = SizeInfo(p_size, p_size, p_size - 1, 0);
        _shared_points = true;
        _points = p_xy;
        this->CalculateRanges();
    }
    T Evaluate(const T x) const {
        if (!_xy_range.IncludesX(x)) return std::numeric_limits<T>::quiet_NaN();

        const T xmin = _points[0].x();
        const T xmax = _points[_n_p - 1].x();
        if (x == xmin) return _points[0].y();
        if (x == xmax) return _points[_n_p - 1].y();
        // estimate the location of the needed segment
        // assuming uniform distribution of the samples
        const T t = (x - xmin) / (xmax - xmin);
        int idxl = static_cast<int>(std::floor(t * (double)(_n_p - 1)));
        while (!((x >= _points[idxl].x()) && (x <= _points[idxl + 1].x()))) {
            if (x < _points[idxl].x()) {
                idxl--;
            } else {
                idxl++;
            }
            if (idxl < 0 || idxl >(int)_n_p - 1) return std::numeric_limits<T>::quiet_NaN();
        }
        const T p = (x - _points[idxl].x()) / (_points[idxl + 1].x() - _points[idxl].x());
        const T y = p * (_points[idxl + 1].y() - _points[idxl].y()) + _points[idxl].y();
        return y;
    }
    const SizeInfo& GetSizeInfo(void) const { return _size_info; }
    const XYrange<T>& GetXYrange(void) const { return _xy_range; }
    const Vec2<T>& GetPoint(const size_t idx) const { return _points[idx]; }
private:
    void CalculateRanges(void) const {
        unsigned int start_i = 0;
        bool start_i_found = false;
        for (start_i = 0; start_i < _n_p; start_i++) {
            if (std::isfinite(_points[start_i].x()) &&
                std::isfinite(_points[start_i].y())) {
                start_i_found = true;
                break;
            }
        }
        unsigned int start_j = 0;
        bool start_j_found = false;
        for (start_j = 0; start_j < _n_p; start_j++) {
            if (std::isfinite(_points[start_j].x()) &&
                std::isfinite(_points[start_j].y())) {
                start_j_found = true;
                break;
            }
        }

        if (!start_i_found || !start_j_found) {
            fprintf(stderr, "ERROR: something is definitely wrong with the input data.\n");
            return;
        }

        _xy_range = XYrange<T>(
            _points[start_i].x(), _points[start_j].x() - _points[start_i].x(),
            _points[start_i].y(), _points[start_j].y() - _points[start_i].y());
        for (unsigned int i = 0; i < _n_p; i++) {
            _xy_range.Include(_points[i]);
        }
        _xy_range.FixDegenerateCases();
    }
private:
    unsigned int _n_p; //!< Number of points
    SizeInfo _size_info;
    bool _shared_points;
    Vec2<T>* _points;
    mutable XYrange<T> _xy_range;
public: // visual parameters
    void SetColor(const color_t& color)  { _color = color; }
    void SetMarkerSize(const float size) { _marker_size = size; }
    void SetLineWidth(const float width) { _line_width = width; }
    const color_t& GetColor(void) const   { return _color; }
    const float GetMarkerSize(void) const { return _marker_size; }
    const float GetLineWidth(void) const  { return _line_width; }
private: // visual parameters
    color_t _color = tiny_gl_text_renderer::colors::blue;
    float _marker_size = 5.0f;
    float _line_width = 3.0f;
};

template class Graph<float>;
template class Graph<double>;

typedef Graph<float> GraphF;
typedef Graph<double> GraphD;

} // end of namespace tiny_graph_plot
