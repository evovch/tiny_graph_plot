#pragma once

#include <algorithm>

namespace tiny_graph_plot
{

template<typename T, typename VALUETYPE>
inline void Histogram1d<T, VALUETYPE>::GenGauss(
    const unsigned int nbins, const T xmin, const T xmax, const T a, const T b, const T c)\
{
    n_bins_ = nbins;
    x_min_ = xmin;
    x_max_ = xmax;
    bins_.resize(1u + n_bins_ + 1u); // underflow, data, overflow
    //this->points_.resize(3u * n_bins_);
    this->points_ = new Vec2<T>[3u * n_bins_];
    bins_[0] = 0; // underflow
    for (unsigned int iBin = 0u; iBin < n_bins_; iBin++) {
        bins_[iBin + 1] = 0; // data
    }
    bins_[n_bins_ + 1] = 0; // overflow
    const T bin_width = (x_max_ - x_min_) / T(n_bins_);
    const T k = -T(0.5) / (c * c);
    T y_min = std::numeric_limits<T>::max();
    T y_max = std::numeric_limits<T>::lowest();
    for (unsigned int iBin = 0; iBin < n_bins_; iBin++) {
        const T x = x_min_ + (T(iBin) + T(0.5)) * bin_width;
        const T y = a * exp(k * (x - b) * (x - b));
        const VALUETYPE val = static_cast<VALUETYPE>(floor(y));
        bins_[iBin + 1] = val;
        this->points_[iBin*3+0] = Vec2<T>(x - T(0.5) * bin_width, y);
        this->points_[iBin*3+1] = Vec2<T>(x,                      y);
        this->points_[iBin*3+2] = Vec2<T>(x + T(0.5) * bin_width, y);
        y_min = std::min(y_min, std::floor(y));
        y_max = std::max(y_max, std::floor(y));
    }
    this->size_info_ = SizeInfo(3u * n_bins_, 3u * n_bins_, 3u * n_bins_ - 1u, 0); //TODO
    this->xy_range_ = XYrange<T>(x_min_, x_max_ - x_min_, y_min, y_max - y_min);
}

} // end of namespace tiny_graph_plot