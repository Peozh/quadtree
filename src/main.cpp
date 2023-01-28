#include <iostream>
#include <vector>
#include <chrono>
#include <random>

#include "shape.hpp"
#include "circle.hpp"
#include "rectangle.hpp"
#include "quadtree.hpp"

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
