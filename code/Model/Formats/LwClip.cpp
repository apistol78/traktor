#include "Core/Io/IStream.h"
#include "Model/Formats/LwChunk.h"
#include "Model/Formats/LwClip.h"
#include "Model/Formats/LwRead.h"
#include "Model/Formats/LwTags.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.LwClip", LwClip, Object)

LwClip::LwClip()
:	m_type(0)
{
}

bool LwClip::read(const LwChunk& chunk, IStream* stream)
{
	LwChunk sub;
	int32_t index;

	if (!lwRead< int32_t >(stream, index))
		return false;

	if (!sub.readSub(stream))
		return false;

	m_type = sub.tag;

	//if (m_type == LwTags::STIL)
	//	first = LwStillImage::read(stream);
	//else if(m_type == LwTags::ISEQ)
	//	first = LwImageSequence::read(stream);
	//else if (m_type == LwTags::ANIM)
	//	first = LwPlugInAnimation::read(sub, stream);
	//else if (m_type == LwTags::XREF)
	//	first = LwReference::read(bb);
	//else if (m_type == LwTags::STCC)
	//	first = LwColorCyclingStill::read(stream);
	//else
	//	return false;

	return true;
}

	}
}
