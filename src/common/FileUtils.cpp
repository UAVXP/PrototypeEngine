#include <memory>

#include "FileSystem2.h"

namespace fileutils
{
uint8_t* LoadBinaryFile( const char* pszFileName, IFileSystem* pFileSystem )
{
	FileHandle_t hFile = pFileSystem->Open( pszFileName, "rb" );

	if( hFile == FILESYSTEM_INVALID_HANDLE )
		return nullptr;

	const auto size = pFileSystem->Size( hFile );

	//This function is used for text files too, so null terminate it. - Solokiller
	auto data = std::make_unique<uint8_t[]>( size + 1 );

	const auto read = pFileSystem->Read( data.get(), size, hFile );

	pFileSystem->Close( hFile );

	if( read != size )
		return nullptr;

	data[ size ] = '\0';

	return data.release();
}

void FreeBinaryFile( uint8_t* pBuffer )
{
	if( !pBuffer )
		return;

	delete[] pBuffer;
}
}
