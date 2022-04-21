#pragma once

#include <limits>
#include <algorithm>

#include "drawable.h"

namespace tiny_graph_plot {

//using tiny_gl_text_renderer::color_t;

template<typename T, typename VALUETYPE>
class Histogram1d : public Drawable<T>
{
    template<typename T>
    friend class GraphManager;
private:
    Histogram1d(void) : Drawable<T>(),
        _n_bins(0),
        _x_min(0.0),
        _x_max(0.0)
        //_points(nullptr),
        //_size_info(0, 0, 0, 0),
        //_xy_range(0.0, 1.0, 0.0, 1.0)
    {
    }
    virtual ~Histogram1d(void) {
        if (this->_points) delete[] this->_points;
    }
public:
    void GenGauss(const unsigned int nbins, const T xmin, const T xmax,
        const T a, const T b, const T c) {
        _n_bins = nbins;
        _x_min = xmin;
        _x_max = xmax;
        _bins.resize(1 + _n_bins + 1); // underflow, data, overflow
        //this->_points.resize(3 * _n_bins);
        this->_points = new Vec2<T>[3 * _n_bins];
        _bins[0] = 0; // underflow
        for (unsigned int iBin = 0; iBin < _n_bins; iBin++) {
            _bins[iBin + 1] = 0; // data
        }
        _bins[_n_bins + 1] = 0; // overflow
        const T bin_width = (_x_max - _x_min) / T(_n_bins);
        const T k = -T(0.5) / (c * c);
        T y_min = std::numeric_limits<T>::max();
        T y_max = std::numeric_limits<T>::lowest();
        for (unsigned int iBin = 0; iBin < _n_bins; iBin++) {
            const T x = _x_min + (T(iBin) + T(0.5)) * bin_width;
            const T y = a * exp(k * (x - b) * (x - b));
            const VALUETYPE val = static_cast<VALUETYPE>(floor(y));
            _bins[iBin + 1] = val;
            this->_points[iBin*3+0] = Vec2<T>(x - T(0.5) * bin_width, y);
            this->_points[iBin*3+1] = Vec2<T>(x,                      y);
            this->_points[iBin*3+2] = Vec2<T>(x + T(0.5) * bin_width, y);
            y_min = std::min(y_min, floor(y));
            y_max = std::max(y_max, floor(y));
        }
        this->_size_info = SizeInfo(3 * _n_bins, 3 * _n_bins, 3 * _n_bins - 1, 0); //TODO
        this->_xy_range = XYrange<T>(_x_min, _x_max - _x_min, y_min, y_max - y_min);
    }
//    const Vec2<T>& GetPoint(const size_t idx) const { return _points[idx]; }
//    const SizeInfo& GetSizeInfo(void) const { return _size_info; }
//    const XYrange<T>& GetXYrange(void) const { return _xy_range; }
private:
    unsigned int _n_bins; //!< Number of bins not including the underflow and the overflow bins
    T _x_min;
    T _x_max;
    std::vector<VALUETYPE> _bins; // [1+_n_bins+1]
/*
    Vec2<T>* _points;
    SizeInfo _size_info;
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
//*/
};

template class Histogram1d<float, unsigned long>;
template class Histogram1d<double, unsigned long>;

typedef Histogram1d<float, unsigned long> Histogram1dF;
typedef Histogram1d<double, unsigned long> Histogram1dD;

} // end of namespace tiny_graph_plot
