#pragma once

#include "types.h"
#include "options.h"

struct AssimpImportData;

#define ASSET_EXTENSION "bin"

namespace binary_exporter
{
    extern void export_mesh(AssimpImportData* data, std::string output, Options options);
	extern void export_mesh(std::string file, std::string output, Options options = Options());
}
