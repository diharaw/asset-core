#include <offline/image_exporter.h>
#include <common/filesystem.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <glm.hpp>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define NUM_COEFFICIENTS 9

// ----------------------------------------------------------------------------

static const float Pi       = 3.141592654f;
static const float CosineA0 = Pi;
static const float CosineA1 = (2.0f * Pi) / 3.0f;
static const float CosineA2 = Pi * 0.25f;

// ----------------------------------------------------------------------------

inline float lerp(float a, float b, float t)
{
    return a * (1.f - t) + b * t;
}

// ----------------------------------------------------------------------------

inline float area_integral(float x, float y)
{
    return std::atan2f(x * y, std::sqrtf(x * x + y * y + 1));
}

// ----------------------------------------------------------------------------

inline float unlerp(int val, int max)
{
    return (val + 0.5f) / max;
}

// ----------------------------------------------------------------------------

struct SH9
{
    float c[9];

    SH9()
    {
        for (uint32_t i = 0; i < 9; i++)
            c[i] = 0.0f;
    }
};

// ----------------------------------------------------------------------------

struct SH9Color
{
    glm::vec3 c[9];

    SH9Color()
    {
        for (uint32_t i = 0; i < 9; i++)
            c[i] = glm::vec3(0.0f);
    }
};

// ----------------------------------------------------------------------------

SH9 project_onto_sh9(glm::vec3 dir)
{
    SH9 sh;

    // Band 0
    sh.c[0] = 0.282095f;

    // Band 1
    sh.c[1] = -0.488603f * dir.y;
    sh.c[2] = 0.488603f * dir.z;
    sh.c[3] = -0.488603f * dir.x;

    // Band 2
    sh.c[4] = 1.092548f * dir.x * dir.y;
    sh.c[5] = -1.092548f * dir.y * dir.z;
    sh.c[6] = 0.315392f * (3.0f * dir.z * dir.z - 1.0f);
    sh.c[7] = -1.092548f * dir.x * dir.z;
    sh.c[8] = 0.546274f * (dir.x * dir.x - dir.y * dir.y);

    return sh;
}

// ----------------------------------------------------------------------------

glm::vec3 evaluate_sh9(const SH9Color& coef, const glm::vec3& direction)
{
    SH9 basis = project_onto_sh9(direction);

    glm::vec3 color = glm::vec3(0.0f);

    for (uint32_t i = 0; i < 9; i++)
        color += coef.c[i] * basis.c[i];

    color.x = std::max(0.0f, color.x);
    color.y = std::max(0.0f, color.y);
    color.z = std::max(0.0f, color.z);

    return color;
}

// ----------------------------------------------------------------------------

glm::vec3 evaluate_sh9_irradiance(const SH9Color& coef, const glm::vec3& direction)
{
    SH9 basis = project_onto_sh9(direction);

    basis.c[0] *= CosineA0;
    basis.c[1] *= CosineA1;
    basis.c[2] *= CosineA1;
    basis.c[3] *= CosineA1;
    basis.c[4] *= CosineA2;
    basis.c[5] *= CosineA2;
    basis.c[6] *= CosineA2;
    basis.c[7] *= CosineA2;
    basis.c[8] *= CosineA2;

    glm::vec3 color = glm::vec3(0.0f);

    for (uint32_t i = 0; i < 9; i++)
        color += coef.c[i] * basis.c[i];

    color.x = std::max(0.0f, color.x);
    color.y = std::max(0.0f, color.y);
    color.z = std::max(0.0f, color.z);

    return color;
}

// ----------------------------------------------------------------------------

struct Cubemap
{
    enum Face : uint32_t
    {
        POS_X,
        NEG_X,
        POS_Y,
        NEG_Y,
        POS_Z,
        NEG_Z,
        NUM_FACES
    };

    struct Data
    {
        glm::vec3* pixels = nullptr;

        ~Data()
        {
            free(pixels);
        }
    };

    Data    faces[NUM_FACES];
    int32_t width;
    int32_t height;
    int32_t comp;

    Cubemap(int32_t w, int32_t h) :
        width(w), height(h), comp(3)
    {
        for (uint32_t i = 0; i < NUM_FACES; i++)
            faces[i].pixels = new glm::vec3[w * h];
    }

    Cubemap(const std::string paths[])
    {
        for (uint32_t i = 0; i < NUM_FACES; i++)
            faces[i].pixels = (glm::vec3*)stbi_loadf(paths[i].c_str(), &width, &height, &comp, 0);
    }

    void set_texel(glm::vec3 color, Face face, int32_t x, int32_t y)
    {
        assert(face < NUM_FACES);
        assert(x < width);
        assert(y < height);

        faces[face].pixels[y * width + x] = color;
    }

