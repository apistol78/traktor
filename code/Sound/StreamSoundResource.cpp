#include "Sound/StreamSoundResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.StreamSoundResource", StreamSoundResource, SoundResource)

StreamSoundResource::StreamSoundResource(const Type* decoderType)
:	m_decoderType(decoderType)
{
}

const Type* StreamSoundResource::getDecoderType() const
{
	return m_decoderType;
}

bool StreamSoundResource::serialize(Serializer& s)
{
	return s >> MemberType(L"decoderType", m_decoderType);
}

	}
}
