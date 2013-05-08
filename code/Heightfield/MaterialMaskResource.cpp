#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Heightfield/MaterialMaskResource.h"
#include "Heightfield/MaterialMaskResourceLayer.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MaterialMaskResource", 1, MaterialMaskResource, ISerializable)

MaterialMaskResource::MaterialMaskResource()
:	m_size(0)
{
}

uint32_t MaterialMaskResource::getSize() const
{
	return m_size;
}

const RefArray< MaterialMaskResourceLayer >& MaterialMaskResource::getLayers() const
{
	return m_layers;
}

void MaterialMaskResource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);

	if (s.getVersion() >= 1)
		s >> MemberRefArray< MaterialMaskResourceLayer >(L"layers", m_layers);
}

	}
}
