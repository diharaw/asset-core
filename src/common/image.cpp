#include <common/image.h>

namespace ast
{
    Image::Image(const PixelType& pixel_type) : mip_slices(0), array_slices(0), type(pixel_type), compression(COMPRESSION_NONE)
    {
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
                data[i][j].data = nullptr;
        }
    }
    
    Image::~Image()
    {
        deallocate();
    }
    
    Image& Image::operator=(Image other)
    {
        deallocate();
        allocate(other.type, other.data[other.array_slices][other.mip_slices].width, other.data[other.array_slices][other.mip_slices].height, other.components, other.array_slices, other.mip_slices);
        
        return *this;
    }
    
    void Image::allocate(const PixelType& pixel_type,
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
    
    void Image::deallocate()
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
    
    size_t Image::size(int array_slice, int mip_slice) const
    {
        return size_t(type) * data[array_slice][mip_slice].width * data[array_slice][mip_slice].height * components;
    }
    
    void Image::to_bgra(int array_slice, int mip_slice)
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
    
    bool Image::to_rgba(Image& img, int array_slice, int mip_slice)
    {
        if (components == 4)
        {
            img = *this;
            return true;
        }
        
        Data& imgData = data[array_slice][mip_slice];
        void* new_data = malloc(imgData.width * imgData.height * 4 * size_t(type));
        
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
}
