#pragma once

#include <iostream>
#include <stdint.h>
#include <../dependency/stb/stb_image.h>

namespace ast
{
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
						free(data[i][j].data);
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
	};
}
