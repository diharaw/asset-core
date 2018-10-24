#include <offline/importer.h>
#include <offline/exporter.h>
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
        std::string output = "/Users/diharaw/Desktop/sun_temple/ast";//filesystem::get_current_working_directory();
        
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
        
        output = "/Users/diharaw/Desktop/sun_temple/ast/textures";//filesystem::get_current_working_directory();
        
        ast::Texture2DImportDesc tex_import_desc;
        tex_import_desc.pixel_type = ast::PIXEL_TYPE_UNORM8;
        tex_import_desc.options.srgb = true;
        
        tex_import_desc.source = ast::IMPORT_SOURCE_FILE;
        tex_import_desc.file = "/Users/diharaw/Desktop/sun_temple/Textures/M_Arch_Inst_Red_2_0_BaseColor.dds";
        
        ast::TextureDesc tex_desc;
        
        if (ast::import_texture_2d(tex_import_desc, tex_desc))
        {
            if (!ast::export_texture(output, tex_desc))
                printf("Failed to output Texture!\n");
        }
        else
            printf("Failed to import Texture!\n");
    }

	return 0;
}
