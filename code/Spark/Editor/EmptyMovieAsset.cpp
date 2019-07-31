#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Editor/EmptyMovieAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.EmptyMovieAsset", 0, EmptyMovieAsset, ISerializable)

EmptyMovieAsset::EmptyMovieAsset()
:	m_stageWidth(1920)
,	m_stageHeight(1280)
,	m_frameRate(60)
,	m_backgroundColor(255, 255, 255, 255)
{
}

void EmptyMovieAsset::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"stageWidth", m_stageWidth);
	s >> Member< int32_t >(L"stageHeight", m_stageHeight);
	s >> Member< int32_t >(L"frameRate", m_frameRate);
	s >> Member< Color4ub >(L"backgroundColor", m_backgroundColor);
}

	}
}
