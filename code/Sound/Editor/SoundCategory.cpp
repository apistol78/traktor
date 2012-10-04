#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Editor/SoundCategory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundCategory", 1, SoundCategory, ISerializable)


SoundCategory::SoundCategory()
:	m_volume(1.0f)
,	m_presence(0.0f)
{
}

bool SoundCategory::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"parent", m_parent, AttributeType(type_of< SoundCategory >()));
	s >> Member< float >(L"volume", m_volume);

	if (s.getVersion() >= 1)
		s >> Member< float >(L"presence", m_presence, AttributeRange(0.0f));

	return true;
}

	}
}
