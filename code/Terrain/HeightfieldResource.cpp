#include "Terrain/HeightfieldResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.HeightfieldResource", HeightfieldResource, ISerializable)

HeightfieldResource::HeightfieldResource()
:	m_size(0)
,	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
,	m_patchDim(0)
,	m_detailSkip(0)
{
}

bool HeightfieldResource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);
	return true;
}

	}
}
