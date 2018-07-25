#include <binary_exporter.h>
#include <filesystem.h>
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
        
        binary_exporter::export_mesh(argv[1], output);
    }

	return 0;
}
