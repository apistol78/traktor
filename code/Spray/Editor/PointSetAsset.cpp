#include "Spray/Editor/PointSetAsset.h"
#include "Spray/PointSet.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.spray.PointSetAsset", PointSetAsset, editor::Asset)

PointSetAsset::PointSetAsset()
:	m_fromFaces(false)
{
}

bool PointSetAsset::fromFaces() const
{
	return m_fromFaces;
}

const Type* PointSetAsset::getOutputType() const
{
	return &type_of< PointSet >();
}

bool PointSetAsset::serialize(Serializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	return s >> Member< bool >(L"fromFaces", m_fromFaces);
}

	}
}
