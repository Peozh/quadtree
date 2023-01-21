#include <iostream>
#include <vector>
#include <chrono>
#include <deque>
#include <random>
#include <map>
#include <algorithm>

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
public:
    virtual Point getNearestPoint(const Point& target) const = 0;
    bool isOverlap(const Shape* target) const // false when touch case
    {
        Point nearestTargetPoint = target->getNearestPoint(this->center);
        Point nearestSelfPoint = this->getNearestPoint(target->center);
        // inside case
        if (nearestTargetPoint == this->center) return true;
        if (nearestSelfPoint == target->center) return true;
        // outside case
        return Point::distance(this->center, nearestSelfPoint) > 
            Point::distance(this->center, nearestTargetPoint) ? true : false;
    }
};

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

class QuadTree
{
private:
    class Node : public Rectangle 
    {
    public:
        Node() = delete;
        Node(float center_x_, float center_y_, float radius_w_, float radius_h_, size_t capacity_)
         : Rectangle(center_x_, center_y_, radius_w_, radius_h_), capacity(capacity_) {}
        ~Node() 
        {
            if (childs[0] != nullptr) { delete childs[0]; childs[0] = nullptr; }
            if (childs[1] != nullptr) { delete childs[1]; childs[1] = nullptr; }
            if (childs[2] != nullptr) { delete childs[2]; childs[2] = nullptr; }
            if (childs[3] != nullptr) { delete childs[3]; childs[3] = nullptr; }
        }
    public:
        std::deque<Shape*> p_entries;
        bool isSplit = false;
        Node* childs[4] { nullptr, }; // LU, RU, LD, RD
        size_t capacity;
        size_t entryCount = 0;
    public:
        bool push(Shape* p_entry) // true if pushed
        {
            if (p_entry == nullptr) return false;
            // not included point
            auto eval = this->getNearestPoint(p_entry->center);
            if (this->getNearestPoint(p_entry->center) != p_entry->center) 
            {
                return false; 
            }
            // included point (not splitted)
            if (!this->isSplit) 
            { 
                this->p_entries.push_back(p_entry);
                ++this->entryCount;
                if (this->p_entries.size() > this->capacity) { this->split(); }
                return true; 
            } 
            // included point (splitted)
            if (childs[0]->push(p_entry)) { ++this->entryCount; return true; }
            if (childs[1]->push(p_entry)) { ++this->entryCount; return true; }
            if (childs[2]->push(p_entry)) { ++this->entryCount; return true; }
            if (childs[3]->push(p_entry)) { ++this->entryCount; return true; }
            return false;
        }
        void split()
        {
            float child_radius_w = this->radius_w/2;
            float child_radius_h = this->radius_h/2;
            if (childs[0] == nullptr) { childs[0] = new Node { this->center.x - child_radius_w, this->center.y + child_radius_h, child_radius_w, child_radius_h, this->capacity }; }
            if (childs[1] == nullptr) { childs[1] = new Node { this->center.x + child_radius_w, this->center.y + child_radius_h, child_radius_w, child_radius_h, this->capacity }; }
            if (childs[2] == nullptr) { childs[2] = new Node { this->center.x - child_radius_w, this->center.y - child_radius_h, child_radius_w, child_radius_h, this->capacity }; }
            if (childs[3] == nullptr) { childs[3] = new Node { this->center.x + child_radius_w, this->center.y - child_radius_h, child_radius_w, child_radius_h, this->capacity }; }
            this->isSplit = true;
            while (!p_entries.empty()) 
            { 
                auto p_entry = p_entries.front(); 
                p_entries.pop_front();
                if (childs[0]->push(p_entry)) continue;
                if (childs[1]->push(p_entry)) continue;
                if (childs[2]->push(p_entry)) continue;
                if (childs[3]->push(p_entry)) continue;
            }
        }
        void print(std::string prefix, std::string nodeID) const
        {
            std::cout << prefix << nodeID << " : " << this->p_entries.size() << " (" << this->entryCount << ")" << std::endl;
            if (childs[0] != nullptr) childs[0]->print(prefix + '\t', "LU");
            if (childs[1] != nullptr) childs[1]->print(prefix + '\t', "RU");
            if (childs[2] != nullptr) childs[2]->print(prefix + '\t', "LD");
            if (childs[3] != nullptr) childs[3]->print(prefix + '\t', "RD");
        }
        bool erase(Shape* p_entry)
        {
            if (p_entry == nullptr) return false;
            // not included point
            if (this->getNearestPoint(p_entry->center) != p_entry->center) return false;
            // included point (not splitted)
            if (!this->isSplit) 
            {
                auto iter = std::find(this->p_entries.begin(), this->p_entries.end(), p_entry);
                this->p_entries.erase(iter);
                --this->entryCount;
                return true; 
            } 
            // included point (splitted)
            if (childs[0]->erase(p_entry)) { --this->entryCount; }
            else if (childs[1]->erase(p_entry)) { --this->entryCount; }
            else if (childs[2]->erase(p_entry)) { --this->entryCount; }
            else if (childs[3]->erase(p_entry)) { --this->entryCount; }
            if (this->entryCount < this->capacity/2) { this->shrink(); }
            return true;
        }
        void shrink()
        {
            for (size_t idx = 0; idx < 4; ++idx) 
            {
                if (childs[idx] != nullptr) 
                { 
                    this->p_entries.insert(this->p_entries.cend(), childs[idx]->p_entries.cbegin(), childs[idx]->p_entries.cend()); 
                    childs[idx]->p_entries.clear(); 
                    delete childs[idx]; 
                    childs[idx] = nullptr; 
                }
            }
            this->isSplit = false;
        }
    };

public:
    QuadTree();
    QuadTree(float center_x_, float center_y_, float radius_w_, float radius_h_, size_t capacity_)
     : center_x(center_x_), center_y(center_y), radius_w(radius_w_), radius_h(radius_h_), capacity(capacity_)
    {
        this->p_root = new Node(center_x_, center_y_, radius_w_, radius_h_, capacity_);
    }
    ~QuadTree() 
    { 
        if (p_root != nullptr) delete p_root;
        for (auto p_shape : p_shapes) { delete p_shape.second; }
    }
public:
    Node* p_root = nullptr;
    std::map<int64_t, Shape*> p_shapes;
    size_t capacity = 4;
    float center_x;
    float center_y;
    float radius_w;
    float radius_h;
public:
    void initialize(float center_x_, float center_y_, float radius_w_, float radius_h_, size_t capacity_)
    {
        if (this->p_root != nullptr) { delete this->p_root; this->p_root = nullptr; }
        for (auto p_shape : p_shapes) { delete p_shape.second; }
        this->p_shapes.clear();
        this->center_x = center_x_;
        this->center_y = center_y_;
        this->radius_w = radius_w_;
        this->radius_h = radius_h_;
        this->capacity = capacity_;
        this->p_root = new Node(center_x_, center_y_, radius_w_, radius_h_, capacity_);
    }

