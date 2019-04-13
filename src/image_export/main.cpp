#include <offline/image_exporter.h>
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

                if (output_name != "")
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
    printf("usage: image_export [options] infile [outpath]\n\n");

    printf("Input options:\n");
    printf("  -D			Debug Output.\n");
    printf("  -R			Generate radiance.\n");
    printf("  -I			Generate irradiance.\n");
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
        ast::CubemapImageExportOptions export_options;
        bool                           debug = false;

        int32_t input_idx = 99999;

        for (int32_t i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                char c = tolower(argv[i][1]);

                if (c == 'd')
                    debug = true;
                else if (c == 'r')
                    export_options.radiance = true;
                else if (c == 'i')
                    export_options.irradiance = true;
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
                        printf("ERROR: Invalid output path: %s\n\n", argv[i]);
                        print_usage();

                        return 1;
                    }
                }
            }
        }

        if (!ast::cubemap_from_latlong(input, export_options))
        {
            printf("ERROR: Failed to export mesh!\n\n");
            return 1;
        }

        if (debug)
        {
            std::string env_path = export_options.path + "/" + filesystem::get_filename(input) + ".ast";
            std::cout << env_path << std::endl;
            read_and_export_image(env_path);

            if (export_options.radiance)
            {
                std::string rad_path = export_options.path + "/" + filesystem::get_filename(input) + "_radiance.ast";
                std::cout << rad_path << std::endl;
                read_and_export_image(rad_path);
            }

            if (export_options.irradiance)
            {
                std::string irr_path = export_options.path + "/" + filesystem::get_filename(input) + "_irradiance.ast";
                std::cout << irr_path << std::endl;
                read_and_export_image(irr_path);
            }
        }

        return 0;
    }
}
