#include <exporter/image_exporter.h>
#include <importer/mesh_importer.h>
#include <exporter/mesh_exporter.h>
#include <common/filesystem.h>
#include <loader/loader.h>
#include <stdio.h>

void print_usage()
{
    printf("usage: mesh_export [options] infile [outpath]\n\n");

    printf("Input options:\n");
    printf("  -S<path>      Source path for textures.\n");
    printf("  -T<path>      Relative path for textures.\n");
    printf("  -M<path>      Relative path for Materials.\n");
    printf("  -C            Disable texture compression for output textures.\n");
    printf("  -G            Flip normal map green channel.\n");
    printf("  -J            Output metadata JSON.\n");
    printf("  -V            Vertex function path.\n");
    printf("  -F            Fragment function path.\n");
    printf("  -D            Displacement as normal.\n");
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
        ast::Mesh              mesh;

        int32_t input_idx = 99999;

        for (int32_t i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                char c = tolower(argv[i][1]);

                if (c == 's')
                {
                    std::string str  = argv[i];
                    std::string path = str.substr(2, str.size() - 2);

                    if (path.size() == 0)
                    {
                        printf("ERROR: Invalid parameter: -S (%s)\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                    else
                        export_options.texture_source_path = path;
                }
                else if (c == 't')
                {
                    std::string str  = argv[i];
                    std::string path = str.substr(2, str.size() - 2);

                    if (path.size() == 0)
                    {
                        printf("ERROR: Invalid parameter: -T (%s)\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                    else
                        export_options.relative_texture_path = path;
                }
                else if (c == 'm')
                {
                    std::string str  = argv[i];
                    std::string path = str.substr(2, str.size() - 2);

                    if (path.size() == 0)
                    {
                        printf("ERROR: Invalid parameter: -M (%s)\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                    else
                        export_options.relative_material_path = path;
                }
                else if (c == 'g')
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
                    export_options.path = argv[i];

                    if (export_options.path.size() == 0)
                    {
                        printf("ERROR: Invalid export path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
            }
        }

        // If a relative Texture Sourc Path is not specified, it will be automatically set to the absolute path to the
        // folder containing the input mesh
        if (export_options.texture_source_path.length() == 0)
        {
            std::string mesh_path = filesystem::get_file_path(input);

            if (filesystem::is_absolute_path(input))
                export_options.texture_source_path = mesh_path;
            else
            {
                std::string cwd_path = filesystem::get_current_working_directory();

                export_options.texture_source_path = cwd_path;

                if (mesh_path.length() != 0)
                    export_options.texture_source_path = export_options.texture_source_path + "/" + mesh_path;
            }
        }

        if (ast::import_mesh(input, mesh, import_options))
        {
            if (!ast::export_mesh(mesh, export_options))
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
