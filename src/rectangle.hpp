#pragma once
#include "shape.hpp"

class Rectangle : public Shape
{
public:
    Rectangle() = delete;
    Rectangle(float center_x_, float center_y_, float radius_w_, float radius_h_)
     : Shape(center_x_, center_y_), radius_w(radius_w_), radius_h(radius_h_) {}
    virtual ~Rectangle() {}
public:
    float radius_w;
    float radius_h;
public:
    virtual Point getNearestPoint(const Point& target) const override
    {
        bool left  = target.x < center.x - radius_w ? true : false;
        bool right = target.x > center.x + radius_w ? true : false;
        bool down  = target.y < center.y - radius_h ? true : false;
        bool up    = target.y > center.y + radius_h ? true : false;
        // corner outside case
        if (left  && up)   return center + Point { -radius_w,  radius_h };
        if (right && up)   return center + Point {  radius_w,  radius_h };
        if (left  && down) return center + Point { -radius_w, -radius_h };
        if (right && down) return center + Point {  radius_w, -radius_h };
        // edge outside case
        if (up)    return Point {            target.x, center.y + radius_h };
        if (down)  return Point {            target.x, center.y - radius_h };
        if (left)  return Point { center.x - radius_w,            target.y };
        if (right) return Point { center.x + radius_w,            target.y };
        // inside case
        return target;
    }
};
