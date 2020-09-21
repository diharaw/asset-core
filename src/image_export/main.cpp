#include <exporter/image_exporter.h>
#include <common/filesystem.h>
#include <loader/loader.h>
#include <stdio.h>

void print_usage()
{
    printf("usage: image_export [options] infile [outpath]\n\n");

    printf("Input options:\n");
    printf("  -E			Cubemap.\n");
    printf("  -C			Compressed.\n");
    printf("  -D			Debug Output.\n");
    printf("  -R			Generate radiance.\n");
    printf("  -I			Generate irradiance.\n");
    printf("  -M			Generate mipmaps.\n");
    printf("  -N			Normal map.\n");
    printf("  -F			Flip green channel.\n");
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
        std::string                    input;
        ast::CubemapImageExportOptions cubemap_export_options;
        ast::ImageExportOptions        image_export_options;
        bool                           cubemap     = false;
        bool                           compression = false;

        int32_t input_idx = 99999;

        for (int32_t i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                char c = tolower(argv[i][1]);

                if (c == 'd')
                {
                    cubemap_export_options.debug_output = true;
                    image_export_options.debug_output   = true;
                }
                else if (c == 'r')
                    cubemap_export_options.radiance = true;
                else if (c == 'i')
                    cubemap_export_options.irradiance = true;
                else if (c == 'c')
                    compression = true;
                else if (c == 'e')
                    cubemap = true;
                else if (c == 'n')
                    image_export_options.normal_map = true;
                else if (c == 'm')
                    image_export_options.output_mips = -1;
                else if (c == 'f')
                    image_export_options.flip_green = true;
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
                    cubemap_export_options.path = argv[i];
                    image_export_options.path   = argv[i];

                    if (cubemap_export_options.path.size() == 0)
                    {
                        printf("ERROR: Invalid output path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
            }
        }

        if (cubemap)
        {
            if (!ast::cubemap_from_latlong(input, cubemap_export_options))
            {
                printf("ERROR: Failed to export cubemap!\n\n");
                return 1;
            }
        }
        else
        {
            ast::Image img;

            if (ast::import_image(img, input))
            {
                if (compression)
                {
                    if (img.components == 1)
                        image_export_options.compression = ast::COMPRESSION_BC4;
                    else if (img.components == 3)
                        image_export_options.compression = ast::COMPRESSION_BC1;
                    else if (img.components == 4)
                        image_export_options.compression = ast::COMPRESSION_BC3;
                }
                else
                    image_export_options.compression = ast::COMPRESSION_NONE;

                if (!ast::export_image(img, image_export_options))
                {
                    printf("ERROR: Failed to export image!\n\n");
                    return 1;
                }

                img.deallocate();
            }
        }

        return 0;
    }
}
