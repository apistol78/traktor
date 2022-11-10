/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Compress/Test/CaseLzf.h"
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/MemoryStream.h"

namespace traktor
{
	namespace compress
	{
		namespace test
		{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.compress.test.CaseLzf", 0, CaseLzf, traktor::test::Case)

void CaseLzf::run()
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
	compress::DeflateStreamLzf deflateStream(&deflateDestinationStream);
	for (size_t i = 0; i < sizeof_array(source); i += 256)
	{
		int64_t nwritten = deflateStream.write(&source[i], 256);
		CASE_ASSERT_EQUAL(nwritten, 256);
	}
	deflateStream.flush();

	// Get size of compressed data.
	compressedSize = deflateDestinationStream.tell();
	CASE_ASSERT(compressedSize > 0);

	// Uncompress data by InflateStream.
	MemoryStream inflateSourceStream(compressed, compressedSize, true, false);
	compress::InflateStreamLzf inflateStream(&inflateSourceStream);
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
	CASE_ASSERT(std::memcmp(source, destination, sizeof_array(source)) == 0);
}

		}
	}
}
