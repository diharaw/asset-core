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
        PIXEL_TYPE_DEFAULT = 0,
        PIXEL_TYPE_UNORM8 = 8,
        PIXEL_TYPE_FLOAT16 = 16,
        PIXEL_TYPE_FLOAT32 = 32
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
    
	template<typename T>
	struct Image
	{
		template<size_t N>
		struct Pixel
		{
			T c[N];
		};
	
		struct Data
		{
			T* data;
			int width;
			int height;
		};
	
		Image()
		{
			for (int i = 0; i < 16; i++)
			{
				for (int j = 0; j < 16; j++)
					data[i][j].data = nullptr;
			}
		}
	
		void unload()
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
			return sizeof(T) * data[array_slice][mip_slice].width * data[array_slice][mip_slice].height * components;
		}
	
		void to_bgra(int array_slice, int mip_slice)
		{
			Data& imgData = data[array_slice][mip_slice];
	
			Pixel<4>* src = (Pixel<4>*)imgData.data;
	
			for (int y = 0; y < imgData.height; y++)
			{
				for (int x = 0; x < imgData.width; x++)
				{
					T red = src[y * imgData.width + x].c[2];
	
					src[y * imgData.width + x].c[0] = src[y * imgData.width + x].c[2];
					src[y * imgData.width + x].c[2] = red;
				}
			}
		}
	
		bool to_rgba(Image<T>& img, int array_slice, int mip_slice)
		{
			if (components == 4)
			{
				img = *this;
				return true;
			}
	
			Data& imgData = data[array_slice][mip_slice];
			Pixel<4>* dst = new Pixel<4>[imgData.width * imgData.height];
	
			if (components == 3)
			{
				Pixel<3>* src = (Pixel<3>*)imgData.data;
	
				for (int y = 0; y < imgData.height; y++)
				{
					for (int x = 0; x < imgData.width; x++)
					{
						// Initialize values
						dst[y * imgData.width + x].c[0] = 0;
						dst[y * imgData.width + x].c[1] = 0;
						dst[y * imgData.width + x].c[2] = 0;
						dst[y * imgData.width + x].c[3] = std::numeric_limits<T>::max();
	
						for (int c = 0; c < components; c++)
						{
							dst[y * imgData.width + x].c[c] = src[y * imgData.width + x].c[c];
						}
					}
				}
			}
			else if (components == 2)
			{
				Pixel<2>* src = (Pixel<2>*)imgData.data;
	
				for (int y = 0; y < imgData.height; y++)
				{
					for (int x = 0; x < imgData.width; x++)
					{
						// Initialize values
						dst[y * imgData.width + x].c[0] = 0;
						dst[y * imgData.width + x].c[1] = 0;
						dst[y * imgData.width + x].c[2] = 0;
						dst[y * imgData.width + x].c[3] = std::numeric_limits<T>::max();
	
						for (int c = 0; c < components; c++)
						{
							dst[y * imgData.width + x].c[c] = src[y * imgData.width + x].c[c];
						}
					}
				}
			}
			else if (components == 1)
			{
				Pixel<1>* src = (Pixel<1>*)imgData.data;
	
				for (int y = 0; y < imgData.height; y++)
				{
					for (int x = 0; x < imgData.width; x++)
					{
						// Initialize values
						dst[y * imgData.width + x].c[0] = 0;
						dst[y * imgData.width + x].c[1] = 0;
						dst[y * imgData.width + x].c[2] = 0;
						dst[y * imgData.width + x].c[3] = std::numeric_limits<T>::max();
	
						for (int c = 0; c < components; c++)
						{
							dst[y * imgData.width + x].c[c] = src[y * imgData.width + x].c[c];
						}
					}
				}
			}
	
			img.components = 4;
			img.array_slices = array_slices;
			img.mip_slices = mip_slices;
			img.data[array_slice][mip_slice].data = (T*)dst;
			img.data[array_slice][mip_slice].width = data[array_slice][mip_slice].width;
			img.data[array_slice][mip_slice].height = data[array_slice][mip_slice].height;
	
			return true;
		}
	
		int components;
		int mip_slices;
		int array_slices;
		Data data[16][16];
        std::string name;
	};
}
