#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Heightfield/Editor/MaterialMaskAssetLayer.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.MaterialMaskAssetLayer", 0, MaterialMaskAssetLayer, ISerializable)

const Color4ub& MaterialMaskAssetLayer::getColor() const
{
	return m_color;
}

const RefArray< ISerializable >& MaterialMaskAssetLayer::getParams() const
{
	return m_params;
}

void MaterialMaskAssetLayer::serialize(ISerializer& s)
{
	s >> Member< Color4ub >(L"color", m_color);
	s >> MemberRefArray< ISerializable >(L"params", m_params);
}

	}
}
