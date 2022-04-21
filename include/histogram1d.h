#pragma once

#include <limits>
#include <algorithm>

#include "drawable.h"

namespace tiny_graph_plot {

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
    {
    }
    virtual ~Histogram1d(void) {
        if (this->_points != nullptr) {
            delete[] this->_points;
        }
    }
public:
    void Init(const unsigned int nbins, const T xmin, const T xmax) {
        _n_bins = nbins;
        _x_min = xmin;
        _x_max = xmax;
        this->_size_info = SizeInfo(3 * _n_bins, 3 * _n_bins, 3 * _n_bins - 1, 0); //TODO
        _bins.resize(1 + _n_bins + 1); // underflow, data, overflow
        this->_points = new Vec2<T>[3 * _n_bins];
    }
    void SetUnderflowValue(const VALUETYPE value) {
        _bins[0] = value;
    }
    void SetOverflowValue(const VALUETYPE value) {
        _bins[_n_bins + 1] = value;
    }
    void SetBinValue(const unsigned int iBin, const VALUETYPE value) {
        _bins[iBin + 1] = value;
        const T bin_width = (_x_max - _x_min) / T(_n_bins);
        const T x = _x_min + (T(iBin) + T(0.5)) * bin_width;
        const T y = static_cast<T>(value);
        this->_points[iBin*3+0] = Vec2<T>(x - T(0.5) * bin_width, y);
        this->_points[iBin*3+1] = Vec2<T>(x,                      y);
        this->_points[iBin*3+2] = Vec2<T>(x + T(0.5) * bin_width, y);
    }
    //std::vector<VALUETYPE>& GetBinsToModify(void) { return _bins; }
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
private:
    unsigned int _n_bins; //!< Number of bins not including the underflow and the overflow bins
    T _x_min;
    T _x_max;
    std::vector<VALUETYPE> _bins; // [1+_n_bins+1]
};

template class Histogram1d<float, unsigned long>;
template class Histogram1d<double, unsigned long>;

typedef Histogram1d<float, unsigned long> Histogram1dF;
typedef Histogram1d<double, unsigned long> Histogram1dD;

} // end of namespace tiny_graph_plot
