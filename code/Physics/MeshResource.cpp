#include "Physics/MeshResource.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshResource", 0, MeshResource, ISerializable)

bool MeshResource::serialize(ISerializer& s)
{
	return true;
}

	}
}