    bool push(Shape* p_entry) 
    { 
        if (p_entry == nullptr) return false;
        if (this->p_root == nullptr) return false;
        this->p_shapes.insert(std::pair<int64_t, Shape*>((int64_t)p_entry, p_entry));
        return this->p_root->push(p_entry);
    }

    void print()
    {
        if (p_root == nullptr) return;
        p_root->print("", "root");
    }

    bool erase(Shape* p_entry)
    {
        if (p_entry == nullptr) return false;
        if (this->p_root == nullptr) return false;
        auto iter = this->p_shapes.find((int64_t)p_entry);
        this->p_shapes.erase(iter);
        delete p_entry;
        return this->p_root->erase(p_entry);
    }

    bool isContain(Shape* p_entry)
    {
        return (this->p_shapes.find((int64_t)p_entry) == this->p_shapes.end()) ? false : true;
    }
};

int main()
{
    constexpr float center_x = 50.0f;
    constexpr float center_y = 50.0f;
    constexpr float radius_w = 50.0f;
    constexpr float radius_h = 50.0f;
    constexpr size_t capacity = 8;
    constexpr size_t entry_count = 100;
    constexpr float entry_radius = 2.0f;

    QuadTree qt(center_x, center_y, radius_w, radius_h, capacity);

    float st_x = center_x - radius_w;
    float st_y = center_y - radius_h;
    float en_x = center_x + radius_w;
    float en_y = center_y + radius_h;

    std::random_device rnd_device;
    std::mt19937 mersenne_engine {rnd_device()};
    std::uniform_real_distribution<float> dist {st_x, en_x};
    for (size_t count = 0; count < entry_count; ++count)
    {   
        float x = dist(mersenne_engine);
        float y = dist(mersenne_engine);
        auto p_entry = new Circle { x, y, entry_radius };
        if (!qt.push(p_entry)) std::cout << "push failed [x : " << x << ", y : " << y << "]" << std::endl;
    }
    qt.print();
    for (auto p_shape : qt.p_shapes)
    {
        qt.erase(p_shape.second);
    }
    qt.print();
}

// todo : entry 객체간 겹침 확인하고 색상 변경하기. entry-node 간 겹침 확인 후, 대상 노드에 속한 노드의 entry 들만 확인하기