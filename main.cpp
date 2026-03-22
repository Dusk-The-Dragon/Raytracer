#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

const int WIDTH = 800;
const int HEIGHT = 600;
const int SAMPLES_PER_PIXEL = 50;
const int MAX_DEPTH = 50;

// Random number generator
std::mt19937 g_rng(std::random_device{}());
std::uniform_real_distribution<float> g_rand(0.0f, 1.0f);

float random_float() {
    return g_rand(g_rng);
}

struct Vector3 {
    float x, y, z;
    
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float t) const { return Vector3(x * t, y * t, z * t); }
    Vector3 operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
    float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalize() const { float len = length(); return Vector3(x / len, y / len, z / len); }
    Vector3 reflect(const Vector3& n) const { return *this - n * (2 * dot(n)); }
};

Vector3 random_in_unit_sphere() {
    while (true) {
        Vector3 p(random_float() * 2 - 1, random_float() * 2 - 1, random_float() * 2 - 1);
        if (p.length() < 1.0f) return p;
    }
}

struct Ray {
    Vector3 origin, direction;
    Ray(Vector3 o, Vector3 d) : origin(o), direction(d.normalize()) {}
};

enum MaterialType {
    DIFFUSE,
    METAL,
    DIELECTRIC
};

struct Material {
    MaterialType type;
    Vector3 albedo;
    float fuzziness;  // for metal
    float refraction_index;  // for dielectric
    
    Material(MaterialType t = DIFFUSE, Vector3 a = Vector3(1, 1, 1), float f = 0, float ri = 1.5f) 
        : type(t), albedo(a), fuzziness(f), refraction_index(ri) {}
};

struct Sphere {
    Vector3 center;
    float radius;
    Material material;
    
    Sphere(Vector3 c, float r, Material m = Material()) : center(c), radius(r), material(m) {}
    bool intersect(const Ray& ray, float& t, Vector3& normal, Material& mat) const {
        Vector3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) return false;
        
        float t1 = (-b - std::sqrt(discriminant)) / (2 * a);
        float t2 = (-b + std::sqrt(discriminant)) / (2 * a);
        
        if (t1 > 0.001f) {
            t = t1;
            Vector3 hit = ray.origin + ray.direction * t;
            normal = (hit - center).normalize();
            mat = material;
            return true;
        }
        if (t2 > 0.001f) {
            t = t2;
            Vector3 hit = ray.origin + ray.direction * t;
            normal = (hit - center).normalize();
            mat = material;
            return true;
        }
        return false;
    }
};

Vector3 rayTrace(const Ray& ray, const std::vector<Sphere>& spheres, int depth) {
    if (depth <= 0) return Vector3(0, 0, 0);
    
    float closest_t = 1e10f;
    int closest_sphere = -1;
    Vector3 closest_normal;
    Material closest_mat;
    
    // Check intersections with all spheres
    for (int i = 0; i < spheres.size(); ++i) {
        float t;
        Vector3 normal;
        Material mat;
        if (spheres[i].intersect(ray, t, normal, mat) && t < closest_t) {
            closest_t = t;
            closest_sphere = i;
            closest_normal = normal;
            closest_mat = mat;
        }
    }
    
    // If no intersection, return background with sky gradient
    if (closest_sphere == -1) {
        float t = (ray.direction.y + 1.0f) * 0.5f;
        return Vector3(1.0f, 1.0f, 1.0f) * (1.0f - t) + Vector3(0.5f, 0.7f, 1.0f) * t;
    }
    
    // Calculate hit point
    Vector3 hit_point = ray.origin + ray.direction * closest_t;
    
    // Make sure normal points opposite to ray direction
    if (closest_normal.dot(ray.direction) > 0) {
        closest_normal = closest_normal * -1.0f;
    }
    
    // Handle different material types
    if (closest_mat.type == DIFFUSE) {
        // Diffuse material: scatter in random direction
        Vector3 scatter_direction = closest_normal + random_in_unit_sphere().normalize();
        Ray scattered(hit_point, scatter_direction);
        Vector3 recursive_color = rayTrace(scattered, spheres, depth - 1);
        return closest_mat.albedo * recursive_color;
    }
    else if (closest_mat.type == METAL) {
        // Metal material: reflect with optional fuzziness
        Vector3 reflected = ray.direction.reflect(closest_normal);
        Vector3 fuzz = random_in_unit_sphere() * closest_mat.fuzziness;
        Ray scattered(hit_point, reflected + fuzz);
        if (scattered.direction.dot(closest_normal) > 0) {
            Vector3 recursive_color = rayTrace(scattered, spheres, depth - 1);
            return closest_mat.albedo * recursive_color;
        }
        return Vector3(0, 0, 0);
    }
    
    return Vector3(0, 0, 0);
}

