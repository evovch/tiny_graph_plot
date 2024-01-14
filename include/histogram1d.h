#pragma once

//#include <limits>
#include <cassert>
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
        n_bins_(0u),
        x_min_(T(0.0)),
        x_max_(T(0.0)) {}
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
        n_bins_ = nbins;
        x_min_ = xmin;
        x_max_ = xmax;
        this->size_info_ = SizeInfo(3u * n_bins_, 3u * n_bins_, 3u * n_bins_ - 1u, 0); //TODO
        bins_.resize(1u + n_bins_ + 1u); // underflow, data, overflow
        this->points_ = new Vec2<T>[3u * n_bins_];
    }
    void SetUnderflowValue(const VALUETYPE value) noexcept {
        bins_[0u] = value;
    }
    void SetOverflowValue(const VALUETYPE value) noexcept {
        bins_[n_bins_ + 1u] = value;
    }
    void SetBinValue(const unsigned int iBin, const VALUETYPE value) {
        assert(iBin < bins_.size());
        bins_[iBin + 1u] = value;
        const T bin_width = (x_max_ - x_min_) / T(n_bins_);
        const T x = x_min_ + (T(iBin) + T(0.5)) * bin_width;
        const T y = static_cast<T>(value);
        this->points_[iBin * 3u + 0u] = Vec2<T>(x - T(0.5) * bin_width, y);
        this->points_[iBin * 3u + 1u] = Vec2<T>(x,                      y);
        this->points_[iBin * 3u + 2u] = Vec2<T>(x + T(0.5) * bin_width, y);
    }
    //std::vector<VALUETYPE>& GetBinsToModify() { return bins_; }
    void GenGauss(
        const unsigned int nbins, const T xmin, const T xmax, const T a, const T b, const T c);
private:
    unsigned int n_bins_; //!< Number of bins not including the underflow and the overflow bins
    T x_min_;
    T x_max_;
    std::vector<VALUETYPE> bins_; // [1+n_bins_+1]
};

template class Histogram1d<float, unsigned long>;
template class Histogram1d<double, unsigned long>;

using Histogram1dF = Histogram1d<float, unsigned long>;
using Histogram1dD = Histogram1d<double, unsigned long>;

} // end of namespace tiny_graph_plot

#include "histogram1d_inline.h"
