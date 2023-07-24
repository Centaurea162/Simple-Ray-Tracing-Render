#ifndef RAYTRACING_HITTABLE_H
#define RAYTRACING_HITTABLE_H

#include "utils/rtweekend.h"
#include "utils/ray.h"
#include "hittable/aabb.h"

class material;

struct hit_record{
    point3 p;
    vec3 normal;
    shared_ptr<material> mat_ptr;
    double t;
    double u;
    double v;
    bool front_face;

    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal :-outward_normal;
    }


};

class hittable{
    public:
        virtual bool hit(const ray& r,double t_min,double t_max, hit_record& rec) const = 0;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
        virtual double pdf_value(const point3& o, const vec3& v) const {
            return 0.0;
        }

        virtual vec3 random(const vec3& o) const {
            return vec3(1, 0, 0);
        }
};


class translate : public hittable {
public:
    translate(shared_ptr<hittable> p, const vec3& displacement)
            : ptr(p), offset(displacement) {}

    virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
    shared_ptr<hittable> ptr;
    vec3 offset;
};


bool translate::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    ray moved_r(r.origin() - offset, r.direction(), r.time());
    if (!ptr->hit(moved_r, t_min, t_max, rec))
        return false;
    // 为什么这样可行 因为我们用于渲染的 只是rec.p 只要先移动光源得到rec.p的光照结果 然后做变换即可
    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

bool translate::bounding_box(double time0, double time1, aabb& output_box) const {
    if (!ptr->bounding_box(time0, time1, output_box))
        return false;

    output_box = aabb(
            output_box.min() + offset,
            output_box.max() + offset);

    return true;
}

class rotate_y : public hittable {
public:
    rotate_y(shared_ptr<hittable> p, double angle);

    virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
        output_box = bbox;
        return hasbox;
    }

public:
    shared_ptr<hittable> ptr;
    double sin_theta;
    double cos_theta;
    bool hasbox;
    aabb bbox;
};

rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    point3 min( infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    // 此循环会遍历所有点的排列方式
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                // i*... + (1-i)*... 其实就是保证只取其中一个点
                auto x = i*bbox.max().x() + (1-i)*bbox.min().x();
                auto y = j*bbox.max().y() + (1-j)*bbox.min().y();
                auto z = k*bbox.max().z() + (1-k)*bbox.min().z();

                auto newx =  cos_theta*x + sin_theta*z;
                auto newz = -sin_theta*x + cos_theta*z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }
    // 注意 此时aabb的两个点 不再是模型上实际存在的点 而是算出来的边界
    bbox = aabb(min, max);
}

bool rotate_y::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = cos_theta*r.origin()[0] - sin_theta*r.origin()[2];
    origin[2] = sin_theta*r.origin()[0] + cos_theta*r.origin()[2];

    direction[0] = cos_theta*r.direction()[0] - sin_theta*r.direction()[2];
    direction[2] = sin_theta*r.direction()[0] + cos_theta*r.direction()[2];

    // 还是熟悉的方法 先旋转光线 然后得到结果后旋转得到的结果 包括点和法线
    ray rotated_r(origin, direction, r.time());

    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] =  cos_theta*rec.p[0] + sin_theta*rec.p[2];
    p[2] = -sin_theta*rec.p[0] + cos_theta*rec.p[2];

    normal[0] =  cos_theta*rec.normal[0] + sin_theta*rec.normal[2];
    normal[2] = -sin_theta*rec.normal[0] + cos_theta*rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}


class flip_face : public hittable {
public:
    flip_face(shared_ptr<hittable> p) : ptr(p) {}

    virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override {

        if (!ptr->hit(r, t_min, t_max, rec))
            return false;

        // 为何这样赋值 首先考虑到scatter到此处的光线 均是front_face=true的 因为它们都不在上一个物体的内部
        rec.front_face = !rec.front_face;
        return true;
    }

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
        return ptr->bounding_box(time0, time1, output_box);
    }

public:
    shared_ptr<hittable> ptr;
};

#endif //RAYTRACING_HITTABLE_H
