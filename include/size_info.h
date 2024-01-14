#pragma once

namespace tiny_graph_plot
{

class SizeInfo
{
public:
    explicit SizeInfo() = default;
    explicit SizeInfo(const unsigned int n_v, const unsigned int n_m,
        const unsigned int n_w, const unsigned int n_tr)
    :   _n_v(n_v), _n_m(n_m), _n_w(n_w), _n_tr(n_tr) {}
    ~SizeInfo() = default;
    SizeInfo(const SizeInfo& other) = delete;
    SizeInfo(SizeInfo&& other) = default;
    SizeInfo& operator=(const SizeInfo& other) = delete;
    SizeInfo& operator=(SizeInfo&& other) = default;
public:
    SizeInfo& operator+=(const SizeInfo& rhs) {
        _n_v += rhs._n_v;
        _n_m += rhs._n_m;
        _n_w += rhs._n_w;
        _n_tr += rhs._n_tr;
        return *this;
    }
public:
    unsigned int _n_v = 0u;
    unsigned int _n_m = 0u;
    unsigned int _n_w = 0u;
    unsigned int _n_tr = 0u;
};

} // end of namespace tiny_graph_plot
