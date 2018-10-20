#include <runtime/loader.h>

#define WRITE_AND_OFFSET(stream, dest, size, offset) stream.write((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    
}
