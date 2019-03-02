#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/Editor/FlashEmptyMovieAsset.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.flash.FlashEmptyMovieAsset", 0, FlashEmptyMovieAsset, ISerializable)

FlashEmptyMovieAsset::FlashEmptyMovieAsset()
:	m_stageWidth(1920)
,	m_stageHeight(1280)
,	m_frameRate(60)
,	m_backgroundColor(255, 255, 255, 255)
{
}

void FlashEmptyMovieAsset::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"stageWidth", m_stageWidth);
	s >> Member< int32_t >(L"stageHeight", m_stageHeight);
	s >> Member< int32_t >(L"frameRate", m_frameRate);
	s >> Member< Color4ub >(L"backgroundColor", m_backgroundColor);
}

	}
}
