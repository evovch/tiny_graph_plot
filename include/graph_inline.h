#pragma once

#include <cmath>

namespace tiny_graph_plot
{

template<typename T>
inline T Graph<T>::Evaluate(const T x) const
{
    if (!this->xy_range_.IncludesX(x)) return std::numeric_limits<T>::quiet_NaN();

    const T xmin = this->points_[0].x();
    const T xmax = this->points_[_n_p - 1].x();
    if (x == xmin) return this->points_[0].y();
    if (x == xmax) return this->points_[_n_p - 1].y();
    // estimate the location of the needed segment
    // assuming uniform distribution of the samples
    const T t = (x - xmin) / (xmax - xmin);
    int idxl = static_cast<int>(std::floor(t * (double)(_n_p - 1)));
    while (!((x >= this->points_[idxl].x()) && (x <= this->points_[idxl + 1].x()))) {
        if (x < this->points_[idxl].x()) {
            idxl--;
        } else {
            idxl++;
        }
        if (idxl < 0 || idxl >(int)_n_p - 1) return std::numeric_limits<T>::quiet_NaN();
    }
    const T p = (x - this->points_[idxl].x()) / (this->points_[idxl + 1].x() - this->points_[idxl].x());
    const T y = p * (this->points_[idxl + 1].y() - this->points_[idxl].y()) + this->points_[idxl].y();
    return y;
}

template<typename T>
inline void Graph<T>::CalculateRanges() const
{
    unsigned int start_i = 0;
    bool start_i_found = false;
    for (start_i = 0; start_i < _n_p; start_i++) {
        if (std::isfinite(this->points_[start_i].x()) &&
            std::isfinite(this->points_[start_i].y())) {
            start_i_found = true;
            break;
        }
    }
    unsigned int start_j = 0;
    bool start_j_found = false;
    for (start_j = 0; start_j < _n_p; start_j++) {
        if (std::isfinite(this->points_[start_j].x()) &&
            std::isfinite(this->points_[start_j].y())) {
            start_j_found = true;
            break;
        }
    }

    if (!start_i_found || !start_j_found) {
        fprintf(stderr, "ERROR: something is definitely wrong with the input data.\n");
        return;
    }

    this->xy_range_ = XYrange<T>(
        this->points_[start_i].x(), this->points_[start_j].x() - this->points_[start_i].x(),
        this->points_[start_i].y(), this->points_[start_j].y() - this->points_[start_i].y());
    for (unsigned int i = 0; i < _n_p; i++) {
        this->xy_range_.Include(this->points_[i]);
    }
    this->xy_range_.FixDegenerateCases();
}

} // end of namespace tiny_graph_plot