    glm::vec3 read_texel(Face face, int32_t x, int32_t y) const
    {
        assert(face < NUM_FACES);
        assert(x < width);
        assert(y < height);

        return faces[face].pixels[y * width + x];
    }

    glm::vec3 read_texel_clamped(Face face, int32_t x, int32_t y) const
    {
        x = std::max(std::min(x, width - 1), 0);
        y = std::max(std::min(y, height - 1), 0);

        return read_texel(face, x, y);
    }

    glm::vec3 sample_face(Face face, float s, float t)
    {
        const float x = s * width;
        const float y = t * height;

        const int   x_base  = static_cast<int>(x);
        const int   y_base  = static_cast<int>(y);
        const float x_fract = x - x_base;
        const float y_fract = y - y_base;

        const glm::vec3 sample_00(read_texel_clamped(face, x_base, y_base));
        const glm::vec3 sample_10(read_texel_clamped(face, x_base + 1, y_base));
        const glm::vec3 sample_01(read_texel_clamped(face, x_base, y_base + 1));
        const glm::vec3 sample_11(read_texel_clamped(face, x_base + 1, y_base + 1));

        const glm::vec3 mix_0 = glm::mix(sample_00, sample_10, x_fract);
        const glm::vec3 mix_1 = glm::mix(sample_01, sample_11, x_fract);

        return glm::mix(mix_0, mix_1, y_fract);
    }

    float calculate_solid_angle(Face face, int32_t x, int32_t y) const
    {
        float s = unlerp(x, width) * 2.0f - 1.0f;
        float t = unlerp(y, height) * 2.0f - 1.0f;

        // assumes square face
        float half_texel_size = 1.0f / width;

        float x0 = s - half_texel_size;
        float y0 = t - half_texel_size;
        float x1 = s + half_texel_size;
        float y1 = t + half_texel_size;

        return area_integral(x0, y0) - area_integral(x0, y1) - area_integral(x1, y0) + area_integral(x1, y1);
    }

    glm::vec3 calculate_direction(Face face, int32_t face_x, int32_t face_y) const
    {
        float s = unlerp(face_x, width) * 2.0f - 1.0f;
        float t = unlerp(face_y, height) * 2.0f - 1.0f;

        float x, y, z;

        switch (face)
        {
            case POS_Z:
                x = s;
                y = -t;
                z = 1;
                break;
            case NEG_Z:
                x = -s;
                y = -t;
                z = -1;
                break;
            case NEG_X:
                x = -1;
                y = -t;
                z = s;
                break;
            case POS_X:
                x = 1;
                y = -t;
                z = -s;
                break;
            case POS_Y:
                x = s;
                y = 1;
                z = t;
                break;
            case NEG_Y:
                x = s;
                y = -1;
                z = -t;
                break;
        }

        // Normalize vector
        glm::vec3 d;
        float     inv_len = 1.0f / std::sqrtf(x * x + y * y + z * z);
        d.x               = x * inv_len;
        d.y               = y * inv_len;
        d.z               = z * inv_len;

        return d;
    }

    SH9Color project_sh9()
    {
        SH9Color coef;

        float weight_sum = 0.0f;

        for (int32_t face = 0; face < NUM_FACES; face++)
        {
            for (int32_t y = 0; y < height; y++)
            {
                for (int32_t x = 0; x < width; x++)
                {
                    const Cubemap::Face cube_face   = static_cast<Cubemap::Face>(face);
                    const glm::vec3     texel       = read_texel(cube_face, x, y);
                    const float         solid_angle = calculate_solid_angle(cube_face, x, y);
                    const glm::vec3     direction   = calculate_direction(cube_face, x, y);
                    const SH9           basis       = project_onto_sh9(direction);

                    for (uint32_t i = 0; i < 9; i++)
                        coef.c[i] += texel * basis.c[i] * solid_angle;

                    weight_sum += solid_angle;
                }
            }
        }

		printf("Weight Sum = %f\n", weight_sum);

        float scale = (4.0f * Pi) / weight_sum;

        for (uint32_t i = 0; i < 9; i++)
            coef.c[i] *= scale;

        return coef;
    }
};

// ----------------------------------------------------------------------------

void output_irradiance_cube_map(const SH9Color& coef, const std::string& name, const int32_t& w, const int32_t& h)
{
    std::string endings[] = {
        "_pos_x.hdr",
        "_neg_x.hdr",
        "_pos_y.hdr",
        "_neg_y.hdr",
        "_pos_z.hdr",
        "_neg_z.hdr"
    };

    Cubemap cubemap(w, h);

    for (int32_t face = 0; face < Cubemap::NUM_FACES; face++)
    {
        for (int32_t y = 0; y < cubemap.height; y++)
        {
            for (int32_t x = 0; x < cubemap.width; x++)
            {
                const Cubemap::Face cube_face = static_cast<Cubemap::Face>(face);
                const glm::vec3     direction = cubemap.calculate_direction(cube_face, x, y);

                glm::vec3 color = evaluate_sh9_irradiance(coef, direction);

                cubemap.set_texel(color / Pi, cube_face, x, y);
            }
        }

        std::string path = name + endings[face];

        if (stbi_write_hdr(path.c_str(), w, h, 3, &cubemap.faces[face].pixels[0].x) == 0)
            std::cout << "Failed to output cubemap face: " + path << std::endl;
    }
}

