#include <runtime/loader.h>
#include <common/header.h>
#include <fstream>

#define READ_AND_OFFSET(stream, dest, size, offset) stream.read((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    bool load_image(const std::string& path, Image& image)
    {
        std::fstream f(path, std::ios::in | std::ios::binary);
        
        if (!f.is_open())
            return false;
        
        BINFileHeader file_header;
        BINImageHeader image_header;
        uint16_t len = 0;
        
        size_t offset = 0;
        
        READ_AND_OFFSET(f, &file_header, sizeof(BINFileHeader), offset);
        
        READ_AND_OFFSET(f, &len, sizeof(uint16_t), offset);
        image.name.resize(len);
        
        READ_AND_OFFSET(f, image.name.c_str(), len, offset);
        
        READ_AND_OFFSET(f, &image_header, sizeof(BINImageHeader), offset);
        
        image.array_slices = image_header.num_array_slices;
        image.mip_slices = image_header.num_mip_slices;
        image.components = image_header.num_channels;
        image.type = image_header.channel_size;
        image.compression = image_header.compression;
        
        if (image_header.num_array_slices == 0)
            return false;
        
        for (int i = 0; i < image_header.num_array_slices; i++)
        {
            for (int j = 0; j < image_header.num_array_slices; j++)
            {
                BINMipSliceHeader mip_header;
                READ_AND_OFFSET(f, &mip_header, sizeof(BINMipSliceHeader), offset);
                
                image.data[i][j].width = mip_header.width;
                image.data[i][j].height = mip_header.height;
                image.data[i][j].data = malloc(mip_header.size);
                
                READ_AND_OFFSET(f, image.data[i][j].data, mip_header.size, offset);
            }
        }
        
        return true;
    }
    
    bool load_mesh(const std::string& path, Mesh& mesh)
    {
        return true;
    }
    
    bool load_material(const std::string& path, Material& material)
    {
        return true;
    }
}
