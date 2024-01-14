#pragma once

#include <algorithm>

namespace tiny_graph_plot
{

template<typename T, typename VALUETYPE>
inline void Histogram1d<T, VALUETYPE>::GenGauss(
    const unsigned int nbins, const T xmin, const T xmax, const T a, const T b, const T c)\
{
    _n_bins = nbins;
    _x_min = xmin;
    _x_max = xmax;
    _bins.resize(1 + _n_bins + 1); // underflow, data, overflow
    //this->points_.resize(3 * _n_bins);
    this->points_ = new Vec2<T>[3 * _n_bins];
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
        this->points_[iBin*3+0] = Vec2<T>(x - T(0.5) * bin_width, y);
        this->points_[iBin*3+1] = Vec2<T>(x,                      y);
        this->points_[iBin*3+2] = Vec2<T>(x + T(0.5) * bin_width, y);
        y_min = std::min(y_min, std::floor(y));
        y_max = std::max(y_max, std::floor(y));
    }
    this->size_info_ = SizeInfo(3 * _n_bins, 3 * _n_bins, 3 * _n_bins - 1, 0); //TODO
    this->xy_range_ = XYrange<T>(_x_min, _x_max - _x_min, y_min, y_max - y_min);
}

} // end of namespace tiny_graph_plot