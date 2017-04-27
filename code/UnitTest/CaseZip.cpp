/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "UnitTest/CaseZip.h"
#include "Compress/Zip/DeflateStreamZip.h"
#include "Compress/Zip/InflateStreamZip.h"
#include "Core/Io/MemoryStream.h"

namespace traktor
{

void CaseZip::run()
{
	uint8_t source[4096];
	uint8_t compressed[8172];
	int64_t compressedSize;
	uint8_t destination[4096];

	// Prepare random source data.
	for (size_t i = 0; i < sizeof_array(source); ++i)
		source[i] = uint8_t(std::rand() & 63);

	// Compress source data by DeflateStream.
	MemoryStream deflateDestinationStream(compressed, sizeof_array(compressed), false, true);
	compress::DeflateStreamZip deflateStream(&deflateDestinationStream);
	for (size_t i = 0; i < sizeof_array(source); i += 256)
	{
		int64_t nwritten = deflateStream.write(&source[i], 256);
		CASE_ASSERT_EQUAL (nwritten, 256);
	}
	deflateStream.flush();

	// Get size of compressed data.
	compressedSize = deflateDestinationStream.tell();
	CASE_ASSERT (compressedSize > 0);

	// Uncompress data by InflateStream.
	MemoryStream inflateSourceStream(compressed, compressedSize, true, false);
	compress::InflateStreamZip inflateStream(&inflateSourceStream);
	uint8_t* destinationPtr = destination;
	for (;;)
	{
		uint8_t buf[256];
		int64_t nread = inflateStream.read(buf, sizeof_array(buf));
		if (nread <= 0)
			break;

		std::memcpy(destinationPtr, buf, nread);
		destinationPtr += nread;
	}
	inflateStream.flush();

	// Ensure data integrity.
	CASE_ASSERT (std::memcmp(source, destination, sizeof_array(source)) == 0);
}

}
