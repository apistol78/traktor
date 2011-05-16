#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/ISoundResource.h"
#include "Sound/Editor/SoundAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", 1, SoundAsset, editor::Asset)

SoundAsset::SoundAsset()
:	m_stream(false)
,	m_preload(true)
{
}

const TypeInfo* SoundAsset::getOutputType() const
{
	return &type_of< ISoundResource >();
}

bool SoundAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< bool >(L"stream", m_stream);
	
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preload", m_preload);

	return true;
}

	}
}
