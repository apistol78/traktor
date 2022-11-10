#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Mesh/MeshResource.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshResource", MeshResource, ISerializable)

void MeshResource::serialize(ISerializer& s)
{
	s >> Member< bool >(L"compressed", m_compressed);
}

}
