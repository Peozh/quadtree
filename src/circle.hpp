#pragma once
#include "shape.hpp"

class Circle : public Shape
{
public:
    Circle() = delete;
    Circle(float center_x_, float center_y_, float radius_)
     : Shape(center_x_, center_y_), radius(radius_) {}
    virtual ~Circle() {}
public:
    float radius;
public:
    virtual Point getNearestPoint(const Point& target) const override
    {
        // inside case
        if (Point::distance(center, target) <= radius) return target;
        // outside case
        Point v = target - center;
        v.normalize();
        return center + (v*radius);
    }
};
