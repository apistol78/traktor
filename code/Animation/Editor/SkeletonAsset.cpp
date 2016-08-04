#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.SkeletonAsset", 3, SkeletonAsset, editor::Asset)

SkeletonAsset::SkeletonAsset()
:	m_offset(Vector4::origo())
,	m_scale(1.0f)
,	m_radius(0.25f)
,	m_invertX(false)
,	m_invertZ(false)
{
}

void SkeletonAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 1)
	{
		s >> Member< Vector4 >(L"offset", m_offset, AttributePoint());

		if (s.getVersion() >= 3)
			s >> Member< float >(L"scale", m_scale, AttributeRange(0.0f));
		
		if (s.getVersion() >= 2)
			s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
		else
			s >> Member< float >(L"boneRadius", m_radius, AttributeRange(0.0f));

		s >> Member< bool >(L"invertX", m_invertX);
		s >> Member< bool >(L"invertZ", m_invertZ);
	}
}


	}
}
