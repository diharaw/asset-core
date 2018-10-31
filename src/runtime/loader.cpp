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
        
        size_t offset = 0;
        
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
