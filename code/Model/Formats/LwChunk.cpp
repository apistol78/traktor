/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwRead.h"

namespace traktor
{
	namespace model
	{

LwChunk::LwChunk()
:	tag(0)
,	length(0)
,	end(0)
{
}

bool LwChunk::read(IStream* stream)
{
	if (!lwRead< uint32_t >(stream, tag))
		return false;

	if (!lwRead< uint32_t >(stream, length))
		return false;

	end = stream->tell() + length;
	return true;
}

bool LwChunk::readSub(IStream* stream)
{
	if (!lwRead< uint32_t >(stream, tag))
		return false;

	uint16_t u16;
	if (!lwRead< uint16_t >(stream, u16))
		return false;

	length = u16;

	end = stream->tell() + length;
	return true;
}

	}
}
