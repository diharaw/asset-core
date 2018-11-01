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
	
        Image(const PixelType& pixel_type = PIXEL_TYPE_UNORM8) : mip_slices(0), array_slices(0), type(pixel_type), compression(COMPRESSION_NONE)
		{
			for (int i = 0; i < 16; i++)
			{
				for (int j = 0; j < 16; j++)
					data[i][j].data = nullptr;
			}
		}
        
        ~Image()
        {
            deallocate();
        }
        
        Image& operator=(Image other)
        {
            deallocate();
            allocate(other.type, other.data[other.array_slices][other.mip_slices].width, other.data[other.array_slices][other.mip_slices].height, other.components, other.array_slices, other.mip_slices);
            
            return *this;
        }
        
        void allocate(const PixelType& pixel_type,
                      const uint32_t& base_mip_width,
                      const uint32_t& base_mip_height,
                      const uint32_t& component_count,
                      const uint32_t& array_slice_count,
                      const uint32_t& mip_slice_count)
        {
            type = pixel_type;
            components = component_count;
            mip_slices = mip_slice_count;
            array_slices = array_slice_count;
            
            for (uint32_t i = 0; i < array_slices; i++)
            {
                uint32_t w = base_mip_width;
                uint32_t h = base_mip_height;
                
                for (uint32_t j = 0; j < mip_slices; j++)
                {
                    data[i][j].width = w;
                    data[i][j].height = h;
                    data[i][j].data = malloc(w * h * components * size_t(type));
                    
                    w /= 2;
                    h /= 2;
                }
            }
        }
	
		void deallocate()
		{
			for (int i = 0; i < array_slices; i++)
			{
				for (int j = 0; j < mip_slices; j++)
				{
					if (data[i][j].data)
                    {
                        free(data[i][j].data);
                        data[i][j].data = nullptr;
                    }
				}
			}
		}
	
		size_t size(int array_slice, int mip_slice) const
		{
			return size_t(type) * data[array_slice][mip_slice].width * data[array_slice][mip_slice].height * components;
		}
	
		void to_bgra(int array_slice, int mip_slice)
		{
			Data& imgData = data[array_slice][mip_slice];
	
			if (type == PIXEL_TYPE_UNORM8)
            {
                TO_BGRA(uint8_t, imgData)
            }
            else if (type == PIXEL_TYPE_FLOAT16)
            {
                TO_BGRA(int16_t, imgData)
            }
            else if (type == PIXEL_TYPE_FLOAT32)
            {
                TO_BGRA(float, imgData)
            }
		}
	
		bool to_rgba(Image& img, int array_slice, int mip_slice)
		{
			if (components == 4)
			{
				img = *this;
				return true;
			}
        
			Data& imgData = data[array_slice][mip_slice];
			void* new_data = malloc(imgData.width * imgData.height * components * size_t(type));
	
            if (type == PIXEL_TYPE_UNORM8)
            {
                if (components == 3)
                {
                    TO_RGBA(uint8_t, 3, new_data)
                }
                else if (components == 2)
                {
                    TO_RGBA(uint8_t, 2, new_data)
                }
                else if (components == 1)
                {
                    TO_RGBA(uint8_t, 1, new_data)
                }
            }
            else if (type == PIXEL_TYPE_FLOAT16)
            {
                if (components == 3)
                {
                    TO_RGBA(int16_t, 3, new_data)
                }
                else if (components == 2)
                {
                    TO_RGBA(int16_t, 2, new_data)
                }
                else if (components == 1)
                {
                    TO_RGBA(int16_t, 1, new_data)
                }
            }
            else if (type == PIXEL_TYPE_FLOAT32)
            {
                if (components == 3)
                {
                    TO_RGBA(float, 3, new_data)
                }
                else if (components == 2)
                {
                    TO_RGBA(float, 2, new_data)
                }
                else if (components == 1)
                {
                    TO_RGBA(float, 1, new_data)
                }
            }

			img.components = 4;
			img.array_slices = array_slices;
			img.mip_slices = mip_slices;
			img.data[array_slice][mip_slice].data = new_data;
			img.data[array_slice][mip_slice].width = data[array_slice][mip_slice].width;
			img.data[array_slice][mip_slice].height = data[array_slice][mip_slice].height;
	
			return true;
		}
	};
}
