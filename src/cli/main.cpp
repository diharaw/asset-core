#include <offline/importer.h>
#include <offline/exporter.h>
#include <common/filesystem.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
        printf("usage: AssetConverter <filename> [output_path]\n");
        printf("<> = required\n");
        printf("[] = optional\n");
        return 1;
    }
    else
    {
        std::string output = filesystem::get_current_working_directory();
        
        if (argc > 2)
            output = argv[2];
        
        ast::MeshDesc mesh;
        
        ast::MeshImportDesc import_desc;
        
        import_desc.file = argv[1];
        
        if (ast::import_mesh(import_desc, mesh))
        {
            if (!ast::export_mesh(output, mesh))
                printf("Failed to output mesh.\n");
        }
        else
            printf("Failed to import mesh.\n");
    }

	return 0;
}
