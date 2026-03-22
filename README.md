# GLSL Raytracer

A fast CPU-based raytracer written in C++ that generates photorealistic images in PPM format.

## Dependencies

- C++17 compatible compiler
- CMake 3.10+
- Standard C++ library

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./raytracer
```

This will generate an `output.ppm` file containing the raytraced image. You can view the PPM file with any image viewer (e.g., `feh`, `eog`, `convert`, etc.).

Example viewing commands:
```bash
feh output.ppm        # using feh
eog output.ppm        # using Eye of GNOME
display output.ppm    # using ImageMagick
```

## Features

- **Ray tracing**: Casts rays from the camera through each pixel
- **Sphere intersection**: Detects collisions with spheres in the scene
- **Diffuse shading**: Applies basic lighting calculations for realistic appearance
- **PPM output**: Saves results in portable PPM format (text and binary supported)
- **Progress reporting**: Shows rendering progress every 100 rows

## Scene

The default scene contains:
- One red sphere at the origin
- One green sphere to the left
- One blue sphere to the right
- A large gray ground plane below

The scene can be easily modified by editing the `main()` function.