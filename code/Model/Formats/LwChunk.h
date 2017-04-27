/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_LwChunk_H
#define traktor_model_LwChunk_H

#include "Core/Config.h"

namespace traktor
{

class IStream;

	namespace model
	{

struct LwChunk
{
	uint32_t tag;
	uint32_t length;
	uint32_t end;

	LwChunk();

	bool read(IStream* stream);

	bool readSub(IStream* stream);
};

	}
}

#endif	// traktor_model_LwChunk_H
