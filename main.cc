//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include <algorithm>
#include <fstream>
#include <iostream>

#include "random.h"
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "worker.h"

#ifdef __EMSCRIPTEN__
#include "canvas.h"
#endif

vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
             return attenuation*color(scattered, world, depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    }
}


hitable *random_scene() {
    int n = 500;
    hitable **list = new hitable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a+0.9*drand48(),0.2,b+0.9*drand48()); 
            if ((center-vec3(4,0.2,0)).length() > 0.9) { 
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new sphere(center, 0.2, new lambertian(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48())));
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new sphere(center, 0.2,
                            new metal(vec3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48())),  0.5*drand48()));
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new bvh_node(list, i, 0.0, 1.0);
}

const int nx = 1200;
const int ny = 800;
const int max_iterations = 10; // not checked on web

inline int rgb_index(int x, int y) { return 3 * (y * nx + x); }
inline int index(int x, int y) { return y * nx + x; }

int frames_count = 0;
int vertical_scan_position = 0;

#ifdef __EMSCRIPTEN__
function<void()> update;
void loop() { update(); }
#endif

int main() {
    hitable *list[5];
    float R = cos(M_PI/4);
    list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list,5);
    world = random_scene();

    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);
    camera* cam_ptr = &cam;

    float* image = new float[nx * ny * 3];
    int* image_gamma = new int[nx * ny * 3];
    int* image_sample_count = new int[nx * ny];

    for (int i = 0; i < nx * ny * 3; i++) {
        image[i] = 0;
        image_gamma[i] = 128;
    }

    for (int i = 0; i < nx * ny; i++) {
        image_sample_count[0] = 0;
    }

    function<void(int line)> render_scanline = [=](int scan_line) {
        int j = vertical_scan_position - scan_line;
        if (j < 0) return;
        for (int i = 0; i < nx; i++) {
            float u = float(i + drand48()) / float(nx);
            float v = float(j + drand48()) / float(ny);
            ray ry = cam_ptr->get_ray(u, v);
            vec3 p = ry.point_at_parameter(2.0);
            vec3 col = color(ry, world, 0);

            int idx = index(i, j);
            int idx_rgb = rgb_index(i, j);
            int n_samples = image_sample_count[idx]+1;
            float r = image[idx_rgb + 0] + col[0];
            float g = image[idx_rgb + 1] + col[1];
            float b = image[idx_rgb + 2] + col[2];
            image[idx_rgb + 0] = r;
            image[idx_rgb + 1] = g;
            image[idx_rgb + 2] = b;
            image_gamma[idx_rgb + 0] = int(255.99*sqrt(r / n_samples));
            image_gamma[idx_rgb + 1] = int(255.99*sqrt(g / n_samples));
            image_gamma[idx_rgb + 2] = int(255.99*sqrt(b / n_samples));
            image_sample_count[idx] = n_samples;
        }
    };

    int threads = std::min((int)thread::hardware_concurrency(), 8);
    cout << "hardware_concurrency: " << threads << std::endl;
    WorkerGroup scanline_workers;

    for (int n = 0; n < threads; ++n)
    {
        scanline_workers.AddWorker([=] {
            render_scanline(n);
        });
    }

#ifdef __EMSCRIPTEN__
    vertical_scan_position = ny - 1;

    canvas_setup(nx, ny);

    update = [&]() {
        if (vertical_scan_position < 0) {
            frames_count++;
            vertical_scan_position = ny - 1;
        }

        scanline_workers.Run();

        canvas_draw_data(image_gamma, nx, ny, vertical_scan_position, threads);
        vertical_scan_position -= threads;
    };

    emscripten_set_main_loop(loop, 0, 1);

#else // other platforms

    for (int i = 0; i < max_iterations; i++) {
        for (int j = ny - 1; j >= 0; j-=threads) {
            vertical_scan_position = j;
            scanline_workers.Run();
            std::cout << frames_count << "/" << max_iterations << ": ";
            std::cout << 100.0 * (ny - j) / ny << "%" << std::endl;
        }
        frames_count++;
    }

    std::ofstream out("output.ppm");
    out << "P3\n" << nx << " " << ny << "\n255\n";
    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            int idx = rgb_index(i, j);
            int ir = image_gamma[idx+0];
            int ig = image_gamma[idx+1];
            int ib = image_gamma[idx+2];
            out << ir << " " << ig << " " << ib << "\n";
        }
    }
    out.close();
#endif

    scanline_workers.Terminate();
    delete[] image;
}