// ----------------------------------------------------------------------------

void output_cube_map(const SH9Color& coef, const std::string& name, const int32_t& w, const int32_t& h)
{
    std::string endings[] = {
        "_pos_x.hdr",
        "_neg_x.hdr",
        "_pos_y.hdr",
        "_neg_y.hdr",
        "_pos_z.hdr",
        "_neg_z.hdr"
    };

    Cubemap cubemap(w, h);

    for (int32_t face = 0; face < Cubemap::NUM_FACES; face++)
    {
        for (int32_t y = 0; y < cubemap.height; y++)
        {
            for (int32_t x = 0; x < cubemap.width; x++)
            {
                const Cubemap::Face cube_face = static_cast<Cubemap::Face>(face);
                const glm::vec3     direction = cubemap.calculate_direction(cube_face, x, y);

                glm::vec3 color = evaluate_sh9(coef, direction);

                cubemap.set_texel(color, cube_face, x, y);
            }
        }

        std::string path = name + endings[face];

        if (stbi_write_hdr(path.c_str(), w, h, 3, &cubemap.faces[face].pixels[0].x) == 0)
            std::cout << "Failed to output cubemap face: " + path << std::endl;
    }
}

// ----------------------------------------------------------------------------

void print_usage()
{
    printf("usage: brdf_lut [input_common_name] [outpath]\n\n");

    printf("Input options:\n");
    printf("  [input_common_name]	All faces must be HDR images with the name followed by '_posx', '_negx' etc \n");
    printf("  -P					Print spherical harmonics coefficients.\n");
    printf("  -I					Output irradiance map.\n");
}

// ----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        print_usage();
        return 1;
    }
    else
    {
        std::string input;
        std::string output;
        bool        print_coefficients = false;
        bool        output_irradiance  = false;

        int32_t input_idx = 99999;

        for (int32_t i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                char c = tolower(argv[i][1]);

                if (c == 'p')
                    print_coefficients = true;
                else if (c == 'i')
                    output_irradiance = true;
            }
            else if (i > 0)
            {
                if (i < input_idx)
                {
                    input_idx = 1;
                    input     = argv[i];

                    if (input.size() == 0)
                    {
                        printf("ERROR: Invalid input path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
                else
                {
                    output = argv[i];

                    if (output.size() == 0)
                    {
                        printf("ERROR: Invalid output path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
            }
        }

        auto start = std::chrono::high_resolution_clock::now();

        ast::ImageExportOptions options;
        options.compression = ast::COMPRESSION_NONE;
        options.pixel_type  = ast::PIXEL_TYPE_FLOAT32;
        options.normal_map  = false;
        options.flip_green  = false;
        options.output_mips = 0;

        std::string filename = filesystem::get_filename(output);
        std::string path     = filesystem::get_file_path(output);

        if (path.size() > 0)
            options.path = path;
        else
            options.path = "";

        ast::Image img;
        img.name = filename;
        img.allocate(ast::PIXEL_TYPE_FLOAT32, NUM_COEFFICIENTS, 1, 3, 1, 1);

        glm::vec3* pixels = (glm::vec3*)img.data[0][0].data;

        std::string faces[] = {
            input + "_posx.hdr",
            input + "_negx.hdr",
            input + "_posy.hdr",
            input + "_negy.hdr",
            input + "_posz.hdr",
            input + "_negz.hdr"
        };

        Cubemap cubemap(faces);

        SH9Color coef = cubemap.project_sh9();

		if (print_coefficients)
		{
			for (uint32_t i = 0; i < NUM_COEFFICIENTS; i++)
			{
			    printf("%i = [ %f, %f, %f ]\n", i, coef.c[i].x, coef.c[i].y, coef.c[i].z);
			    pixels[i] = coef.c[i];
			}
		}

		if (output_irradiance)
			output_irradiance_cube_map(coef, "output_irradiance_", 512, 512);

        if (!export_image(img, options))
            printf("Failed to output Spherical Harmonics coefficients: %s\n", output.c_str());

        auto                          finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time   = finish - start;

        printf("\nSuccessfully generated Spherical Harmonics coefficients in %f seconds\n\n", time.count());
    }

    return 0;
}

// ----------------------------------------------------------------------------