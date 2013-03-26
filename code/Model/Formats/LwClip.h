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
