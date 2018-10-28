//#include <offline/importer.h>
//#include <offline/exporter.h>
#include <offline/image_exporter.h>
#include <offline/mesh_importer.h>
#include <offline/mesh_exporter.h>
#include <common/filesystem.h>
#include <stdio.h>

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
//        ast::Image<uint8_t> img;
//
//        ast::ImageExportOptions options;
//
//        options.compression = ast::COMPRESSION_BC1;
//        options.normal_map = false;
//        options.output_mips = -1;
//        options.path = "/Users/diharaw/Desktop/sun_temple/ast/textures/M_Arch_Inst_Red_2_0_BaseColor.ast";
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
    
    std::string input = "/Users/diharaw/Desktop/sun_temple/SunTemple.fbx";
    
    ast::Mesh mesh;
    
    if (ast::import_mesh(input, mesh))
    {
        ast::MeshExportOption options;
        
        options.path = "/Users/diharaw/Desktop/sun_temple/ast";
        options.texture_source_path = "/Users/diharaw/Desktop/sun_temple";
        
        if (!ast::export_mesh(mesh, options))
            printf("Failed to output Material!\n");
    }
    else
        printf("Failed to import Material!\n");

	return 0;
}
