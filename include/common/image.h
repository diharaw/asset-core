#pragma once

#include <iostream>
#include <stdint.h>
#include <string>

namespace ast
{
    enum CompressionType
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_BC1 = 1,
        COMPRESSION_BC1a = 2,
        COMPRESSION_BC2 = 3,
        COMPRESSION_BC3 = 4,
        COMPRESSION_BC3n = 5,
        COMPRESSION_BC4 = 6,
        COMPRESSION_BC5 = 7,
        COMPRESSION_BC6 = 8,
        COMPRESSION_BC7 = 9,
        COMPRESSION_ETC1 = 10,
        COMPRESSION_ETC2 = 11,
        COMPRESSION_PVR = 12
    };
    
    enum PixelType
    {
        PIXEL_TYPE_UNORM8 = 1,
        PIXEL_TYPE_FLOAT16 = 2,
        PIXEL_TYPE_FLOAT32 = 4
    };
    
    struct BINImageHeader
    {
        uint8_t  compression;
        uint8_t  channel_size;
        uint8_t  num_channels;
        uint16_t num_array_slices;
        uint8_t  num_mip_slices;
    };
    
    struct BINMipSliceHeader
    {
        uint16_t width;
        uint16_t height;
        int size;
    };
    
#define TO_BGRA(type, imgData)                                              \
Pixel<type, 4>* src = (Pixel<type, 4>*)imgData.data;                        \
                                                                            \
for (int y = 0; y < imgData.height; y++)                                    \
{                                                                           \
    for (int x = 0; x < imgData.width; x++)                                 \
    {                                                                       \
        type red = src[y * imgData.width + x].c[2];                         \
                                                                            \
        src[y * imgData.width + x].c[0] = src[y * imgData.width + x].c[2];  \
        src[y * imgData.width + x].c[2] = red;                              \
    }                                                                       \
}
    
#define TO_RGBA(type, num_components, dst_data)                                     \
Pixel<type, 4>* dst = (Pixel<type, 4>*) dst_data;                                   \
Pixel<type, num_components>* src = (Pixel<type, num_components>*)imgData.data;      \
                                                                                    \
    for (int y = 0; y < imgData.height; y++)                                        \
    {                                                                               \
        for (int x = 0; x < imgData.width; x++)                                     \
        {                                                                           \
            dst[y * imgData.width + x].c[0] = 0;                                    \
            dst[y * imgData.width + x].c[1] = 0;                                    \
            dst[y * imgData.width + x].c[2] = 0;                                    \
            dst[y * imgData.width + x].c[3] = std::numeric_limits<type>::max();     \
                                                                                    \
            for (int c = 0; c < num_components; c++)                                \
            {                                                                       \
                dst[y * imgData.width + x].c[c] = src[y * imgData.width + x].c[c];  \
            }                                                                       \
        }                                                                           \
    }
    
	struct Image
	{
		template<typename T, size_t N>
		struct Pixel
		{
			T c[N];
		};
	
		struct Data
		{
			void* data;
			int width;
			int height;
		};
        
        int components;
        int mip_slices;
        int array_slices;
        Data data[16][16];
        std::string name;
        PixelType type;
        CompressionType compression;
	
        Image(const PixelType& pixel_type = PIXEL_TYPE_UNORM8);
        ~Image();
        Image& operator=(Image other);
        void allocate(const PixelType& pixel_type,
                      const uint32_t& base_mip_width,
                      const uint32_t& base_mip_height,
                      const uint32_t& component_count,
                      const uint32_t& array_slice_count,
                      const uint32_t& mip_slice_count);
        void deallocate();
        size_t size(int array_slice, int mip_slice) const;
        void to_bgra(int array_slice, int mip_slice);
        bool to_rgba(Image& img, int array_slice, int mip_slice);
	};
}
