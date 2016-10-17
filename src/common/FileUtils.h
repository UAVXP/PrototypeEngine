#ifndef COMMON_FILEUTILS_H
#define COMMON_FILEUTILS_H

#include <cstdint>

class IFileSystem;

namespace fileutils
{
uint8_t* LoadBinaryFile( const char* pszFileName, IFileSystem* pFileSystem );

void FreeBinaryFile( uint8_t* pBuffer );
}

#endif //COMMON_FILEUTILS_H
