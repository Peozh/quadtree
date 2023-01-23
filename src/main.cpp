#include <iostream>
#include <vector>
#include <chrono>
#include <deque>
#include <random>
#include <map>
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
        Node(float center_x_, float center_y_, float radius_w_, float radius_h_, size_t capacity_, Node* p_parent_ = nullptr, float min_node_radius_ = 10.0f)
         : Rectangle(center_x_, center_y_, radius_w_, radius_h_), capacity(capacity_), p_parent(p_parent_), min_node_radius(min_node_radius_) {}
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
        Node* p_parent = nullptr;
        float min_node_radius;
    public:
        bool push(Shape* p_entry) // true if pushed
        {
            // dfs end condition (invalid input)
            if (p_entry == nullptr) { return false; }
            // not included center point
            // dfs end condition (not included)
            if (this->getNearestPoint(p_entry->center) != p_entry->center) { return false; }
            // included center point (not splitted)
            // dfs end condition (success on leaf)
            if (!this->isSplit) 
            { 
                this->p_entries.push_back(p_entry);
                ++this->entryCount;
                p_entry->p_region = this;
                if (this->p_entries.size() > this->capacity) { this->split(); }
                return true; 
            } 
            // included center point (splitted)
            // dfs orders
            if (childs[0]->push(p_entry)) { ++this->entryCount; return true; }
            else if (childs[1]->push(p_entry)) { ++this->entryCount; return true; }
            else if (childs[2]->push(p_entry)) { ++this->entryCount; return true; }
            else if (childs[3]->push(p_entry)) { ++this->entryCount; return true; }

            // dfs end condition (fail)
            return false;
        }
        void split()
        {
            if (!this->radius_h/2 > this->min_node_radius) return; 
            float child_radius_w = this->radius_w/2;
            float child_radius_h = this->radius_h/2;
            if (childs[0] == nullptr) { childs[0] = new Node { this->center.x - child_radius_w, this->center.y + child_radius_h, child_radius_w, child_radius_h, this->capacity, this }; }
            if (childs[1] == nullptr) { childs[1] = new Node { this->center.x + child_radius_w, this->center.y + child_radius_h, child_radius_w, child_radius_h, this->capacity, this }; }
            if (childs[2] == nullptr) { childs[2] = new Node { this->center.x - child_radius_w, this->center.y - child_radius_h, child_radius_w, child_radius_h, this->capacity, this }; }
            if (childs[3] == nullptr) { childs[3] = new Node { this->center.x + child_radius_w, this->center.y - child_radius_h, child_radius_w, child_radius_h, this->capacity, this }; }
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
                for (auto p_other_entry : p_entry->p_shapes_overlaped)
                {
                    auto iter2 = std::find(p_other_entry->p_shapes_overlaped.begin(), p_other_entry->p_shapes_overlaped.end(), p_entry);
                    p_other_entry->p_shapes_overlaped.erase(iter2);
                }
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
                    // update p_region
                    for (auto p_entry : childs[idx]->p_entries) p_entry->p_region = this;
                    this->p_entries.insert(this->p_entries.cend(), childs[idx]->p_entries.cbegin(), childs[idx]->p_entries.cend()); 
                    childs[idx]->p_entries.clear(); 
                    delete childs[idx]; 
                    childs[idx] = nullptr; 
                }
            }
            this->isSplit = false;
        }
        void updateOverlap(Shape* p_entry, float min_node_radius)
        {
            // leaf node case
            // find rect having overlappable entries
            if (!this->isSplit) 
            {
                if (!p_entry->isOverlap(this, min_node_radius)) return; // not overlappable
                for (auto p_target_entry : this->p_entries) {
                    if (p_target_entry == p_entry) continue;
                    if (p_entry->isOverlap(p_target_entry)) { 
                        p_entry->p_shapes_overlaped.insert(p_target_entry); 
                        p_target_entry->p_shapes_overlaped.insert(p_entry);
                    }
                }
                return;
            }
            // non-leaf node case
            this->childs[0]->updateOverlap(p_entry, min_node_radius);
            this->childs[1]->updateOverlap(p_entry, min_node_radius);
            this->childs[2]->updateOverlap(p_entry, min_node_radius);
            this->childs[3]->updateOverlap(p_entry, min_node_radius);
        }
    };

