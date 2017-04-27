/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_LwClip_H
#define traktor_model_LwClip_H

#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace model
	{

struct LwChunk;

class LwClip : public Object
{
	T_RTTI_CLASS;

public:
	LwClip();

	bool read(const LwChunk& chunk, IStream* stream);

private:
	uint32_t m_type;
};

	}
}

#endif	// traktor_model_LwClip_H
