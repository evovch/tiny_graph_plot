#pragma once

//#include <limits>
#include <vector>
#include <type_traits>

#include "drawable.h"

namespace tiny_graph_plot
{

template<typename T> class GraphManager;

template<typename T, typename VALUETYPE>
class Histogram1d : public Drawable<T>
{
    static_assert(std::is_same<T, float>::value
               || std::is_same<T, double>::value, "");
    friend class GraphManager<T>;
private:
    explicit Histogram1d()
    :   Drawable<T>(),
        _n_bins(0),
        _x_min(0.0),
        _x_max(0.0) {}
    virtual ~Histogram1d() {
        if (this->points_ != nullptr) {
            delete[] this->points_;
        }
    }
    Histogram1d(const Histogram1d& other) = delete;
    Histogram1d(Histogram1d&& other) = delete;
    Histogram1d& operator=(const Histogram1d& other) = delete;
    Histogram1d& operator=(Histogram1d&& other) = delete;
public:
    void Init(const unsigned int nbins, const T xmin, const T xmax) {
        _n_bins = nbins;
        _x_min = xmin;
        _x_max = xmax;
        this->size_info_ = SizeInfo(3 * _n_bins, 3 * _n_bins, 3 * _n_bins - 1, 0); //TODO
        _bins.resize(1 + _n_bins + 1); // underflow, data, overflow
        this->points_ = new Vec2<T>[3 * _n_bins];
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
        this->points_[iBin*3+0] = Vec2<T>(x - T(0.5) * bin_width, y);
        this->points_[iBin*3+1] = Vec2<T>(x,                      y);
        this->points_[iBin*3+2] = Vec2<T>(x + T(0.5) * bin_width, y);
    }
    //std::vector<VALUETYPE>& GetBinsToModify() { return _bins; }
    void GenGauss(
        const unsigned int nbins, const T xmin, const T xmax, const T a, const T b, const T c);
private:
    unsigned int _n_bins; //!< Number of bins not including the underflow and the overflow bins
    T _x_min;
    T _x_max;
    std::vector<VALUETYPE> _bins; // [1+_n_bins+1]
};

template class Histogram1d<float, unsigned long>;
template class Histogram1d<double, unsigned long>;

using Histogram1dF = Histogram1d<float, unsigned long>;
using Histogram1dD = Histogram1d<double, unsigned long>;

} // end of namespace tiny_graph_plot

#include "histogram1d_inline.h"
