//
// Created by 86153 on 2023/5/18.
//

#ifndef RAYTRACING_HITTABLE_LIST_H
#define RAYTRACING_HITTABLE_LIST_H

#include "hittable/hittable.h"
#include "hittable/aabb.h"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable{
    public:
        hittable_list() {}
        hittable_list(shared_ptr<hittable> object) { add(object); }

        void clear() {objects.clear();}
        void add(shared_ptr<hittable> object) { objects.push_back(object); }
        double pdf_value(const point3& o, const vec3& v) const;
        vec3 random(const vec3& o) const;

        virtual bool hit(
                const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(
                double time0, double time1, aabb& output_box) const override;


    public:
        // a vector store shared_ptrs that point hittable obj
        std::vector<shared_ptr<hittable>> objects;

};


bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    // intersect with multiobj we want the closest one
    auto closest_so_far = t_max;


    for(const auto& object : objects){
        // 判断第一个与光线相交的物体 我们使用closest_so_far 注意closest_so_far被传入当参数 且会被改变
        // 我们保证最后的rec存的是最近的物体的信息
        // 因为当我们得到最近的物体的 closest_so_far 之后 其他的物体都会因为距离过大而被提出
        if(object->hit(r, t_min, closest_so_far, temp_rec)){
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}


bool hittable_list::bounding_box(double time0, double time1, aabb& output_box) const{
    if(objects.empty()) return false;

    aabb temp_box;
    bool first_box = true;

    for (const auto& object : objects) {
        if (!object->bounding_box(time0, time1, temp_box)) return false; // 在此处更新temp_box
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }

    return true;

}


double hittable_list::pdf_value(const point3& o, const vec3& v) const {
    auto weight = 1.0/objects.size();
    auto sum = 0.0;

    for (const auto& object : objects)
        sum += weight * object->pdf_value(o, v);

    return sum;
}

vec3 hittable_list::random(const vec3& o) const {
    auto int_size = static_cast<int>(objects.size());
    return objects[random_int(0, int_size-1)]->random(o);
}


#endif //RAYTRACING_HITTABLE_LIST_H
