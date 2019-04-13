//#include <offline/importer.h>
//#include <offline/exporter.h>
#include <offline/image_exporter.h>
#include <offline/mesh_importer.h>
#include <offline/mesh_exporter.h>
#include <offline/scene_exporter.h>
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

void print_usage()
{
	printf("usage: AssetCore [options] infile [outpath]\n\n");

	printf("Input options:\n");
	printf("  -S<path>		Source path for textures.\n");
	printf("  -T<path>      Relative path for textures.\n");
	printf("  -M<path>		Relative path for Materials.\n");
	printf("  -C			Disable texture compression for output textures.\n");
	printf("  -F			Flip normal map green channel.\n");
    printf("  -J            Output metadata JSON.\n");
}

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
		print_usage();
        return 1;
    }
	else
	{
		std::string input;
		ast::MeshExportOption export_options;
		ast::Mesh mesh;

		int32_t input_idx = 99999;

		for (int32_t i = 0; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				char c = tolower(argv[i][1]);

				if (c == 's')
				{
					std::string str = argv[i];
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
					std::string str = argv[i];
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
					std::string str = argv[i];
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
				else if (c == 'f')
					export_options.normal_map_flip_green = true;
				else if (c == 'c')
					export_options.use_compression = false;
                else if (c == 'j')
                    export_options.output_metadata = true;
			}
			else if (i > 0)
			{
				if (i < input_idx)
				{
					input_idx = 1;
					input = argv[i];

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
						printf("ERROR: Invalid input path: %s\n\n", argv[i]);
						print_usage();

						return 1;
					}
				}
			}
		}

		if (ast::import_mesh(input, mesh))
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
}
