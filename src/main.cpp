#include "utils/rtweekend.h"
#include "utils/vec3.h"
#include "utils/color.h"
#include "utils/camera.h"
#include "utils/material.h"
#include "utils/pdf.h"

#include "hittable/hittable_list.h"
#include "hittable/sphere.h"
#include "hittable/moving_sphere.h"
#include "hittable/aarect.h"
#include "hittable/box.h"
#include "hittable/bvh.h"

#include <iostream>



double hit_sphere(const point3& center, double radius, const ray& r) {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = half_b*half_b - a*c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-half_b - sqrt(discriminant) ) / a;
    }
}


color ray_color(
        const ray& r, const color& background, const hittable& world,
        shared_ptr<hittable> lights, int depth
){
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0){
        return color(0,0,0);
    }

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;


    // 光来自物体光源
    scatter_record srec;
    // 当mat不是emitting material 调用父类方法返回0 此处只是初始化而已
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

    // 当tracing到emitting material时 返回false 此时的rec记录为emitting material的信息
    if (!rec.mat_ptr->scatter(r, rec, srec))
        // !False 没scatter到东西 返回初始化的emitted值 (0,0,0)
        return emitted;

    if (srec.is_specular) {
        return srec.attenuation
               * ray_color(srec.specular_ray, background, world, lights, depth-1);
    }


    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);


    ray scattered = ray(rec.p, p.generate(), r.time());
    auto pdf_val = p.value(scattered.direction());


    return emitted
           + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                         * ray_color(scattered, background, world, lights, depth-1) / pdf_val;

}





hittable_list cornell_box() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));


    shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), aluminum);    box1 = make_shared<rotate_y>(box1, 15);

    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(190,90,190), 90 , glass));

    return objects;
}


int main() {

    // Image
    const auto aspect_ratio = 1.0 / 1.0;
    const int image_width = 600;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 1000;
    const int max_depth = 50;

    // World

    auto world = cornell_box();
    auto lights = make_shared<hittable_list>();
    lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>()));
    lights->add(make_shared<sphere>(point3(190, 90, 190), 90, shared_ptr<material>()));

    color background(0,0,0);

    // Camera

    point3 lookfrom(278, 278, -800);
    point3 lookat(278, 278, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.0;
    auto vfov = 40.0;
    auto time0 = 0.0;
    auto time1 = 1.0;

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, time0, time1);

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height-1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, lights, max_depth);

            }
            // we let our mission of scale pixel color handle in write_color function
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }
    std::cerr << "\nDone.\n";
}