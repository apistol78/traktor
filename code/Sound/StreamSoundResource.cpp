#include "Sound/StreamSoundResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StreamSoundResource", StreamSoundResource, SoundResource)

StreamSoundResource::StreamSoundResource(const TypeInfo* decoderType)
:	m_decoderType(decoderType)
{
}

const TypeInfo* StreamSoundResource::getDecoderType() const
{
	return m_decoderType;
}

bool StreamSoundResource::serialize(ISerializer& s)
{
	return s >> MemberType(L"decoderType", m_decoderType);
}

	}
}
