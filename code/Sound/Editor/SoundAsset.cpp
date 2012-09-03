#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundCategory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", 2, SoundAsset, editor::Asset)

SoundAsset::SoundAsset()
:	m_stream(false)
,	m_preload(true)
{
}

bool SoundAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	if (s.getVersion() >= 2)
		s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));

	s >> Member< bool >(L"stream", m_stream);
	
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preload", m_preload);

	return true;
}

	}
}
