#pragma once
#include <cmath>
#include <algorithm>
#include <set>

class Shape
{
protected:
    class Point
    {
    public:
        Point() = default;
        Point(float x_, float y_) : x(x), y(y_) {}
        Point(const Point& rhs) = default;
        Point(Point&& rhs) = default;
        Point& operator=(const Point& rhs) = default;
        Point& operator=(Point&& rhs) = default;
    public:
        float x = 0;
        float y = 0;

        Point& operator+=(const Point& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
        Point operator+(const Point& rhs) const { Point ret { *this }; return ret += rhs; }
        Point& operator-=(const Point& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
        Point operator-(const Point& rhs) const { Point ret { *this }; return ret -= rhs; }
        Point& operator*=(float rhs) { this->x *= rhs; this->y *= rhs; return *this; }
        Point operator*(float rhs) const { Point ret { *this }; return ret *= rhs; }
        bool operator==(const Point& rhs) const { return this->x == rhs.x && this->y == rhs.y ? true : false; }
        bool operator!=(const Point& rhs) const { return !(*this == rhs); }

        void normalize()
        {
            float norm = std::hypotf(x, y);
            this->x /= norm;
            this->y /= norm;
        }
        Point getNormalized()
        {
            Point ret { *this };
            ret.normalize();
            return ret;
        }

        static float distance(const Point& a, const Point& b) 
        {
            return std::hypotf(a.x - b.x, a.y - b.y);
        }
    };
public:
    Shape() = delete;
    Shape(float x_, float y_) { this->center.x = x_; this->center.y = y_; }
    virtual ~Shape() {};
public:
    Point center;
    std::set<Shape*> p_shapes_overlaped;
    Shape* p_region = nullptr;
public:
    virtual Point getNearestPoint(const Point& target) const = 0;
    // false when touch case, entry(circle) should call it
    bool isOverlap(const Shape* target, float offset = 0.0f) const 
    {
        Point nearestTargetPoint = target->getNearestPoint(this->center);
        Point nearestSelfPoint = this->getNearestPoint(nearestTargetPoint);
        // overlap case
        if (nearestTargetPoint == this->center) return true;
        if (nearestSelfPoint == nearestTargetPoint) return true;
        // outside case
        return Point::distance(nearestSelfPoint, nearestTargetPoint) > offset ? false : true;
    }
};
