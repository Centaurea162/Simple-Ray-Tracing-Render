//
// Created by 86153 on 2023/5/18.
//

#ifndef RAYTRACING_CAMERA_H
#define RAYTRACING_CAMERA_H

#include "rtweekend.h"

class camera {
public:
    camera(
            point3 lookfrom,
            point3 lookat,
            vec3   vup,  // We can actually use any up vector we want, and simply project it onto this plane to get an up vector for the camera.
            double vfov, // vertical field-of-view in degrees
            double aspect_ratio,
            double aperture,
            double focus_dist,
            double _time0 = 0,
            double _time1 = 0
            ) {
        auto theta = degrees_to_radians(vfov);
        // z = 1
        auto h = tan(theta/2);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        
        auto focal_length = 1.0;

        origin = lookfrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;

        lens_radius = aperture / 2;
        time0 = _time0;
        time1 = _time1;
    }

    ray get_ray(double s, double t) const {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x() + v * rd.y();
        // 我们从一个圆盘 也就是透镜射出光线
        return ray(
                origin + offset,
                lower_left_corner + s*horizontal + t*vertical - origin - offset,
                random_double(time0, time1)
        );
    }

private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
    double time0, time1;  // shutter open/close times
};



#endif //RAYTRACING_CAMERA_H
