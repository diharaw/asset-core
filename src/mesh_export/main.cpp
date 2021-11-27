#include <exporter/image_exporter.h>
#include <importer/mesh_importer.h>
#include <exporter/mesh_exporter.h>
#include <common/filesystem.h>
#include <stdio.h>

void print_usage()
{
    printf("usage: mesh_export [options] infile [ouput_root_folder]\n\n");

    printf("Input options:\n");
    printf("  -C            Disable texture compression for output textures.\n");
    printf("  -G            Flip normal map green channel.\n");
    printf("  -J            Output metadata JSON.\n");
    printf("  -D            Displacement as normal.\n");
    printf("  -O            Input mesh is from the ORCA library.\n");
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        print_usage();
        return 1;
    }
    else
    {
        std::string            input;
        ast::MeshImportOptions import_options;
        ast::MeshExportOption  export_options;
        ast::MeshImportResult  import_result;

        int32_t input_idx = 99999;

        for (int32_t i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                char c = tolower(argv[i][1]);

                if (c == 'g')
                    export_options.normal_map_flip_green = true;
                else if (c == 'c')
                    export_options.use_compression = false;
                else if (c == 'j')
                    export_options.output_metadata = true;
                else if (c == 'd')
                    import_options.displacement_as_normal = true;
                else if (c == 'o')
                    import_options.is_orca_mesh = true;
            }
            else if (i > 0)
            {
                if (i < input_idx)
                {
                    input_idx = 1;
                    input     = argv[i];

                    if (input.size() == 0)
                    {
                        printf("ERROR: Invalid input path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
                else
                {
                    export_options.output_root_folder_path = argv[i];

                    if (export_options.output_root_folder_path.size() == 0)
                    {
                        printf("ERROR: Invalid export path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
            }
        }

        if (ast::import_mesh(input, import_result, import_options))
        {
            if (!ast::export_mesh(import_result, export_options))
            {
                printf("ERROR: Failed to export mesh!\n\n");
                return 1;
            }
        }
        else
        {
            printf("ERROR: Failed to import mesh!\n\n");
            return 1;
        }

        return 0;
    }

    return 0;
}
