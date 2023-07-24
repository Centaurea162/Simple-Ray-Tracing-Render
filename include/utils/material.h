#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "utils/rtweekend.h"
#include "utils/texture.h"
#include "utils/pdf.h"
#include "utils/vec3.h"
#include "utils/onb.h"


#include "hittable/hittable.h"

struct scatter_record {
    ray specular_ray;
    bool is_specular;
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
};




class material {
public:

    virtual bool scatter(
            const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const {
        return false;
    }

    virtual double scattering_pdf(
            const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        return 0;
    }

    virtual color emitted(
            const ray& r_in, const hit_record& rec, double u, double v, const point3& p
    ) const {
        return color(0,0,0);
    }
};


class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}


    // scatter 负责计算反射方向 设置颜色 计算pdf
    virtual bool scatter(
            const ray& r_in, const hit_record& rec, scatter_record& srec
    )  const override {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }

    double scattering_pdf(
            const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine/pi;
    }



public:
    shared_ptr<texture> albedo;
};

class metal : public material {
public:
    metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
            const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected+fuzz*random_in_unit_sphere());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = 0;
        return true;

    }

public:
    color albedo;
    double fuzz;
};


class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
            const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = color(1.0, 1.0, 1.0);
        // 是从空气进入电解质还是从电解质进入空气 决定了refraction_ratio
        double refraction_ratio = rec.front_face ? (1.0/ir) : ir;

        vec3 unit_direction = unit_vector(r_in.direction());
        vec3 refracted = refract(unit_direction, rec.normal, refraction_ratio);

        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
            // Must Reflect
            direction = reflect(unit_direction, rec.normal);;
        } else {
            // Can Refract
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        }

        srec.specular_ray = ray(rec.p, direction, r_in.time());
        return true;
    }

public:
    double ir; // Index of Refraction 折射率


private:
    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine),5);
    }
};







class diffuse_light : public material  {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    //注意 scatter 在light material处停止 所以tracing的终点是光源
    virtual bool scatter(
            const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        return false;
    }

    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v,
                          const point3& p) const override {

        if (rec.front_face)
            return emit->value(u, v, p);
        else
            return color(0,0,0);
    }

public:
    shared_ptr<texture> emit;
};



#endif //RAYTRACING_MATERIAL_H
