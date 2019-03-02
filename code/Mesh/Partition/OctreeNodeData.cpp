#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Partition/OctreeNodeData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.OctreeNodeData", 0, OctreeNodeData, ISerializable)

void OctreeNodeData::serialize(ISerializer& s)
{
	s >> MemberAabb3(L"boundingBox", m_boundingBox);
	s >> MemberStlMap
		<
			uint8_t,
			std::vector< uint32_t >,
			MemberStlPair
			<
				uint8_t,
				std::vector< uint32_t >,
				Member< uint8_t >,
				MemberStlVector< uint32_t >
			>
		>(L"partIndices", m_partIndices);
	s >> MemberStaticArray< Ref< OctreeNodeData >, 8, MemberRef< OctreeNodeData > >(L"children", m_children);
}

	}
}
