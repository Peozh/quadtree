#pragma once
#include "circle.hpp"
#include "rectangle.hpp"

#include <deque>
#include <string>
#include <map>
#include <iostream>

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
