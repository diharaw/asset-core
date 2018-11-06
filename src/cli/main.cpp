//#include <offline/importer.h>
//#include <offline/exporter.h>
#include <offline/image_exporter.h>
#include <offline/mesh_importer.h>
#include <offline/mesh_exporter.h>
#include <common/filesystem.h>
#include <runtime/loader.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void read_and_export_image(const std::string& input)
{
    ast::Image image;
    
    if (ast::load_image(input, image))
    {
        std::string name = filesystem::get_filename(input);
        
        for (int i = 0; i < image.array_slices; i++)
        {
            for (int j = 0; j < image.mip_slices; j++)
            {
                std::string output_name = filesystem::get_file_path(input);
                output_name += "/";
                output_name += name;
                output_name += "_";
                output_name += std::to_string(i);
                output_name += "_";
                output_name += std::to_string(j);
                
                if (image.type == ast::PIXEL_TYPE_UNORM8)
                {
                    if (image.components == 1)
                    {
                        output_name += ".png";
                         stbi_write_png(output_name.c_str(), image.data[i][j].width, image.data[i][j].height, image.components, image.data[i][j].data, image.data[i][j].width * image.type * image.components);
                    }
                    else
                    {
                        output_name += ".bmp";
                        stbi_write_bmp(output_name.c_str(), image.data[i][j].width, image.data[i][j].height, image.components, image.data[i][j].data);
                    }
                }
                else
                {
                    output_name += ".hdr";
                    stbi_write_hdr(output_name.c_str(), image.data[i][j].width, image.data[i][j].height, image.components, (float*)image.data[i][j].data);
                }
            }
        }
    }
    else
        printf("Failed to Load Image!\n");
}

