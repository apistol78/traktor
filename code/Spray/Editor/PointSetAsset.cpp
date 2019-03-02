#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Editor/PointSetAsset.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.PointSetAsset", 0, PointSetAsset, editor::Asset)

PointSetAsset::PointSetAsset()
:	m_fromFaces(false)
{
}

bool PointSetAsset::fromFaces() const
{
	return m_fromFaces;
}

void PointSetAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> Member< bool >(L"fromFaces", m_fromFaces);
}

	}
}
