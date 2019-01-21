#include <offline/image_exporter.h>
#include <offline/mesh_importer.h>
#include <offline/mesh_exporter.h>
#include <offline/scene_exporter.h>
#include <common/filesystem.h>
#include <runtime/loader.h>
#include <stdio.h>

void print_usage()
{
	printf("usage: brdf_lut [outpath]\n\n");
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
		std::string output = argv[1];

		if (output.size() == 0)
		{
			printf("ERROR: Invalid input path: %s\n\n", argv[i]);
			print_usage();

			return 1;
		}

		
		return 0;
	}
}