int main(int argc, char * argv[])
{
//    if (argc == 1)
//    {
//        printf("usage: AssetConverter <filename> [output_path]\n");
//        printf("<> = required\n");
//        printf("[] = optional\n");
//        return 1;
//    }
//    else
    {
//        std::string output = "/Users/diharaw/Desktop/sun_temple/ast";//filesystem::get_current_working_directory();
        
//        if (argc > 2)
//            output = argv[2];
        
//        ast::MeshDesc mesh;
//
//        ast::MeshImportDesc import_desc;
//
//        import_desc.source = ast::IMPORT_SOURCE_FILE;
//        import_desc.file = "/Users/diharaw/Desktop/sun_temple/SunTemple.fbx";//argv[1];
//
//        if (ast::import_mesh(import_desc, mesh))
//        {
//            if (!ast::export_mesh(output, mesh))
//                printf("Failed to output mesh.\n");
//        }
//        else
//            printf("Failed to import mesh.\n");
        
//        output = "/Users/diharaw/Desktop/sun_temple/ast/textures";//filesystem::get_current_working_directory();
        
//        ast::Texture2DImportDesc tex_import_desc;
//        tex_import_desc.pixel_type = ast::PIXEL_TYPE_UNORM8;
//        tex_import_desc.options.srgb = false;
//        tex_import_desc.options.generate_mip_chain = true;
//        tex_import_desc.options.normal_map = false;
//        tex_import_desc.options.compression = ast::COMPRESSION_BC5;
//
//        tex_import_desc.source = ast::IMPORT_SOURCE_FILE;
//        tex_import_desc.file = "/Users/diharaw/Desktop/sun_temple/Textures/M_Arch_Inst_Red_2_0_BaseColor.dds";
//
//        ast::TextureDesc tex_desc;
//
//        std::string input = "/Users/diharaw/Desktop/sun_temple/Textures/M_Arch_Inst_Red_2_0_BaseColor.dds";
//        ast::Image img;
//
//        ast::ImageExportOptions options;
//
//        options.compression = ast::COMPRESSION_NONE;
//        options.normal_map = false;
//        options.output_mips = -1;
//        options.path = "/Users/diharaw/Desktop/sun_temple/ast";
//
//        if (ast::import_image(img, input))
//        {
//            if (!ast::export_image(img, options))
//                printf("Failed to output Texture!\n");
//        }
//        else
//            printf("Failed to import Texture!\n");
    }
    
//    std::string input = "/Users/diharaw/Desktop/TextureExport/Arches_E_PineTree_3k.hdr";
//
//    ast::Image<float> img;
//
//    if (ast::import_image(img, input))
//    {
//        ast::CubemapImageExportOptions options;
//
//        options.compression = ast::COMPRESSION_BC6;
//        options.irradiance = true;
//        options.radiance = true;
//        options.output_mips = 0;
//        options.path = "/Users/diharaw/Desktop";
//
//        if (!ast::cubemap_from_latlong(img, options))
//            printf("Failed to output Texture!\n");
//    }
//    else
//        printf("Failed to import Texture!\n");
    
//    std::string input = "/Users/diharaw/Desktop/lpshead/head.OBJ";
//
//    ast::Mesh mesh;
//
//    if (ast::import_mesh(input, mesh))
//    {
//        ast::MeshExportOption options;
//
//        options.path = "/Users/diharaw/Desktop/lpshead/ast";
//        options.texture_source_path = "/Users/diharaw/Desktop/lpshead";
//        options.relative_material_path = "../materials";
//        options.relative_texture_path = "../textures";
//        options.use_compression = false;
//
//        if (!ast::export_mesh(mesh, options))
//            printf("Failed to output Material!\n");
//        else
//        {
//            ast::Mesh loaded_mesh;
//
//            if (ast::load_mesh("/Users/diharaw/Desktop/lpshead/ast/head.ast", loaded_mesh))
//            {
//                printf("Name                 : %s \n", loaded_mesh.name.c_str());
//                printf("Mesh Count           : %i \n", (int)loaded_mesh.submeshes.size());
//                printf("Vertex Count         : %i \n", (int)loaded_mesh.vertices.size());
//                printf("Skeletal Vertex Count: %i \n", (int)loaded_mesh.skeletal_vertices.size());
//                printf("Material Count       : %i \n", (int)loaded_mesh.materials.size());
//
//                for (const auto& mat : loaded_mesh.materials)
//                {
//                    printf("\tName: %s \n", mat.name.c_str());
//                }
//            }
//            else
//                printf("Failed to Load Mesh!\n");
//        }
//    }
//    else
//        printf("Failed to import Material!\n");
//    ast::Mesh loaded_mesh;
//
//    if (ast::load_mesh("/Users/diharaw/Desktop/lpshead/ast/head.ast", loaded_mesh))
//    {
//        printf("Name                 : %s \n", loaded_mesh.name.c_str());
//        printf("Mesh Count           : %i \n", (int)loaded_mesh.submeshes.size());
//        printf("Vertex Count         : %i \n", (int)loaded_mesh.vertices.size());
//        printf("Skeletal Vertex Count: %i \n", (int)loaded_mesh.skeletal_vertices.size());
//        printf("Material Count       : %i \n", (int)loaded_mesh.materials.size());
//
//        for (const auto& mat : loaded_mesh.materials)
//        {
//            printf("\tName: %s \n", mat.name.c_str());
//        }
//    }
//    else
//        printf("Failed to Load Mesh!\n");
    
//    ast::Image img;
//
//    if (ast::import_image(img, "/Users/diharaw/Desktop/lpshead/bump-lowRes.png"))
//    {
//        ast::ImageExportOptions opt;
//
//        opt.compression = ast::COMPRESSION_NONE;
//        opt.normal_map = false;
//        opt.output_mips = -1;
//        opt.path = "/Users/diharaw/Desktop";
//        opt.pixel_type = ast::PIXEL_TYPE_UNORM8;
//
//        ast::export_image(img, opt);
//    }
    
    read_and_export_image("/Users/diharaw/Desktop/lpshead/textures/bump-lowRes.ast");
    read_and_export_image("/Users/diharaw/Desktop/lpshead/textures/lambertian.ast");
    
	return 0;
}
