#pragma once

namespace tiny_graph_plot {

class SizeInfo {
public:
    unsigned int _n_v;
    unsigned int _n_m;
    unsigned int _n_w;
    unsigned int _n_tr;
public:
    explicit SizeInfo(void) :
        _n_v(0), _n_m(0), _n_w(0), _n_tr(0) {}
    explicit SizeInfo(const unsigned int n_v, const unsigned int n_m,
        const unsigned int n_w, const unsigned int n_tr) :
        _n_v(n_v), _n_m(n_m), _n_w(n_w), _n_tr(n_tr) {}
    ~SizeInfo(void) {}
    __forceinline SizeInfo& operator=(const SizeInfo& rhs) {
        _n_v = rhs._n_v;
        _n_m = rhs._n_m;
        _n_w = rhs._n_w;
        _n_tr = rhs._n_tr;
        return *this;
    }
    __forceinline SizeInfo& operator+=(const SizeInfo& rhs) {
        _n_v += rhs._n_v;
        _n_m += rhs._n_m;
        _n_w += rhs._n_w;
        _n_tr += rhs._n_tr;
        return *this;
    }
};

} // end of namespace tiny_graph_plot
