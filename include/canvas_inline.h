#pragma once

namespace tiny_graph_plot
{

// Transformation matrices are stored using single precision floats,
// thus performing transformations in double precision make no sense.
template<typename T>
inline Vec4f Canvas<T>::TransformToVisrange(const double xs, const double ys,
    Vec4f* const o_in_clip_space, Vec4f* const o_in_viewport_space) const
{
    const Vec4f ps((float)xs, (float)ys, 0.0f, 1.0f);
    return this->TransformToVisrange(ps, o_in_clip_space, o_in_viewport_space);
}

template<typename T>
inline Vec4f Canvas<T>::TransformToVisrange(const Vec4f& in_screen_space,
    Vec4f* const o_in_clip_space, Vec4f* const o_in_viewport_space) const
{
    const Vec4f in_viewport_space = _screen_to_viewport * in_screen_space;
    const Vec4f in_clip_space = _viewport_to_clip * in_viewport_space;
    if (o_in_viewport_space != nullptr) {
        *o_in_viewport_space = in_viewport_space;
    }
    if (o_in_clip_space != nullptr) {
        *o_in_clip_space = in_clip_space;
    }
    return _clip_to_visrange * in_clip_space;
}

//template<typename T>
//inline Vec4d Canvas<T>::TransformToVisrangeHP(const double xs, const double ys,
//    Vec4d* const o_in_clip_space) const
//{
//    const Vec4d ps(xs, ys, 0.0, 1.0);
//    return this->TransformToVisrangeHP(ps, o_in_clip_space);
//}
//
//template<typename T>
//inline Vec4d Canvas<T>::TransformToVisrangeHP(const Vec4d& in_screen_space,
//    Vec4d* const o_in_clip_space) const
//{
//    //const Vec4d in_clip_space = _screen_to_clip_hp * in_screen_space;
//    const Vec4d in_viewport_space = _screen_to_viewport_hp * in_screen_space;
//    const Vec4d in_clip_space = _viewport_to_clip_hp * in_viewport_space;
//    if (o_in_clip_space != nullptr) {
//        *o_in_clip_space = in_clip_space;
//    }
//    return _clip_to_visrange_hp * in_clip_space;
//}

} // end of namespace tiny_graph_plot
