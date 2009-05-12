#include "Physics/MeshResource.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.MeshResource", MeshResource, Serializable)

bool MeshResource::serialize(Serializer& s)
{
	return true;
}

	}
}
