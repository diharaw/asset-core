#pragma once

#include <importer/image_importer.h>
#include <ostream>
#include <fstream>

namespace ast
{
struct ImageExportOptions
{
    std::string     path;
    PixelType       pixel_type  = PIXEL_TYPE_UNORM8;
    CompressionType compression = COMPRESSION_NONE;
    bool            normal_map  = false;
    bool            flip_green  = false;
#if defined(ENABLE_DEBUG_OUTPUT)
    bool debug_output = false;
#endif
    int output_mips = 0;
};

struct CubemapImageExportOptions
{
    std::string     path;
    CompressionType compression = COMPRESSION_NONE;
    int             output_mips = 0;
    int             force_cmp   = 0;
    bool            irradiance  = false;
    bool            radiance    = false;
#if defined(ENABLE_DEBUG_OUTPUT)
    bool debug_output = false;
#endif
};

extern bool export_image(Image& img, const ImageExportOptions& options);
extern bool cubemap_from_latlong(Image& src, const CubemapImageExportOptions& options);
extern bool cubemap_from_latlong(const std::string& input, const CubemapImageExportOptions& options);
}; // namespace ast