public:
    QuadTree() = delete;
    QuadTree(float center_x_, float center_y_, float radius_w_, float radius_h_, size_t capacity_, float min_node_radius_ = 10.0f)
     : center_x(center_x_), center_y(center_y), radius_w(radius_w_), radius_h(radius_h_), capacity(capacity_), min_node_radius(min_node_radius_)
    {
        this->p_root = new Node(center_x_, center_y_, radius_w_, radius_h_, capacity_, nullptr, min_node_radius_);
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
    float min_node_radius; // same as max entry radius
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
        bool ret = this->p_root->push(p_entry);
        this->p_root->updateOverlap(p_entry, min_node_radius);
        return ret;
    }

    void print()
    {
        if (this->p_root == nullptr) return;
        p_root->print("", "root");
    }

    bool erase(Shape* p_entry)
    {
        if (p_entry == nullptr) return false;
        if (this->p_root == nullptr) return false;
        auto iter = this->p_shapes.find((int64_t)p_entry);
        this->p_shapes.erase(iter);
        auto ret = this->p_root->erase(p_entry);
        delete p_entry;
        return ret;
    }

    bool isContain(Shape* p_entry)
    {
        return (this->p_shapes.find((int64_t)p_entry) == this->p_shapes.end()) ? false : true;
    }

    void clear()
    {
        if (this->p_root == nullptr) return;
        
        for (auto [key, p_shape] : this->p_shapes)
        {
            this->p_root->erase(p_shape);
            delete p_shape;
        }
        this->p_shapes.clear();
    }
};

int main()
{
    constexpr float center_x = 50.0f;
    constexpr float center_y = 50.0f;
    constexpr float radius_w = 50.0f;
    constexpr float radius_h = 50.0f;
    constexpr size_t capacity = 8;
    constexpr size_t entry_count = 1000;
    constexpr float entry_radius = 2.0f;
    constexpr float max_entry_radius = entry_radius;
    constexpr float min_node_radius = max_entry_radius;

    QuadTree qt(center_x, center_y, radius_w, radius_h, capacity, min_node_radius);

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
    qt.clear();
    qt.print();
}

// todo : push/erase 마다 entry 객체간 겹침 확인하고 색상 변경하기. entry-node 간 겹침 확인 후, 대상 노드에 속한 노드의 entry 들만 확인하기
    // todo : push 시 entry 와 겹친 entry 들 검색한 결과 보관하기
    // todo : entry 마다 겹친 entry 객체들 보관하기 (제거 시 색상 업데이트용)

    // todo : node rect length 하한 지정가능하게 하기

    // todo : node 에서 parent node 포인터 가지기, 
    // todo : entry 가 자신이 속한 node* 가지기 (push/split/shrink 시 업데이트됨)

    // todo : 하한 node rect length 의 길이는 겹침 검색이 유효하기 위해서 최대 entry 반지름보다 크거나 같아야함. (엄청 큰 기존 entry 는 다른 추가 삽입된 작은 entry 에서 보이지 않음. 닿지 않는 region 에 중심이 존재하기 때문)
        // todo : 더 큰 entry 를 위해서는 중간노드에도 저장하면 됨. 또한 서치 시 중간노드의 entry 목록도 추가하면 됨. o (다만 큰 entry 가 많으면 최적화 이슈 발생하므로 안하는게 나음)

    // todo : rect 와 circle 간의 최근접점 찾는 알고리즘 수정 요구됨. rect 가 먼저 수행되고 해당 rect 점에 대한 circle 의 점이 서칭되어야 함
    // todo : push 시에 overlap rect 를 찾지 말고, 따로 overlap rect search 분리 해야할듯. x (또는 push 성공적으로 끝나고 돌아오는 길에 또다른 재귀로 업데이트하기)
    // todo : overlap rect 찾을 때, 최대 entry radius 범위만큼 더 확장해서 offset 으로 추가해야할듯. 해당 rect 모서리에 있는 entry 와 겹칠 수 있기 때문