int main() {
    std::cout << "Raytracing image (" << WIDTH << "x" << HEIGHT << ") with " 
              << SAMPLES_PER_PIXEL << " samples per pixel..." << std::endl;
    
    // Set up the "Ray Tracing in One Weekend" scene
    std::vector<Sphere> spheres;
    
    // Ground
    spheres.push_back(Sphere(
        Vector3(0.0f, -1000.0f, 0.0f), 
        1000.0f, 
        Material(DIFFUSE, Vector3(0.5f, 0.5f, 0.5f))
    ));
    
    // Large matte sphere
    spheres.push_back(Sphere(
        Vector3(0.0f, 1.0f, 0.0f),
        1.0f,
        Material(DIFFUSE, Vector3(0.8f, 0.3f, 0.3f))  // Rusty red
    ));
    
    // Metallic sphere
    spheres.push_back(Sphere(
        Vector3(-4.0f, 1.0f, 0.0f),
        1.0f,
        Material(METAL, Vector3(0.7f, 0.6f, 0.5f), 0.8f)  // Gold with fuzziness
    ));
    
    // Another metallic sphere
    spheres.push_back(Sphere(
        Vector3(4.0f, 1.0f, 0.0f),
        1.0f,
        Material(METAL, Vector3(0.8f, 0.8f, 0.8f), 0.3f)  // Silver, shiny
    ));
    
    // Small spheres randomly scattered
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = random_float();
            Vector3 center(a + 0.9f * random_float(), 0.2f, b + 0.9f * random_float());
            
            if ((center - Vector3(4.0f, 0.2f, 0.0f)).length() > 0.9f) {
                if (choose_mat < 0.8f) {
                    // Diffuse
                    Vector3 albedo(random_float() * random_float(), 
                                  random_float() * random_float(), 
                                  random_float() * random_float());
                    spheres.push_back(Sphere(center, 0.2f, Material(DIFFUSE, albedo)));
                } else if (choose_mat < 0.95f) {
                    // Metal
                    Vector3 albedo(0.5f + 0.5f * random_float(),
                                  0.5f + 0.5f * random_float(),
                                  0.5f + 0.5f * random_float());
                    float fuzz = 0.5f * random_float();
                    spheres.push_back(Sphere(center, 0.2f, Material(METAL, albedo, fuzz)));
                } else {
                    // Dielectric (rarely used in this simple version)
                    spheres.push_back(Sphere(center, 0.2f, Material(DIFFUSE, Vector3(0.9f, 0.9f, 0.9f))));
                }
            }
        }
    }
    
    // Accumulation buffer
    std::vector<Vector3> accumulation(WIDTH * HEIGHT, Vector3(0, 0, 0));
    
    // Render with accumulation
    for (int sample = 0; sample < SAMPLES_PER_PIXEL; ++sample) {
        std::cout << "Sample " << (sample + 1) << "/" << SAMPLES_PER_PIXEL << std::endl;
        
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                // Anti-aliasing: random jitter within pixel
                float u = (float)(x + random_float()) / WIDTH;
                float v = (float)(y + random_float()) / HEIGHT;
                
                // Camera setup
                Vector3 camera_pos(13.0f, 2.0f, 3.0f);
                Vector3 look_at(0.0f, 0.0f, 0.0f);
                
                // Simple perspective: rays from camera through viewport
                float fov = 20.0f;  // field of view in degrees
                float aspect = (float)WIDTH / HEIGHT;
                float viewport_width = 2.0f * std::tan(fov * 3.14159f / 360.0f);
                float viewport_height = viewport_width / aspect;
                
                Vector3 forward = (look_at - camera_pos).normalize();
                // For simplicity, compute a right vector
                Vector3 right = Vector3(forward.z, 0, -forward.x).normalize();
                Vector3 up = right.dot(right) > 0 ? 
                    Vector3(forward.y * forward.y + forward.z * forward.z, -forward.x * forward.y, -forward.x * forward.z) :
                    Vector3(0, 1, 0);
                
                // Pixel on the viewport
                float pixel_u = (u - 0.5f) * viewport_width;
                float pixel_v = (0.5f - v) * viewport_height;
                
                Vector3 pixel_pos = camera_pos + forward + right * pixel_u + up * pixel_v;
                Vector3 ray_dir = (pixel_pos - camera_pos).normalize();
                Ray ray(camera_pos, ray_dir);
                
                Vector3 color = rayTrace(ray, spheres, MAX_DEPTH);
                
                // Accumulate color
                int pixel_idx = y * WIDTH + x;
                accumulation[pixel_idx] = accumulation[pixel_idx] + color;
            }
            
            if ((y + 1) % 100 == 0) {
                std::cout << "  Row " << (y + 1) << "/" << HEIGHT << std::endl;
            }
        }
    }
    
    // Convert accumulation to final image
    std::vector<unsigned char> pixels(WIDTH * HEIGHT * 3);
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        Vector3 avg_color = accumulation[i] * (1.0f / SAMPLES_PER_PIXEL);
        
        // Gamma correction (sqrt for gamma 2.0)
        float r = std::sqrt(avg_color.x);
        float g = std::sqrt(avg_color.y);
        float b = std::sqrt(avg_color.z);
        
        pixels[i * 3 + 0] = (unsigned char)(std::min(1.0f, r) * 255.0f);
        pixels[i * 3 + 1] = (unsigned char)(std::min(1.0f, g) * 255.0f);
        pixels[i * 3 + 2] = (unsigned char)(std::min(1.0f, b) * 255.0f);
    }
    
    // Write PPM file
    std::ofstream file("output.ppm", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open output.ppm for writing" << std::endl;
        return -1;
    }
    
    file << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    file.close();
    
    std::cout << "Raytraced image saved to output.ppm" << std::endl;
    return 0;
}