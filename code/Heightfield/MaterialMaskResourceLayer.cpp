#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Heightfield/MaterialMaskResourceLayer.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MaterialMaskResourceLayer", 0, MaterialMaskResourceLayer, ISerializable)

const RefArray< ISerializable >& MaterialMaskResourceLayer::getParams() const
{
	return m_params;
}

void MaterialMaskResourceLayer::serialize(ISerializer& s)
{
	s >> MemberRefArray< ISerializable >(L"params", m_params);
}

	}
}
