/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Model/Formats/LwPolygon.h"
#include "Model/Formats/LwRead.h"

namespace traktor
{
	namespace model
	{

LwPolygon::LwPolygon()
:	type(0)
,	flags(0)
,	surfaceTag(0)
,	surfaceIndex(0)
,	surface(0)
,	smoothGroup(0)
{
}

bool LwPolygon::readLWO2(IStream* stream)
{
	if (!lwRead< uint16_t >(stream, flags))
		return false;

	int32_t nvertices = int32_t(flags & 0x3ff);
	indices.resize(nvertices);

	for (int i = 0; i < nvertices; ++i)
	{
		if (!lwReadIndex(stream, indices[i]))
			return false;
	}

	return true;
}

bool LwPolygon::readLWOB(IStream* stream)
{
	if (!lwRead< uint16_t >(stream, flags))
		return false;

	int32_t nvertices = int32_t(flags & 0xffff);
	indices.resize(nvertices);

	for (int32_t i = 0; i < nvertices; ++i)
	{
		uint16_t index;
		if (!lwRead< uint16_t >(stream, index))
			return false;
		indices[i] = index;
	}

	if (!lwRead< uint16_t >(stream, surfaceTag))
		return false;

	return true;
}

	}
}
