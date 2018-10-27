#pragma once

#include <stdint.h>

#define AST_VERSION 1

namespace ast
{
    enum AssetType
    {
        ASSET_IMAGE = 0,
        ASSET_MESH = 1
    };
    
    struct BINFileHeader
    {
        uint32_t magic;
        uint8_t  version;
        uint8_t  type;
    };
}
